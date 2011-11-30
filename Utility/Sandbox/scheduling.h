#pragma once

#include <map>
#include <vector>

#include <boost/thread.hpp>
#include <boost/mpi.hpp>
namespace mpi = boost::mpi;

#include "../../Core/Object/Object.h"

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

	~TaskGroup(void)
	{ }

	bool IsMember(void)
	{
		int processRank;

		MPI_Group_rank(Group, &processRank);
		
		return processRank != MPI_UNDEFINED;
	}
	
	void Duplicate(TaskGroup *p_group)
	{
		MPI_Comm_create(Communicator, Group, &p_group->Communicator);
		MPI_Comm_group(p_group->Communicator, &p_group->Group);
		p_group->Size = Size;
	}

	void IMerge(int p_high)
	{
		MPI_Intercomm_merge(Communicator, p_high, &Communicator);
		MPI_Comm_group(Communicator, &Group);
		MPI_Group_size(Group, &Size);
	}

	void PartialMerge(TaskGroup *p_subgroup, int p_mergeSize)
	{
		int rank = this->GetRankInMainGroup();
		std::cout << "[" << rank << "] :: PartialMerge :: [This size = " << Size << ", Merge size = " << p_mergeSize << "]" << std::endl;

		// We wish to merge a group to a partial group
		// idle + partial(subgroup)

		// Split sub group
		TaskGroup splitGroup;
		p_subgroup->Split(&splitGroup, p_mergeSize);

		// Merge sub group to this group
		this->Merge(&splitGroup);

		// Partial merge ok
		std::cout << "[" << rank << "] :: Partial merge successful :: [This size = " << this->Size << ", Merge size = " << p_subgroup->Size << "]" << std::endl;
	}

	void Merge(TaskGroup *p_subgroup)
	{
		int rank = this->GetRankInMainGroup();
		std::cout << "[" << rank << "] :: Merge :: [This size = " << Size << ", Merge size = " << p_subgroup->Size << "]" << std::endl;

		MPI_Group_union(Group, p_subgroup->Group, &Group);
		MPI_Group_size(Group, &Size);
		
		if (this->IsMember())
		{
			MPI_Comm_create(MPI_COMM_WORLD, this->Group, &this->Communicator);
		} else {
			MPI_Comm_create(MPI_COMM_WORLD, MPI_GROUP_EMPTY, &this->Communicator);
		}

		if (p_subgroup->IsMember())
		{
			MPI_Group_free(&p_subgroup->Group);
			MPI_Comm_free(&p_subgroup->Communicator);
		}

		std::cout << "[" << rank << "] :: Merge successful :: [This size = " << Size << "]" << std::endl;
	}

	void Split(TaskGroup *p_subgroup, int p_splitSize)
	{
		int rank = this->GetRankInMainGroup();
		std::cout << "[" << rank << "] :: Split :: Before Barrier" << std::endl;		
		MPI_Barrier(MPI_COMM_WORLD);
		std::cout << "[" << rank << "] :: Split :: After Barrier" << std::endl;		

		std::cout << "[" << rank << "] :: Split :: [This size = " << Size <<", Split size = " << p_splitSize << "]" << std::endl;		

		// Determine rank zero in current group
		int rankZero = this->GetRankZero();
		
		// Rank zero is within specified range
		if (rankZero > 0 && rankZero < p_splitSize)
		{
			int leftBound = rankZero - 1,
				rightBound = p_splitSize;

			int range[3][3];

			range[0][0] = 0; range[0][1] = leftBound; range[0][2] = 1;
			range[1][0] = rankZero + 1; range[1][1] = rightBound; range[1][2] = 1;
			range[2][0] = rankZero, range[2][1] = rankZero; range[2][2] = 1;

			std::cout << "[" << rank << "] :: Split ranges :: [ " << 
				range[0][0] << "," << range[0][1] << "," << range[0][2] << "," <<
				range[1][0] << "," << range[1][1] << "," << range[1][2] << "," <<
				range[2][0] << "," << range[2][1] << "," << range[2][2] << "]" << std::endl; 

			MPI_Group_range_incl(Group, 3, range, &p_subgroup->Group);
			MPI_Group_range_excl(Group, 2, range, &Group);
		}
		else
		{
			int range[2][3];

			if (!rankZero) { range[0][0] = 1; range[0][1] = p_splitSize; range[0][2] = 1; }
			else { range[0][0] = 0; range[0][1] = p_splitSize - 1; range[0][2] = 1; }
			range[1][0] = rankZero; range[1][1] = rankZero; range[1][2] = 1;

			std::cout << "[" << rank << "] :: Split ranges :: [ " << 
				range[0][0] << "," << range[0][1] << "," << range[0][2] << "," <<
				range[1][0] << "," << range[1][1] << "," << range[1][2] << "]" << std::endl; 
			
			MPI_Group_range_incl(Group, 2, range, &p_subgroup->Group);
			MPI_Group_range_excl(Group, 1, range, &Group);
		}

		std::cout << "[" << rank << "] :: " << std::hex << this->Group << ", " << p_subgroup->Group << std::dec << std::endl;

		if (this->IsMember())
		{
			MPI_Comm_create(MPI_COMM_WORLD, this->Group, &this->Communicator);
		} else {
			MPI_Comm_create(MPI_COMM_WORLD, MPI_GROUP_EMPTY, &this->Communicator);
		}

		if (p_subgroup->IsMember())
		{
			MPI_Comm_create(MPI_COMM_WORLD, p_subgroup->Group, &p_subgroup->Communicator);
		} else {
			MPI_Comm_create(MPI_COMM_WORLD, MPI_GROUP_EMPTY, &p_subgroup->Communicator);
		}

		// Update group size
		MPI_Group_size(p_subgroup->Group, &p_subgroup->Size);
		MPI_Group_size(Group, &Size);

		std::cout << "[" << rank << "] :: Split Successful :: [This size = " << Size << ", Split Size = " << p_subgroup->Size << "]" << std::endl;
	}

	int GetRankInGroup(void)
	{
		int rank; MPI_Group_rank(Group, &rank);
		return rank;
	}

	int GetRankInMainGroup(void)
	{
		int rank; MPI_Group_rank(GetMainGroup(), &rank);
		return rank;
	}

	int GetRankZero(void)
	{
		int rankZero = 0, 
			rankZeroTranslated;

		MPI_Group group;
		MPI_Comm_group(MPI_COMM_WORLD, &group);
		MPI_Group_translate_ranks(group, 1, &rankZero, Group, &rankZeroTranslated);

		return rankZeroTranslated;
	}

	MPI_Group GetMainGroup(void)
	{
		MPI_Group group;
		MPI_Comm_group(MPI_COMM_WORLD, &group);
		return group;
	}
};

TaskGroup *CreateTaskGroup(TaskGroup &p_idleTaskGroup, int p_taskGroupSize)
{
	TaskGroup *taskGroup = new TaskGroup();
	p_idleTaskGroup.Split(taskGroup, p_taskGroupSize);
	return taskGroup;
}

void MergeTaskGroup(TaskGroup &p_sourceGroup, TaskGroup &p_mergeGroup)
{
	p_sourceGroup.Merge(&p_mergeGroup);
}

void PartialMergeTaskGroup(TaskGroup &p_sourceGroup, TaskGroup &p_mergeGroup, int p_mergeSize)
{
	p_sourceGroup.PartialMerge(&p_mergeGroup, p_mergeSize);
}

int GetRank(void)
{
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	return rank;
}

void InitialiseGroups(TaskGroup &p_worldTaskGroup, TaskGroup &p_idleTaskGroup)
{
	std::cout << "Initialising World TaskGroup..." << std::endl;
	MPI_Comm_dup(MPI_COMM_WORLD, &p_worldTaskGroup.Communicator);
	MPI_Comm_group(p_worldTaskGroup.Communicator, &p_worldTaskGroup.Group);
	MPI_Group_size(p_worldTaskGroup.Group, &p_worldTaskGroup.Size);
	
	std::cout << "Initialising Idle TaskGroup..." << std::endl;
	p_worldTaskGroup.Duplicate(&p_idleTaskGroup);
}

void LoadBalancer(void)
{
	int rank = GetRank();

	// Create initial resource group by excluding load balancer
	std::vector<TaskGroup*> taskGroupList;

	// Keep main task groups handy
	TaskGroup worldTaskGroup, 
		idleTaskGroup;

#pragma region non-ms
	/*
	// Spawn parameters
	// int *errorCodeList = new int[8];
	// char portName[MPI_MAX_PORT_NAME];

	// Open communication port (MSMPI doesn't support this)
	//std::cout << "[LoadBalancer] :: Opening Communication Port" << std::endl;
	//MPI_Open_port(MPI_INFO_NULL, portName);

	// Publish communication port
	// std::cout << "[LoadBalancer] :: Publishing Port" << std::endl;
	// MPI_Publish_name("LoadBalancer", MPI_INFO_NULL, portName);
	*/
#pragma endregion

	// Synchronise processes :: Checkpoint 01
	MPI_Barrier(MPI_COMM_WORLD);
	
	// Initialise Groups
	InitialiseGroups(worldTaskGroup, idleTaskGroup);
	std::cout << "[" << rank << "] :: [LoadBalancer] :: Online" << std::endl;
	std::cout << "[" << rank << "] :: [LoadBalancer] :: Pool :: [Size = " << idleTaskGroup.Size - 1 << "]" << std::endl;

	// Synchronise processes :: Checkpoint 02
	MPI_Barrier(MPI_COMM_WORLD);

	// Load balancer loop
	while(true)
	{
		// Wait 
		boost::this_thread::sleep(boost::posix_time::seconds(1));

		// Simulate a request
		int requestSize = rand() % 2 + 2;
		std::cout << "[" << rank << "] :: [LoadBalancer] :: Incoming request :: [" << requestSize << "]" << std::endl;
		std::cout << "[" << rank << "] :: [LoadBalancer] :: Available :: [" << idleTaskGroup.Size - 1 << "]" << std::endl; 

		// We do not have enough PEs to honour request (remember, idle task group includes load balancer)
		if (requestSize >= idleTaskGroup.Size)
		{
			std::cout << "[" << rank << "] :: [LoadBalancer] :: Unable to satisfy request, starting Merge..." << std::endl;

			Message merge;

			merge.CommandID = LoadBalancerMessage::Merge;
			merge.CommandValue = taskGroupList.size() - 1;

			for (std::vector<TaskGroup*>::iterator groupIterator = taskGroupList.begin();
				 groupIterator != taskGroupList.end(); ++groupIterator)
			{
				MPI_Bcast(&merge, sizeof(Message), MPI_CHAR, (*groupIterator)->GetRankZero(), (*groupIterator)->Communicator);
			}
			
			MPI_Bcast(&merge, sizeof(Message), MPI_CHAR, idleTaskGroup.GetRankZero(), idleTaskGroup.Communicator);
			
			TaskGroup *taskGroup = taskGroupList.back(); 	
			//taskGroupList.pop_back(); 
			//MergeTaskGroup(idleTaskGroup, *taskGroup);
			PartialMergeTaskGroup(idleTaskGroup, *taskGroup, 1);

			std::cout << "[" << rank << "] :: [LoadBalancer] :: Merge successful, Available :: [" << idleTaskGroup.Size << "]" << std::endl;
		}
		else
		{
			std::cout << "[" << rank << "] :: [LoadBalancer] :: Paritioning idle resource group..." << std::endl;

			Message msg;
			
			msg.CommandID = LoadBalancerMessage::Split;
			msg.CommandValue = requestSize;
			
			for (std::vector<TaskGroup*>::iterator groupIterator = taskGroupList.begin();
				 groupIterator != taskGroupList.end(); ++groupIterator)
			{
				MPI_Bcast(&msg, sizeof(Message), MPI_CHAR, (*groupIterator)->GetRankZero(), (*groupIterator)->Communicator);
			}

			MPI_Bcast(&msg, sizeof(Message), MPI_CHAR, idleTaskGroup.GetRankZero(), idleTaskGroup.Communicator);

			// Create new task group and push it back
			taskGroupList.push_back(CreateTaskGroup(idleTaskGroup, requestSize));

			std::cout << "[" << rank << "] :: [LoadBalancer] :: Partitioning successful :: [Available = " << idleTaskGroup.Size << ", Paritition = " << taskGroupList.back()->Size << "]" << std::endl;
		}
	}

#pragma region non-ms
	// Unpublish name
	// std::cout << "[LoadBalancer] :: Unpublishing port name..." << std::endl;
	// MPI_Unpublish_name("LoadBalancer", MPI_INFO_NULL, portName);

	// Close communication port
	// std::cout << "[LoadBalancer] :: Closing Port..." << std::endl;
	// MPI_Close_port(portName);

	// Clean up
	// delete[] errorCodeList;
#pragma endregion
}

struct TaskGroupContext
{
	std::vector<TaskGroup*> *taskGroupList;
	TaskGroup* idleTaskGroup;
	TaskGroup* taskGroup;
};

bool HandleLoadBalancerRequests(TaskGroupContext *p_context, TaskGroup *p_taskGroup);

void Worker(TaskGroupContext *p_context)
{
	int rank = GetRank();
	std::cout << "[" << rank << "] :: [Worker] :: Online" << std::endl;

	while (true)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));

		if (HandleLoadBalancerRequests(p_context, p_context->taskGroup))
		{
			std::cout << "[" << rank << "] :: [Worker] :: Exiting..." << std::endl;
			return;
		}
	}
}

void Coordinator(TaskGroupContext *p_context)
{
	int rank = GetRank();
	std::cout << "[" << rank << "] :: [Coordinator] :: Online" << std::endl;

	while (true)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));

		if (HandleLoadBalancerRequests(p_context, p_context->taskGroup))
		{
			std::cout << "[" << rank << "] :: [Coordinator] :: Exiting..." << std::endl;
			return;
		}
	}
}

bool HandleLoadBalancerRequests(TaskGroupContext *p_context, TaskGroup *p_taskGroup)
{
	int rank = GetRank();

	Message msg;

	MPI_Bcast(&msg, sizeof(Message), MPI_CHAR, p_taskGroup->GetRankZero(), p_taskGroup->Communicator);
	std::cout << "[" << rank << "] :: [HandleLBRequest] :: Received message from Load Balancer : CMDID = [" << msg.CommandID << "]" << std::endl;

	switch (msg.CommandID)
	{
		case LoadBalancerMessage::Split:
		{
			std::cout << "[" << rank << "] :: [HandleLBRequest] :: Split :: CMDVAL = [" << msg.CommandValue << "]" << std::endl;

			p_context->taskGroup = CreateTaskGroup(*(p_context->idleTaskGroup), msg.CommandValue);
			p_context->taskGroupList->push_back(p_context->taskGroup);

			// We need to detect whether we are still part of the idle group, or the assigned group
			// If idle, we keep running idle(), otherwise we go for worker or coordinator
			if (p_context->taskGroup->IsMember())
			{
				if (p_context->taskGroup->GetRankInGroup() == 0)
				{
					Coordinator(p_context);
				}
				else
				{
					Worker(p_context);
				}
			}
			break;
		}

		case LoadBalancerMessage::Merge:
		{
			std::cout << "[" << rank << "] :: [HandleLBRequest] :: Merge :: CMDVAL = [" << msg.CommandValue << "]" << std::endl;

			int subgroupIndex = msg.CommandValue;
			
			if (subgroupIndex < p_context->taskGroupList->size())
			{
				TaskGroup *taskGroup = p_context->taskGroupList->at(subgroupIndex);

				std::vector<TaskGroup*>::iterator position = std::find(p_context->taskGroupList->begin(), p_context->taskGroupList->end(), taskGroup);
				if (position != p_context->taskGroupList->end())
				{
					p_context->taskGroupList->erase(position);
					// MergeTaskGroup(*(p_context->idleTaskGroup), *taskGroup);
					PartialMergeTaskGroup(*(p_context->idleTaskGroup), *taskGroup, 1);
				}

				std::cout << "[" << rank << "] :: Is Member = " << p_context->idleTaskGroup->IsMember() << std::endl;
				return p_context->idleTaskGroup->IsMember();
				//return (taskGroup == p_context->taskGroup);
			}
			else
			{
				std::cout << "[" << rank << "] :: Merge Idle" << std::endl;

				MPI_Comm tempComm;
				MPI_Comm_create(MPI_COMM_WORLD, MPI_GROUP_EMPTY, &tempComm);
			}
		}

		default:
			break;
	}

	return false;
}

void Idle(void)
{
	int rank = GetRank();

	std::vector<TaskGroup*> taskGroupList;

	TaskGroup worldTaskGroup, 
		idleTaskGroup;
	
	// Synchronise processes :: Checkpoint 01
	MPI_Barrier(MPI_COMM_WORLD);

	// Initialise Groups
	InitialiseGroups(worldTaskGroup, idleTaskGroup);
	std::cout << "[" << rank << "] :: [Idle Worker] :: Resource pool size = " << idleTaskGroup.Size - 1 << " of " << worldTaskGroup.Size << std::endl;

	// Synchronise processes:: Checkpoint 02
	MPI_Barrier(MPI_COMM_WORLD);
	std::cout << "[" << rank << "] :: [Idle Worker] :: Online" << std::endl;

	MPI_Status status;
	MPI_Request request;
	Message msg;

	TaskGroup *thisTaskGroup;
	int rankZero = idleTaskGroup.GetRankZero();

	TaskGroupContext context;
	
	context.idleTaskGroup = &idleTaskGroup;
	context.taskGroupList = &taskGroupList;
	context.taskGroup = NULL;

	while (true)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));
		
		HandleLoadBalancerRequests(&context, context.idleTaskGroup);
	}
}

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
}