#pragma once

#include <map>
#include <vector>

#include <boost/thread.hpp>
#include <boost/mpi.hpp>
namespace mpi = boost::mpi;

#include "../../Core/Object/Object.h"

/*
// Start with the temp scheduling staff before moving them into their proper places
// Start by defining a resource, and a resource pool
typedef int ResourceHandle;
typedef int GroupHandle;

// Basic Resource object
class IResource :
	public Object
{
public:
	enum State
	{
		Idle,
		Busy,
		Blocked,
		Unused
	};

protected:
	State m_resourceState;
	ResourceHandle m_resourceHandle;

public:
	IResource(void) { }
	IResource(ResourceHandle p_resourceHandle) 
		: m_resourceHandle(p_resourceHandle) { }

	State GetState(void) const { return m_resourceState; }	
	virtual ResourceHandle GetGlobalHandle(void) const { return m_resourceHandle; }
};

class IResourceGroup
{
protected:
	std::map<ResourceHandle, IResource*> m_resourceMap;
	
public:
	bool Contains(IResource *p_pResource) const 
	{
		return (m_resourceMap.find(p_pResource->GetGlobalHandle()) != m_resourceMap.end());
	}
	
	void Register(IResource *p_pResource) 
	{
		if (!Contains(p_pResource))
			m_resourceMap[p_pResource->GetGlobalHandle()] = p_pResource;
	}
	
	size_t GetSize(void) const 
	{ return m_resourceMap.size(); }
};
*/

// Load balancer
// Node coordinator
// Node worker

/*
// Control Resource (controls a resource group)
class ControlResource :
	public IResource
{ };


// Compute Resource (used to perform computation)
class ComputeResource :
	public IResource
{ };
*/

enum ProcessType
{
	PT_LoadBalancer,
	PT_Resource
};

enum ResourceType
{
	PT_Coordinator,
	PT_Worker
};

ProcessType GetProcessType(int rank, int argc, char **argv)
{
	return (rank) ? PT_Resource : PT_LoadBalancer; 
}

struct Message
{
	int CommandID;
	int CommandValue;
};

struct LoadBalancerMessage
{
	enum TypeID
	{
		Nop,
		Split,
		Merge
	};
};

struct ResourceMessage
{
	enum TypeID
	{
		Nop,
		Response_Split,
		Response_Merge
	};
};

struct TaskGroup
{
	MPI_Comm Communicator;
	MPI_Group Group;

	int Size;

	TaskGroup(void)
		: Communicator(-1)
		, Group(-1)
		, Size(0)
	{ }

	void Duplicate(TaskGroup *p_group)
	{
		std::cout << "Communicator :: " << Communicator << std::endl;
		MPI_Comm_create(Communicator, Group, &p_group->Communicator);
		MPI_Comm_group(p_group->Communicator, &p_group->Group);
		p_group->Size = Size;
	}

	void Exclude(TaskGroup *p_subgroup, int p_nRank)
	{
		MPI_Group_excl(Group, 1, &p_nRank, &p_subgroup->Group);
		MPI_Group_size(p_subgroup->Group, &p_subgroup->Size);
		MPI_Comm_create(Communicator, p_subgroup->Group, &p_subgroup->Communicator);
	}

	void Exclude(TaskGroup *p_subgroup, int p_nStartRank, int p_nEndRank, bool p_bIncludeRankZero = true)
	{
		int range[1][3];
		range[0][0] = p_nStartRank; range[0][1] = 1; range[0][2] = p_nEndRank; 
		range[1][0] = 0; range[1][1] = 1; range[1][2] = 0;

		// Other group -> {0, start rank .. end rank}
		MPI_Group_range_incl(Group, p_bIncludeRankZero ? 1 : 2, range, &p_subgroup->Group); 
		MPI_Group_size(p_subgroup->Group, &p_subgroup->Size);

		MPI_Group tempGroup;
		MPI_Group_range_excl(Group, p_bIncludeRankZero ? 2 : 1, range, &tempGroup);
		MPI_Group_size(Group, &Size);
 
//		MPI_Group_range_excl(Group, 1, range, &p_subgroup->Group);
//		MPI_Group_size(p_subgroup->Group, &p_subgroup->Size);
//		MPI_Comm_create(Communicator, p_subgroup->Group, &p_subgroup->Communicator);
	}

	void Merge(TaskGroup *p_subgroup)
	{
		MPI_Group_intersection(Group, p_subgroup->Group, &Group);
		MPI_Group_size(Group, &Size);

		int rank;
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);
		std::cout << "Rank : " << rank << ", Size : " << Size << std::endl;
	}

	void Split(TaskGroup *p_subgroup, int p_nStartRank, int p_nEndRank, bool p_bZeroRankInBoth = true)
	{
		int range[2][3];
		range[0][0] = p_nStartRank; range[0][1] = p_nEndRank; range[0][2] = 1;
		range[1][0] = 0; range[1][1] = 0; range[1][2] = 1;

		int rank;
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);

		std::cout << "Rank : " << rank << ", Split : " << p_nStartRank << " : " << p_nEndRank << std::endl;

		MPI_Group_range_incl(Group, 2, range, &p_subgroup->Group);
		MPI_Group_size(p_subgroup->Group, &p_subgroup->Size);

		MPI_Group_range_excl(Group, 1, range, &Group);
		MPI_Group_size(Group, &Size);
	}

	~TaskGroup(void)
	{ }
};

TaskGroup *CreateTaskGroup(TaskGroup &p_resourceTaskGroup, TaskGroup &p_idleTaskGroup, int p_exclusionCount)
{
	TaskGroup *taskGroup = new TaskGroup();

	p_idleTaskGroup.Split(taskGroup, 1, p_exclusionCount);

	return taskGroup;
}

void MergeTaskGroup(TaskGroup &p_sourceGroup, TaskGroup &p_mergeGroup)
{
	p_sourceGroup.Merge(&p_mergeGroup);
}

void InitialiseGroups(TaskGroup &p_worldTaskGroup, TaskGroup &p_resourceTaskGroup, TaskGroup &p_idleTaskGroup)
{
	std::cout << "Initialising World TaskGroup...";
	MPI_Comm_dup(MPI_COMM_WORLD, &p_worldTaskGroup.Communicator);
	MPI_Comm_group(p_worldTaskGroup.Communicator, &p_worldTaskGroup.Group);
	MPI_Group_size(p_worldTaskGroup.Group, &p_worldTaskGroup.Size);
	std::cout << "OK!" << std::endl;

	std::cout << "Initialising Resource TaskGroup...";
	p_worldTaskGroup.Duplicate(&p_resourceTaskGroup);
	//p_worldTaskGroup.Exclude(&p_resourceTaskGroup, 0);	
	std::cout << "OK!" << std::endl;
	
	std::cout << "Initialising Idle TaskGroup...";
	p_worldTaskGroup.Duplicate(&p_idleTaskGroup);
	//p_worldTaskGroup.Exclude(&p_idleTaskGroup, 0);
	std::cout << "OK!" << std::endl;
}

void LoadBalancer(void)
{
	// Create initial resource group by excluding load balancer
	std::vector<TaskGroup*> taskGroupList;

	TaskGroup worldTaskGroup, 
		resourceTaskGroup,
		idleTaskGroup;

	// Spawn parameters
	int *errorCodeList = new int[8];
	char portName[MPI_MAX_PORT_NAME];

	// Open communication port
	std::cout << "[LoadBalancer] :: Opening Communication Port" << std::endl;
	MPI_Open_port(MPI_INFO_NULL, portName);

	// Publish communication port
	std::cout << "[LoadBalancer] :: Publishing Port" << std::endl;
	MPI_Publish_name("LoadBalancer", MPI_INFO_NULL, portName);
	
	// Synchronise processes :: Checkpoint 01
	MPI_Barrier(MPI_COMM_WORLD);
	
	// Initialise Groups
	InitialiseGroups(worldTaskGroup, resourceTaskGroup, idleTaskGroup);
	std::cout << "[LoadBalancer] :: Resource pool size = " << resourceTaskGroup.Size << " of " << worldTaskGroup.Size << std::endl;

	// Synchronise processes :: Checkpoint 02
	MPI_Barrier(MPI_COMM_WORLD);
	std::cout << "[LoadBalancer] :: Online" << std::endl;

	// Load balancer loop
	while(true)
	{
		// Wait 
		boost::this_thread::sleep(boost::posix_time::seconds(1));

		// Simulate a request
		int requestSize = rand() % 2 + 2;
		std::cout << "[LoadBalancer] :: Incoming request [n = " << requestSize << "]" << std::endl;
		std::cout << "[LoadBalancer] :: Idle PEs [n = " << idleTaskGroup.Size << "]" << std::endl; 

		// We do not have enough PEs to honour request 
		if (requestSize > idleTaskGroup.Size)
		{
			// ... Ignore for now
			std::cout << "[LoadBalancer] :: Not enough PEs to satisfy request: declining." << std::endl;
		}
		else
		{
			// ... Split resource group
			std::cout << "[LoadBalancer] :: Initialising new group for task." << std::endl;
			
			Message split;
			
			split.CommandID = LoadBalancerMessage::Split;
			split.CommandValue = requestSize;

			MPI_Bcast(&split, sizeof(Message), MPI_CHAR, 0, idleTaskGroup.Communicator);
			std::cout << "[LoadBalancer] :: Paritioning idle resource group." << std::endl;

			MPI_Barrier(idleTaskGroup.Communicator);

			TaskGroup *workerTaskGroup = NULL;
			workerTaskGroup = CreateTaskGroup(resourceTaskGroup, idleTaskGroup, requestSize);
			taskGroupList.push_back(workerTaskGroup);
		}
	}

	// Unpublish name
	std::cout << "[LoadBalancer] :: Unpublishing port name..." << std::endl;
	MPI_Unpublish_name("LoadBalancer", MPI_INFO_NULL, portName);

	// Close communication port
	std::cout << "[LoadBalancer] :: Closing Port..." << std::endl;
	MPI_Close_port(portName);

	// Clean up
	delete[] errorCodeList;
}

void Idle(void)
{
	// Synchronise processes :: Checkpoint 01
	MPI_Barrier(MPI_COMM_WORLD);
	std::vector<TaskGroup*> taskGroupList;

	TaskGroup worldTaskGroup, 
		resourceTaskGroup,
		idleTaskGroup;
	
	// Initialise Groups
	InitialiseGroups(worldTaskGroup, resourceTaskGroup, idleTaskGroup);
	std::cout << "[Idle Worker] :: Resource pool size = " << resourceTaskGroup.Size << " of " << worldTaskGroup.Size << std::endl;

	// Synchronise processes:: Checkpoint 02
	MPI_Barrier(MPI_COMM_WORLD);
	std::cout << "[Idle Worker] :: Online" << std::endl;

	MPI_Status status;
	MPI_Request request;
	Message split;

	TaskGroup *thisTaskGroup;

	while (true)
	{
		std::cout << "[Idle Worker] :: " << std::endl;
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));

		MPI_Bcast(&split, sizeof(Message), MPI_CHAR, 0, idleTaskGroup.Communicator);
		std::cout << "[Idle Worker] :: Received message from Load Balancer." << std::endl;

		// Wait till every idle task in the group has received command
		MPI_Barrier(idleTaskGroup.Communicator);

		if (split.CommandID == LoadBalancerMessage::Split)
		{
			//TaskGroup *workerTaskGroup = NULL;
			//workerTaskGroup = CreateTaskGroup(resourceTaskGroup, idleTaskGroup, split.CommandValue);
			thisTaskGroup = CreateTaskGroup(resourceTaskGroup, idleTaskGroup, split.CommandValue);
			//taskGroupList.push_back(workerTaskGroup);
		} 
		else if (split.CommandID == LoadBalancerMessage::Merge)
		{
			MergeTaskGroup(idleTaskGroup, thisTaskGroup);
		}
	}
}

//void Coordinator(void)
//{
//	MPI_Request request;
//	MPI_Status status;
//
//	// Get parent communicator and wait at barrier
//	MPI_Comm parent,
//		intercomm,
//		intracomm; 
//
//	int minimum = 2,
//		flag;
//
//	char portName[MPI_MAX_PORT_NAME],
//		req;
//
//	// Get parent and block at barrier
//	MPI_Comm_get_parent(&parent);
//	MPI_Barrier(parent);
//
//	// Lookup port name
//	MPI_Lookup_name("LoadBalancer", MPI_INFO_NULL, portName);
//
//	// Connect to port
//	std::cout << "[Coordinator] :: Connecting to load balancer port" << std::endl;
//	MPI_Comm_connect(portName, MPI_INFO_NULL, 0, MPI_COMM_SELF, &intercomm);
//
//	// Coordinator is loaded
//	std::cout << "[Coordinator] :: Started" << std::endl;
//
//	while (true)
//	{
//		// Check mail from load balancer
//		MPI_Irecv(&req, 1, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, intercomm, &request);
//		std::cout << "[Coordinator] :: Receive started, flag == 0" << std::endl;
//
//		for(flag = 0; !flag;)
//		{
//			MPI_Test(&request, &flag, &status);
//			boost::this_thread::sleep(boost::posix_time::milliseconds(250));
//		}
//
//		std::cout << "[Coordinator] :: Received command from LoadBalancer, flag == " << flag << std::endl;
//		std::cout << "[Coordinator] :: Command Id = [" << std::hex << (int)req << std::dec  << "]" << std::endl;
//
//		// Request for resizing group:
//		// 1. If group can be resized, split communicator
//		// 2. If group cannot be resized, consider putting processes to sleep
//		int size; MPI_Comm_size(MPI_COMM_WORLD, &size);
//		
//		// Can't really reduce group size further
//		if (size <= minimum)
//		{
//			std::cout << "[Coordinator] :: Communicator size = " << size << std::endl;
//			std::cout << "[Coordinator] :: Reduce request ignored" << std::endl;
//		}
//		else
//		{
//			std::cout << "[Coordinator] :: Communicator size = " << size << std::endl;
//			std::cout << "[Coordinator] :: Reduce request granted" << std::endl;
//
//			// Now we need to determine which colour will each member of the communicator be
//			// For the time being, we assume the communicator will be split in two groups,
//			// such that the group rank vector is <(N-1), (1)>
//			//
//			// We assume reduction checkpoints : the worker pipeline is not fully pre-emptive
//
//			// Thus, the PE with rank [size - 1] will hold a different colour
//			// than other PEs... based on rank.
//			WorkerCommand workerCommand;
//			workerCommand.CommandID = WorkerCommand::Split;
//			workerCommand.CommandValue = size - 1;
//
//			MPI_Send(&workerCommand, sizeof(WorkerCommand), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
//		}
//		
//		// Output service tag
//		std::cout << "[C.] :: " << std::endl;
//		boost::this_thread::sleep(boost::posix_time::milliseconds(500));
//	}
//
//	// Disconnect port
//	MPI_Comm_disconnect(&intercomm);	
//}
//
//void Worker(void)
//{
//	WorkerCommand workerCommand;
//
//	// Get parent communicator and wait at barrier
//	MPI_Comm parent; 
//
//	MPI_Comm_get_parent(&parent);
//	MPI_Barrier(parent);
//
//	// Worker is loaded
//	std::cout << "[Worker]" << std::endl;
//
//
//	while (true)
//	{
//		std::cout << "[W.] :: " << std::endl;
//		boost::this_thread::sleep(boost::posix_time::milliseconds(500));
//
//		MPI_Recv(&workerCommand, sizeof(WorkerCommand), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD);
//		
//		if (workerCommand.CommandID == WorkerCommand::Split)
//		{
//			MPI_Comm_split(MPI_COMM_WORLD, workerCommand.CommandID >= rank ? 1 : MPI_UNDEFINED, 0, &splitcomm);
//			MPI_Intercomm_merge(splitcomm,,&intracomm);
//		}
//	}
//}

/*
ProcessType GetProcessType(int rank, int argc, char **argv)
{
	// Rank != 0 means this is a worker
	if (rank != 0)
		return PT_Worker;

	MPI_Comm parent; MPI_Comm_get_parent(&parent);

	return parent == MPI_COMM_NULL ? PT_LoadBalancer 
		: PT_Coordinator;
}

void LoadBalancer(void)
{
	// Spawn parameters
	int processPoolSize[] = {2, 4};
	int *errorCodeList = new int[8];
	char portName[MPI_MAX_PORT_NAME];

	MPI_Comm intercomm[2];

	// Open communication port
	std::cout << "[LoadBalancer] :: Opening Communication Port" << std::endl;
	MPI_Open_port(MPI_INFO_NULL, portName);

	// Publish communication port
	std::cout << "[LoadBalancer] :: Publishing Port" << std::endl;
	MPI_Publish_name("LoadBalancer", MPI_INFO_NULL, portName);
	 
	// Spawn process pools
	for (int commIdx = 0; commIdx < 2; ++commIdx)
	{
		std::cout << "[LoadBalancer] :: Spawing processes [n = " << processPoolSize[commIdx] << "]" << std::endl;
		MPI_Comm_spawn("Sandbox.exe", NULL, processPoolSize[commIdx], MPI_INFO_NULL, 0, MPI_COMM_WORLD, &intercomm[commIdx], errorCodeList);
		
		// Block until all spawned processes have loaded
		MPI_Barrier(intercomm[commIdx]);

		std::cout << "[LoadBalancer] :: Accepting communication on port" << std::endl;
        MPI_Comm_accept(portName, MPI_INFO_NULL, 0, MPI_COMM_SELF, &intercomm[commIdx]);
	}
	
	// LB Started
	std::cout << "[LoadBalancer] :: Started" << std::endl;
	
	// ...
	// Request move from comm A to comm B

	// ...
	MPI_Status status;
	MPI_Request request;
	int flag = 0;
	char req = 0;

	while(true)
	{
		req = (char)(rand() % 2);

		MPI_Isend(&req, 1, MPI_CHAR, 0, 0, intercomm[req], &request); 
		boost::this_thread::sleep(boost::posix_time::seconds(1));

		std::cout << "[LoadBalancer] :: Send started, flag == 0" << std::endl;

		for(flag = 0; !flag;)
		{
			MPI_Test(&request, &flag, &status);
			boost::this_thread::sleep(boost::posix_time::milliseconds(250));
		}

		std::cout << "[LoadBalancer] :: Send completed, flag == " << flag << std::endl;
	}

	// ...

	// Sleep and terminate
	boost::this_thread::sleep(boost::posix_time::seconds(4));
	std::cout << "[L] :: Terminating..." << std::endl;

	// Disconnect intercommunicators
	for (int commIdx = 0; commIdx < 2; ++commIdx)
	{
		MPI_Comm_disconnect(&intercomm[commIdx]);
	}

	// Unpublish name
	std::cout << "[LoadBalancer] :: Unpublishing port name..." << std::endl;
	MPI_Unpublish_name("LoadBalancer", MPI_INFO_NULL, portName);

	// Close communication port
	std::cout << "[LoadBalancer] :: Closing Port..." << std::endl;
	MPI_Close_port(portName);

	// Clean up
	delete[] errorCodeList;
}

struct WorkerCommand
{
	enum TypeID
	{
		Nop,
		Split
	};

	char CommandID;
	char CommandValue;
};

void Coordinator(void)
{
	MPI_Request request;
	MPI_Status status;

	// Get parent communicator and wait at barrier
	MPI_Comm parent,
		intercomm,
		intracomm; 

	int minimum = 2,
		flag;

	char portName[MPI_MAX_PORT_NAME],
		req;

	// Get parent and block at barrier
	MPI_Comm_get_parent(&parent);
	MPI_Barrier(parent);

	// Lookup port name
	MPI_Lookup_name("LoadBalancer", MPI_INFO_NULL, portName);

	// Connect to port
	std::cout << "[Coordinator] :: Connecting to load balancer port" << std::endl;
	MPI_Comm_connect(portName, MPI_INFO_NULL, 0, MPI_COMM_SELF, &intercomm);

	// Coordinator is loaded
	std::cout << "[Coordinator] :: Started" << std::endl;

	while (true)
	{
		// Check mail from load balancer
		MPI_Irecv(&req, 1, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, intercomm, &request);
		std::cout << "[Coordinator] :: Receive started, flag == 0" << std::endl;

		for(flag = 0; !flag;)
		{
			MPI_Test(&request, &flag, &status);
			boost::this_thread::sleep(boost::posix_time::milliseconds(250));
		}

		std::cout << "[Coordinator] :: Received command from LoadBalancer, flag == " << flag << std::endl;
		std::cout << "[Coordinator] :: Command Id = [" << std::hex << (int)req << std::dec  << "]" << std::endl;

		// Request for resizing group:
		// 1. If group can be resized, split communicator
		// 2. If group cannot be resized, consider putting processes to sleep
		int size; MPI_Comm_size(MPI_COMM_WORLD, &size);
		
		// Can't really reduce group size further
		if (size <= minimum)
		{
			std::cout << "[Coordinator] :: Communicator size = " << size << std::endl;
			std::cout << "[Coordinator] :: Reduce request ignored" << std::endl;
		}
		else
		{
			std::cout << "[Coordinator] :: Communicator size = " << size << std::endl;
			std::cout << "[Coordinator] :: Reduce request granted" << std::endl;

			// Now we need to determine which colour will each member of the communicator be
			// For the time being, we assume the communicator will be split in two groups,
			// such that the group rank vector is <(N-1), (1)>
			//
			// We assume reduction checkpoints : the worker pipeline is not fully pre-emptive

			// Thus, the PE with rank [size - 1] will hold a different colour
			// than other PEs... based on rank.
			WorkerCommand workerCommand;
			workerCommand.CommandID = WorkerCommand::Split;
			workerCommand.CommandValue = size - 1;

			MPI_Send(&workerCommand, sizeof(WorkerCommand), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
		}
		
		// Output service tag
		std::cout << "[C.] :: " << std::endl;
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));
	}

	// Disconnect port
	MPI_Comm_disconnect(&intercomm);	
}

void Worker(void)
{
	WorkerCommand workerCommand;

	// Get parent communicator and wait at barrier
	MPI_Comm parent; 

	MPI_Comm_get_parent(&parent);
	MPI_Barrier(parent);

	// Worker is loaded
	std::cout << "[Worker]" << std::endl;


	while (true)
	{
		std::cout << "[W.] :: " << std::endl;
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));

		MPI_Recv(&workerCommand, sizeof(WorkerCommand), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD);
		
		if (workerCommand.CommandID == WorkerCommand::Split)
		{
			MPI_Comm_split(MPI_COMM_WORLD, workerCommand.CommandID >= rank ? 1 : MPI_UNDEFINED, 0, &splitcomm);
			MPI_Intercomm_merge(splitcomm,,&intracomm);
		}
	}
}
*/


void TestScheduler(int argc, char **argv)
{
	// Initialise MPI
	MPI_Init(&argc, &argv);
	
	// Get Process Rank
	int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// We need to detect whether this is running as load balancer, coordinator or worker
	ProcessType processType = GetProcessType(rank, argc, argv);

	switch(processType)
	{
		case PT_LoadBalancer:
			LoadBalancer();
			break;
		case PT_Resource:
			Idle();
			break;
		default:
			break;
	}

	// Finalise MPI
	MPI_Finalize();

	/*
	// Initialise MPI
	MPI_Init(&argc, &argv);
	
	// Get Process Rank
	int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// We need to detect whether this is running as load balancer, coordinator or worker
	ProcessType processType = GetProcessType(rank, argc, argv);

	switch(processType)
	{
		case PT_LoadBalancer:
			LoadBalancer();
			break;
		case PT_Coordinator:
			Coordinator();
			break;
		case PT_Worker:
			Worker();
			break;
		default:
			break;
	}
	*/

	// Finalise MPI
	MPI_Finalize();
}