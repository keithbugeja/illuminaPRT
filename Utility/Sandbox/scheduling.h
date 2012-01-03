#pragma once

#include <vector>
#include "taskgroup.h"
#include "taskpipeline.h"

using namespace Illumina::Core;

void Master(void)
{
	//std::vector<TaskGroup*> taskGroupList;

	TaskGroupList taskGroupList;

	TaskGroup *masterGroup = new TaskGroup(0, 0, 0),
		*idleGroup = new TaskGroup(1, 0);

	// Get size of global communicator
	int size; MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	// Initialise all tasks within communicator
	for (int index = 0; index < size; ++index)
	{
		Task *task = new Task(index, 0, -1);
		masterGroup->TaskList.push_back(task);

		// std::cout << task->ToString() << std::endl;
	}

	// Get master task
	Task *masterTask = masterGroup->GetMasterTask();
	
	BOOST_ASSERT(masterTask != NULL);
	std::cout << "[" << masterTask->GetWorkerRank() << "] :: Master online" << std::endl;

	masterGroup->CreateSubGroup(idleGroup, 1, masterGroup->Size() - 1);
	std::cout << "[" << masterTask->GetWorkerRank() << "] :: Idle group created with size = " << idleGroup->Size() << std::endl;

	// Used for async communication
	MPI_Request request;
	MPI_Status status;

	// -- Set to some centralised ID provider
	int groupIDSource = 2,
		peRequest;

	Message rcptMessage;

	while(true)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));

		peRequest = rand() % 2 + 2;
		std::cout << "[" << masterTask->GetWorkerRank() << "] :: Request size = " << peRequest << std::endl;

		if (masterTask->ReceiveAnyAsync(rcptMessage, &request))
		{
			while(!masterTask->IsRequestComplete(&request, &status))
			{
				// Issue resource request to coordinators
				if (peRequest > idleGroup->Size())
				{
					boost::this_thread::sleep(boost::posix_time::milliseconds(500));
					std::cout << "[" << masterTask->GetWorkerRank() << "] :: Cannot satisfy request!" << std::endl;
					
					TerminateMessage terminateMessage;

					if (taskGroupList.Size() > 0)
					{
						int terminateIndex = rand() % taskGroupList.Size();
						TaskGroup *terminateGroup = taskGroupList.GetTaskGroupByIndex(terminateIndex);
						Task *terminateCoordinator = terminateGroup->GetCoordinatorTask();
					
						masterTask->Send(terminateCoordinator, terminateMessage);
					}
				}
				else
				{
					// Create task group from idle group
					TaskGroup *spawnGroup = new TaskGroup(groupIDSource++);
					taskGroupList.AddTaskGroup(spawnGroup->GetId(), spawnGroup);
					idleGroup->Split(spawnGroup, 0, peRequest - 1);

					// Set group coordinator
					spawnGroup->SetCoordinatorRank(spawnGroup->TaskList[0]->GetWorkerRank());

					// Now we must inform PEs that they have been assigned to a new task group
					RequestMessage requestMessage(spawnGroup->GetId(), 
						spawnGroup->GetCoordinatorRank(), 
						spawnGroup->Size());

					spawnGroup->Broadcast(masterTask, requestMessage);
					
					//std::cout << spawnGroup->ToString() << std::endl;
					std::cout << "[" << masterTask->GetWorkerRank() << "] :: Request satisfied!" << std::endl;
				}
			}
		}

		// 
		switch(rcptMessage.Id)
		{
			case MT_Completed:
			{
				CompletedMessage *msg = (CompletedMessage*)&rcptMessage;

				int releasePE = status.MPI_SOURCE,
					groupId = msg->GetGroupId();
				
				TaskGroup *taskGroup = taskGroupList.GetTaskGroupById(groupId);

				if (taskGroup != NULL)
				{
					Task *task = taskGroup->FindTask(releasePE);
					
					if (task != NULL)
						idleGroup->Merge(task);
				}

				break;
			}
		}
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
		{
			std::cout << "[" << idleTask->GetWorkerRank() << "] :: Awaiting assignment..." << std::endl;

			while(!idleTask->IsRequestComplete(&request))
			{
				boost::this_thread::sleep(boost::posix_time::milliseconds(250));
			}
		}

		/*
		 * Handle message
		 */
		switch (idleMessage.Id)
		{
			case MT_Request:
			{
				RequestMessage *requestMessage = (RequestMessage*)&idleMessage;

				std::cout << "[" << idleTask->GetWorkerRank() << "] :: Request received : " 					
					<< ", Group Id = " << requestMessage->GetGroupId()
					<< ", Coordinator Id = " << requestMessage->GetCoordinatorId() 
					<< ", Worker count = " << requestMessage->GetWorkerCount() 
					<< std::endl;

				int groupId = requestMessage->GetGroupId();

				// Set coordinator for idle task
				idleTask->SetCoordinatorRank(requestMessage->GetCoordinatorId());

				// If this task is the coordinator, spawn coordinator code
				if (idleTask->GetCoordinatorRank() == idleTask->GetWorkerRank())
				{
					ITaskPipeline pipeline;

					// Might have to revise constructor for coordinator!!!
					ITaskPipeline::CoordinatorTask coordinator;
					coordinator.task = idleTask;
					coordinator.workerCount = requestMessage->GetWorkerCount() - 1;
					coordinator.group.SetMasterRank(idleTask->GetMasterRank());
					coordinator.group.SetCoordinatorRank(idleTask->GetWorkerRank());

					//coordinator(idleTask, requestMessage->GetWorkerCount() - 1);
					pipeline.Coordinator(coordinator);

					// We need to tell master that we are ready
					CompletedMessage completedMessage(groupId);
					idleTask->SendToMaster(completedMessage);
				}
				else
				{
					ITaskPipeline pipeline;
					pipeline.Worker(idleTask);

					// We need to tell master that we are ready
					CompletedMessage completedMessage(groupId);
					idleTask->SendToMaster(completedMessage);
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