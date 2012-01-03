#pragma once

#include "task.h"

namespace Illumina
{
	namespace Core
	{
		class WorkerTask
			: public Task
		{
		public:
			int PipelineStage;
		};


		// Task Pipeline represents a client
		// Coordinator starts with an initial number of workers
		// -- Workers may decrease or increase
		// -- -- Workers decrease on command from Master
		// -- -- -- Coordinator calls DetachWorker(N), where N is a member of set of Workers
		// -- -- -- Coordinator calls AttachWorker(N), where N is added to the set of Workers
		class ITaskPipeline
		{
		public:
		
			struct CoordinatorTask
			{
				int workerCount;
				Task *task;
				TaskGroup group;

				CoordinatorTask(void) { }

				CoordinatorTask(Task *p_coordinator, int p_workerCount)
					: task(p_coordinator)
					, workerCount(p_workerCount)
				{ }
			};

		protected:
			bool AttachWorker(CoordinatorTask &p_coordinator)
			{
				return true;
			}

			bool DetachWorker(CoordinatorTask &p_coordinator)
			{
				return true;
			}

			bool RegisterWorker(CoordinatorTask &p_coordinator, int p_rank)
			{
				WorkerTask *workerTask = new WorkerTask();
				workerTask->SetCoordinatorRank(p_coordinator.task->GetWorkerRank());
				workerTask->SetWorkerRank(p_rank);
				workerTask->PipelineStage = 0;

				p_coordinator.group.TaskList.push_back(workerTask);

				return true;
			}

			bool RegisterWithCoordinator(Task *p_worker)
			{
				RegisterMessage registerMessage; 
				return p_worker->SendToCoordinator(registerMessage);				
			}

			bool WaitForWorkers(CoordinatorTask &p_coordinator)
			{
				MPI_Status status;
				Message message; 

				for (int index = 0; index < p_coordinator.workerCount; index++)
				{
					std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] : Waiting for registration..." << std::endl;

					p_coordinator.task->ReceiveAny(message, &status, MT_Register);
					RegisterWorker(p_coordinator, status.MPI_SOURCE);

					std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] : Registered [" << status.MPI_SOURCE << "]" << std::endl;
				}

				return true;
			}

		public:
			bool Coordinator(CoordinatorTask &p_coordinator)
			{
				std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] : Initialising coordinator..." << std::endl;

				// Create a task to represent master process
				Task *masterTask = new Task();
				masterTask->SetMasterRank(0);
				masterTask->SetCoordinatorRank(-1);
				masterTask->SetWorkerRank(0);

				std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] : Waiting for workers..." << std::endl;

				// Barrier to rendezvous with workers
				WaitForWorkers(p_coordinator);

				// Start coordinator message loop
				MPI_Request request;
				Message masterMessage;

				for(bool terminate = false; !terminate; ) 
				{
					boost::this_thread::sleep(boost::posix_time::milliseconds(500));
		
					// Can receive request for:
					// 1. Resource count that can be freed
					// 2. Free specific resource count
					// 3. Close task group
		
					// Issue an asynchronous receive from master
					if (p_coordinator.task->ReceiveAsync(masterTask, masterMessage, &request))
					{
						// Execute coordinator code while we wait for a response from master
						while(!p_coordinator.task->IsRequestComplete(&request))
						{
							boost::this_thread::sleep(boost::posix_time::milliseconds(250));

							// Do a coordinator frame
							ExecuteCoordinator(p_coordinator);
						}
					}

					// We have a response from master
					switch (masterMessage.Id)
					{
						case MT_Terminate:
						{
							TerminateMessage terminateMessage;
							p_coordinator.group.Broadcast(p_coordinator.task, terminateMessage);
							terminate = true;
							break;
						}

						case MT_Release:
							break;

						default:
							break;
					}
				}

				return true;
			}

			bool Worker(Task *p_worker)
			{
				RegisterWithCoordinator(p_worker);

				// Worker is online - first step should be handshaking with coordinator
				// std::cout << "Worker online" << std::endl;

				Message msg;

				for(bool terminate = false; !terminate; ) 
				{
					// Worker action is synchronous
					p_worker->ReceiveFromCoordinator(msg);

					switch(msg.Id)
					{
						case MT_Synchronise:
						{
							// We need a barrier here before we start execution

							// Do a worker frame
							ExecuteWorker(p_worker);
						}

						case MT_Terminate:
						{
							// We are terminating this worker
							terminate = true;
						}

						default:
							break;
					}
				}

				return true;
			}

		protected:
			virtual bool ExecuteCoordinator(CoordinatorTask &p_coordinator)
			{
				return true;
			}

			virtual bool ExecuteWorker(Task *p_worker)
			{
				return true;
			}
		};
	}
}