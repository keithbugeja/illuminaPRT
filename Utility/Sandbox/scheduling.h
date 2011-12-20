#pragma once

#include <vector>
#include "taskgroup.h"

using namespace Illumina::Core;

void Master(void)
{
	std::vector<TaskGroup*> taskGroupList;
	TaskGroup *masterGroup = new TaskGroup(),
		*idleGroup = new TaskGroup();

	// Initialise resource group
	int size; MPI_Comm_size(MPI_COMM_WORLD, &size);
	for (int index = 0; index < size; ++index)
	{
		Task *task = new Task();
		task->SetMasterRank(0);
		task->SetCoordinatorRank(-1);
		task->SetWorkerRank(index);

		masterGroup->TaskList.push_back(task);
	}

	// Get master task
	Task *masterTask = masterGroup->FindTask(masterGroup->GetMasterRank());
	BOOST_ASSERT(masterTask != NULL);
	std::cout << "[" << masterTask->GetWorkerRank() << "] :: Master online" << std::endl;

	masterGroup->CreateSubGroup(idleGroup, 1, masterGroup->Size() - 1);
	std::cout << "[" << masterTask->GetWorkerRank() << "] :: Idle group created with size = " << idleGroup->Size() << std::endl;

	while(true)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));

		int request = rand() % 2 + 2;
		std::cout << "[" << masterTask->GetWorkerRank() << "] :: Request size = " << request << std::endl;

		// Issue resource request to coordinators
		if (request > idleGroup->Size())
		{
			std::cout << "[" << masterTask->GetWorkerRank() << "] :: Cannot satisfy request." << std::endl;
		}
		else
		{
			// Create task group from idle group
			TaskGroup *spawnGroup = new TaskGroup();
			taskGroupList.push_back(spawnGroup);
			idleGroup->Split(spawnGroup, 0, request - 1);

			Message splitMessage;
			splitMessage.CmdId = MessageType::Request;
			splitMessage.CmdValue = spawnGroup->TaskList[0]->GetWorkerRank();

			// Now we must inform PEs that they have been assigned to a new task group
			spawnGroup->Broadcast(masterTask, splitMessage);

			std::cout << "[" << masterTask->GetWorkerRank() << "] :: Request satisfied!" << std::endl;
		}
	}
}

void Coordinator(Task *p_task)
{
	// Coordinator is online - first step should be handshaking with workers
	std::cout << "Coordinator online" << std::endl;



	while(true) 
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));
	}
}

void Worker(Task *p_task)
{
	// Worker is online - first step should be handshaking with coordinator
	std::cout << "Worker online" << std::endl;

	while(true) 
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));
	}
}

void Idle(void)
{
	Task *masterTask = new Task(),
		*idleTask = new Task();

	masterTask->SetMasterRank(0);
	masterTask->SetCoordinatorRank(-1);
	masterTask->SetWorkerRank(0);

	int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	idleTask->SetMasterRank(0);
	idleTask->SetCoordinatorRank(-1);
	idleTask->SetWorkerRank(rank);

	std::cout << "[" << idleTask->GetWorkerRank() << "] :: Idle online" << std::endl;

	MPI_Request request;
	Message idleMessage;

	while(true)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));

		if (idleTask->ReceiveAsync(masterTask, idleMessage, &request))
			while(!idleTask->IsRequestComplete(&request))
				boost::this_thread::sleep(boost::posix_time::milliseconds(250));

		/*
		 * Handle message
		 */
		switch (idleMessage.CmdId)
		{
			case MessageType::Request:
			{
				std::cout << "[" << idleTask->GetWorkerRank() << "] :: Message received : " 
					<< idleMessage.CmdId << ", " << idleMessage.CmdValue << std::endl;

				// Set coordinator for idle task
				idleTask->SetCoordinatorRank(idleMessage.CmdValue);

				// If this task is the coordinator, spawn coordinator code
				if (idleTask->GetCoordinatorRank() == idleTask->GetWorkerRank())
				{
					Coordinator(p_task);
				}
				else
				{
					Worker(p_task);
				}
				break;
			}

			default:
				break;
		}
	}
}

void TestScheduler(int argc, char **argv)
{
	// Initialise MPI
	MPI_Init(&argc, &argv);
	
	// Get Process Rank
	int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// We need to detect whether this is running as load balancer, coordinator or worker
	if (rank == 0) Master();
	else Idle();

	// Finalise MPI
	MPI_Finalize();
}