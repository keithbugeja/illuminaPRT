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
		int rank;
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);

		std::cout << "[Rank : " << rank << "] :: Merge [Size = " << Size << ", " << p_mergeSize << " of " << p_subgroup->Size << "]" << std::endl;

		// First divide subgroup
		TaskGroup group;
		p_subgroup->Split(&group, 1, p_mergeSize);

		MPI_Group_union(Group, group.Group, &Group);
		MPI_Group_size(Group, &Size);

		std::cout << "[Rank : " << rank << "] :: Merge [Size = " << Size << ", " << p_subgroup->Size << "]" << std::endl;

		if (this->IsMember())
			MPI_Comm_create(MPI_COMM_WORLD, this->Group, &this->Communicator);
		else 
			MPI_Comm_create(MPI_COMM_WORLD, MPI_GROUP_EMPTY, &this->Communicator);

		std::cout << "[Rank : " << rank << "] :: Merge successful." << std::endl;
	}

	void Merge(TaskGroup *p_subgroup)
	{
		int rank;
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);

		std::cout << "[Rank : " << rank << "] :: Merge [Size = " << Size << ", " << p_subgroup->Size << "]" << std::endl;

		MPI_Group_union(Group, p_subgroup->Group, &Group);
		MPI_Group_size(Group, &Size);

		std::cout << "[Rank : " << rank << "] :: Merge [New size = " << Size << "]" << std::endl;
		
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

		std::cout << "[Rank : " << rank << "] :: Merge successful." << std::endl;
	}

	void Split(TaskGroup *p_subgroup, int p_nStartRank, int p_nEndRank, bool p_bZeroRankInBoth = true)
	{
		int range[2][3];
		range[0][0] = p_nStartRank; range[0][1] = p_nEndRank; range[0][2] = 1;
		range[1][0] = 0; range[1][1] = 0; range[1][2] = 1;

		MPI_Group_range_incl(Group, 2, range, &p_subgroup->Group);
		MPI_Group_range_excl(Group, 1, range, &Group);

		// Update task group communicator
		MPI_Comm_create(Communicator, p_subgroup->Group, &p_subgroup->Communicator);
		MPI_Comm_create(Communicator, Group, &Communicator);

		// Update group size
		MPI_Group_size(p_subgroup->Group, &p_subgroup->Size);
		MPI_Group_size(Group, &Size);

		int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);
		std::cout << "[Rank : " << rank << "] :: Split [New size = " << Size << ", " << p_subgroup->Size << "]" << std::endl;
	}

	int GetRankInGroup(void)
	{
		int rank; MPI_Group_rank(Group, &rank);
		return rank;
	}

	MPI_Group GetRankZero(void)
	{
		int rankZero = 0, 
			rankZeroTranslated;

		MPI_Group group;
		MPI_Comm_group(MPI_COMM_WORLD, &group);
		MPI_Group_translate_ranks(group, 1, &rankZero, Group, &rankZeroTranslated);

		return rankZeroTranslated;
	}
};

TaskGroup *CreateTaskGroup(TaskGroup &p_idleTaskGroup, int p_taskGroupSize)
{
	TaskGroup *taskGroup = new TaskGroup();
	p_idleTaskGroup.Split(taskGroup, 1, p_taskGroupSize);
	return taskGroup;
}

void MergeTaskGroup(TaskGroup &p_sourceGroup, TaskGroup &p_mergeGroup)
{
	p_sourceGroup.Merge(&p_mergeGroup);
}

void InitialiseGroups(TaskGroup &p_worldTaskGroup, TaskGroup &p_idleTaskGroup)
{
	std::cout << "Initialising World TaskGroup...";
	MPI_Comm_dup(MPI_COMM_WORLD, &p_worldTaskGroup.Communicator);
	MPI_Comm_group(p_worldTaskGroup.Communicator, &p_worldTaskGroup.Group);
	MPI_Group_size(p_worldTaskGroup.Group, &p_worldTaskGroup.Size);
	
	std::cout << "Initialising Idle TaskGroup...";
	p_worldTaskGroup.Duplicate(&p_idleTaskGroup);
}

void LoadBalancer(void)
{
	// Create initial resource group by excluding load balancer
	std::vector<TaskGroup*> taskGroupList;

	// Keep main task groups handy
	TaskGroup worldTaskGroup, 
		idleTaskGroup;

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

	// Synchronise processes :: Checkpoint 01
	MPI_Barrier(MPI_COMM_WORLD);
	
	// Initialise Groups
	InitialiseGroups(worldTaskGroup, idleTaskGroup);
	std::cout << "[LoadBalancer] :: Resource pool size = " << idleTaskGroup.Size << " of " << worldTaskGroup.Size << std::endl;

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
		std::cout << "[LoadBalancer] :: Idle PEs [n = " << idleTaskGroup.Size - 1 << "]" << std::endl; 

		// We do not have enough PEs to honour request (remember, idle task group includes load balancer)
		if (requestSize >= idleTaskGroup.Size)
		{
			std::cout << "[LoadBalancer] :: Not enough PEs to satisfy request." << std::endl;

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
			
			//idleTaskGroup.PartialMerge(taskGroup, 1);
			
			taskGroupList.pop_back();
			MergeTaskGroup(idleTaskGroup, *taskGroup);
			
			std::cout << "[LoadBalancer] :: Merge successful, [Idle Size = " << idleTaskGroup.Size << "]" << std::endl;
		}
		else
		{
			std::cout << "[LoadBalancer] :: Paritioning idle resource group..." << std::endl;

			Message msg;
			
			msg.CommandID = LoadBalancerMessage::Split;
			msg.CommandValue = requestSize;

			MPI_Bcast(&msg, sizeof(Message), MPI_CHAR, idleTaskGroup.GetRankZero(), idleTaskGroup.Communicator);

			// Create new task group and push it back
			taskGroupList.push_back(CreateTaskGroup(idleTaskGroup, requestSize));

			std::cout << "[LoadBalancer] :: Partitioning successful, [" << idleTaskGroup.Size << ", " << taskGroupList.back()->Size << "]" << std::endl;
		}
	}

	// Unpublish name
	// std::cout << "[LoadBalancer] :: Unpublishing port name..." << std::endl;
	// MPI_Unpublish_name("LoadBalancer", MPI_INFO_NULL, portName);

	// Close communication port
	// std::cout << "[LoadBalancer] :: Closing Port..." << std::endl;
	// MPI_Close_port(portName);

	// Clean up
	// delete[] errorCodeList;
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
	std::cout << "[Worker] :: Online" << std::endl;

	while (true)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));

		if (HandleLoadBalancerRequests(p_context, p_context->taskGroup))
			return;
	}
}

void Coordinator(TaskGroupContext *p_context)
{
	std::cout << "[Coordinator] :: Online" << std::endl;

	while (true)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));

		if (HandleLoadBalancerRequests(p_context, p_context->taskGroup))
			return;
	}
}

bool HandleLoadBalancerRequests(TaskGroupContext *p_context, TaskGroup *p_taskGroup)
{
	Message msg;

	MPI_Bcast(&msg, sizeof(Message), MPI_CHAR, p_taskGroup->GetRankZero(), p_taskGroup->Communicator);
	std::cout << "[HandleLBRequest] :: Received message from Load Balancer : CMDID = [" << msg.CommandID << "]" << std::endl;

	switch (msg.CommandID)
	{
		case LoadBalancerMessage::Split:
		{
			std::cout << "[HandleLBRequest] :: Split :: CMDVAL = [" << msg.CommandValue << "]" << std::endl;

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
			std::cout << "[HandleLBRequest] :: Merge :: CMDVAL = [" << msg.CommandValue << "]" << std::endl;

			int subgroupIndex = msg.CommandValue;
			
			if (subgroupIndex < p_context->taskGroupList->size())
			{
				TaskGroup *taskGroup = p_context->taskGroupList->at(subgroupIndex);

				std::vector<TaskGroup*>::iterator position = std::find(p_context->taskGroupList->begin(), p_context->taskGroupList->end(), taskGroup);
				if (position != p_context->taskGroupList->end())
				{
					p_context->taskGroupList->erase(position);
					MergeTaskGroup(*(p_context->idleTaskGroup), *taskGroup);
					//p_context->idleTaskGroup->PartialMerge(taskGroup, 1);
				}

				return (taskGroup == p_context->taskGroup);
			}
			else
			{
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
	std::vector<TaskGroup*> taskGroupList;

	TaskGroup worldTaskGroup, 
		idleTaskGroup;
	
	// Synchronise processes :: Checkpoint 01
	MPI_Barrier(MPI_COMM_WORLD);

	// Initialise Groups
	InitialiseGroups(worldTaskGroup, idleTaskGroup);
	std::cout << "[Idle Worker] :: Resource pool size = " << idleTaskGroup.Size - 1 << " of " << worldTaskGroup.Size << std::endl;

	// Synchronise processes:: Checkpoint 02
	MPI_Barrier(MPI_COMM_WORLD);
	std::cout << "[Idle Worker] :: Online" << std::endl;

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