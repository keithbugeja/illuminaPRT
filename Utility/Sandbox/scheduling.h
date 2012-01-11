#pragma once

#include <vector>
#include "taskgroup.h"
#include "taskpipeline.h"

using namespace Illumina::Core;

void Master(void)
{
	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Initialise Master
	//////////////////////////////////////////////////////////////////////////////////////////////////

	// Create two task groups
	// 1. Master : holds all the PEs including the master PE
	// 2. Idle : holds currently idle PEs
	TaskGroup *masterGroup = new TaskGroup(0, 0, 0),
		*idleGroup = new TaskGroup(1, 0);

	// Keeps track of the various groups the system PEs have
	// been partitioned in.
	TaskGroupList taskGroupList;

	// Get size of global communicator
	int size; MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	// Initialise all tasks within communicator
	for (int index = 0; index < size; ++index)
	{
		Task *task = new Task(index, 0, -1);
		masterGroup->TaskList.push_back(task);
	}

	// Get master task
	Task *masterTask = masterGroup->GetMasterTask();

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// DEBUG OUTPUT

	BOOST_ASSERT(masterTask != NULL);
	std::cout << "[" << masterTask->GetWorkerRank() << "] :: Master started." << std::endl;

	// Create idle subgroup
	masterGroup->CreateSubGroup(idleGroup, 1, masterGroup->Size() - 1);
	std::cout << "[" << masterTask->GetWorkerRank() << "] :: Master created idle group created with size = " << idleGroup->Size() << std::endl;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Initialise Master
	//////////////////////////////////////////////////////////////////////////////////////////////////

	// Control communicator
	ControlCommunicator masterCommunicator(masterTask);

	// Group Id counter
	int groupIDSource = 2;
	int requestSize = 0;

	// Buffers for asynchronous receive
	MPI_Request receiveRequest;
	MPI_Status receiveStatus;
	Message receiveMessage;	

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Start master message-loop
	//////////////////////////////////////////////////////////////////////////////////////////////////

	while(true)
	{
		// Set up an asynchronous receive
		if (!masterCommunicator.ReceiveAsync(receiveMessage, MPI_ANY_SOURCE, &receiveRequest, &receiveStatus))
		{
			// Loop until a receive buffer contains a new message
			while(!masterCommunicator.IsRequestComplete(&receiveRequest, &receiveStatus))
			{
				boost::this_thread::sleep(boost::posix_time::milliseconds(250));

				// Generate request
				requestSize = rand() % 2 + 8;
				std::cout << "[" << masterTask->GetRank() << "] :: Master received request size of [" << requestSize << "]" << std::endl;

				// Check if we can handle a request of the specified size
				if (requestSize > idleGroup->Size())
				{
					std::cout << "[" << masterTask->GetWorkerRank() << "] :: Master cannot satisfy request!" << std::endl;
					/*
					TerminateMessage terminateMessage;
					ReleaseMessage releaseMessage(2);

					if (taskGroupList.Size() > 0)
					{
						// Choose group to terminate
						int terminateIndex = rand() % taskGroupList.Size();
						TaskGroup *terminateGroup = taskGroupList.GetTaskGroupByIndex(terminateIndex);

						// Send termination / release message
						masterCommunicator.Send(releaseMessage, terminateGroup->GetCoordinatorRank());
					}
					*/
				}
				else
				{
					// If request can be satisifed, split idle group
					TaskGroup *taskGroup = new TaskGroup(groupIDSource++);
					taskGroupList.AddTaskGroup(taskGroup->GetId(), taskGroup);
					idleGroup->Split(taskGroup, 0, requestSize - 1);

					// Set group coordinator
					taskGroup->SetCoordinatorRank(taskGroup->TaskList[0]->GetRank());

					// Now we must inform PEs that they have been assigned to a new task group
					RequestMessage requestMessage(taskGroup->GetId(), 
						taskGroup->GetCoordinatorRank(), 
						taskGroup->Size());

					taskGroup->Broadcast(masterTask, requestMessage);
					
					std::cout << taskGroup->ToString() << std::endl;
					std::cout << "[" << masterTask->GetRank() << "] :: Master satisfied request for new task group [" << taskGroup->GetId() << "]" << std::endl;
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////
		// Handle async received messages
		//////////////////////////////////////////////////////////////////////////////////////////////////
		switch(receiveMessage.Id)
		{
			case MT_Completed:
			{
				CompletedMessage *completedMessage = (CompletedMessage*)&receiveMessage;

				int releaseIndex = receiveStatus.MPI_SOURCE,
					groupId = completedMessage->GetGroupId();
			
				std::cout << "[" << masterTask->GetRank() << "] :: Master received completed flag from [" << releaseIndex << 
					"] of group [" << groupId << "]" << std::endl;

				TaskGroup *taskGroup = taskGroupList.GetTaskGroupById(groupId);

				if (taskGroup != NULL)
				{
					Task *task = taskGroup->FindTask(releaseIndex);
					
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
	// Determine task rank
	int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// Declare master and idle tasks
	Task *masterTask = new Task(),
		*idleTask = new Task();

	// Initialise master and idle tasks
	masterTask->SetMasterRank(0);
	masterTask->SetCoordinatorRank(-1);
	masterTask->SetWorkerRank(0);

	idleTask->SetMasterRank(0);
	idleTask->SetCoordinatorRank(-1);
	idleTask->SetWorkerRank(rank);

	std::cout << "[" << idleTask->GetWorkerRank() << "] :: Idle task started" << std::endl;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	ControlCommunicator idleCommunicator(idleTask);
	
	MPI_Request receiveRequest;
	MPI_Status receiveStatus;
	
	Message receiveMessage;

	while(true)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));
		
		if (!idleCommunicator.ReceiveAsync(receiveMessage, MPI_ANY_SOURCE, &receiveRequest, &receiveStatus)) 
		{
			std::cout << "[" << idleTask->GetRank() << "] :: Idle task awaiting assignment..." << std::endl;

			while(!idleCommunicator.IsRequestComplete(&receiveRequest, &receiveStatus))
			{
				boost::this_thread::sleep(boost::posix_time::milliseconds(250));
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////
		// Handle received message
		//////////////////////////////////////////////////////////////////////////////////////////////////	
		switch (receiveMessage.Id)
		{
			case MT_Request:
			{
				RequestMessage *requestMessage = (RequestMessage*)&receiveMessage;

				std::cout << "[" << idleTask->GetRank() << "] :: Idle task received request : " 					
					<< "Group Id = " << requestMessage->GetGroupId()
					<< ", Coordinator Id = " << requestMessage->GetCoordinatorId() 
					<< ", Worker count = " << requestMessage->GetWorkerCount() 
					<< std::endl;

				int groupId = requestMessage->GetGroupId();

				// Set coordinator for idle task
				idleTask->SetCoordinatorRank(requestMessage->GetCoordinatorId());

				// If this task is the coordinator, spawn coordinator code
				if (idleTask->GetCoordinatorRank() == idleTask->GetWorkerRank())
				{
					std::cout << "[" << idleTask->GetRank() << "] :: Idle task changing to communicator for group [" << requestMessage->GetGroupId() << "]" << std::endl;

					ITaskPipeline pipeline;

					// Might have to revise constructor for coordinator!!!
					ITaskPipeline::CoordinatorTask coordinator;
					coordinator.task = idleTask;
					coordinator.workerCount = 0;
					coordinator.group.SetMasterRank(idleTask->GetMasterRank());
					coordinator.group.SetCoordinatorRank(idleTask->GetWorkerRank());

					pipeline.Coordinator(coordinator);

					// We need to tell master that we are ready
					CompletedMessage completedMessage(groupId);
					idleCommunicator.SendToMaster(completedMessage);
				}
				else
				{
					std::cout << "[" << idleTask->GetRank() << "] :: Idle task changing to worker for group [" << requestMessage->GetGroupId() << "]" << std::endl;

					ITaskPipeline pipeline;

					pipeline.Worker(idleTask);

					// We need to tell master that we are ready
					CompletedMessage completedMessage(groupId);
					idleCommunicator.SendToMaster(completedMessage);
				}
				break;
			}

			default:
				break;
		}
	}
}

void RunAsServer(int argc, char **argv)
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