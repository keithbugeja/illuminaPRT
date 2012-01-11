#pragma once

#include "../../Core/System/ArgumentMap.h"
#include "task.h"

namespace Illumina
{
	namespace Core
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////	
		//////////////////////////////////////////////////////////////////////////////////////////////////	
		class WorkerTask
			: public Task
		{
		public:
			int PipelineStage;
		};

		//////////////////////////////////////////////////////////////////////////////////////////////////	
		//////////////////////////////////////////////////////////////////////////////////////////////////	
		class ITaskPipeline
		{
		public:
			struct CoordinatorTask
			{
				// Number of workers registered with coordinator
				int workerCount;

				// Coordinator task and worker group
				Task *task;
				TaskGroup group;

				// Scheduling groups 
				//	ready = can receive task 
				//	startup = initialisation stage
				//	shutdown = release stage
				TaskGroup ready;
				TaskGroup startup;
				TaskGroup shutdown;

				bool active;

				// Constructors
				CoordinatorTask(void)
					: active(true)
				{ }

				CoordinatorTask(Task *p_coordinator, int p_workerCount)
					: active(true)
					, task(p_coordinator)
					, workerCount(p_workerCount)
				{ }
			};

		private:
			// Keep original argument string
			std::string m_arguments;

			// Store argument map for easy access to parameters
			ArgumentMap m_argumentMap;

		protected:
			//////////////////////////////////////////////////////////////////////////////////////////////////	
			// Worker methods
			//////////////////////////////////////////////////////////////////////////////////////////////////	
			bool WRegisterWorker(Task *p_worker)
			{
				// Create a control communicator for worker
				ControlCommunicator communicator(p_worker);

				// Register worker with coordinator
				RegisterMessage registerMessage; 
				communicator.SendToCoordinator(registerMessage);

				// Receive configuration information
				char buffer[512];
				MPI_Status probeStatus;

				int coordinatorRank = p_worker->GetCoordinatorRank(),
					packetSize;

				TaskCommunicator::Probe(coordinatorRank, MM_ControlDynamic, &probeStatus);
				TaskCommunicator::Receive(buffer, packetSize = TaskCommunicator::GetSize(&probeStatus, MPI_BYTE), coordinatorRank, MM_ControlDynamic, &probeStatus);

				// Make sure configuration string is properly terminated
				buffer[packetSize] = 0;

				// Display config string
				std::cout << "[" << p_worker->GetRank() << "] : Worker has received argument map : [" << buffer << "]" << std::endl;

				// Load argument string and initialise argument map
				m_arguments.assign(buffer);
				m_argumentMap.Initialise(m_arguments);

				// Worker is now registered, but in startup mode. 
				// We should now call WInitialiseWorker and move to ready mode.
				return true;
			}
			
			bool WInitialiseWorker(Task *p_worker)
			{
				// Create a control communicator for worker
				ControlCommunicator communicator(p_worker);

				// Call pipeline-specific initialisation
				OnInitialiseWorker(m_argumentMap);
				
				// Inform the coordinator initialisation is complete
				AcknowledgeMessage acknowledge;
				communicator.SendToCoordinator(acknowledge);

				// We are now ready to receive work!
				return true;	
			}
			
			bool WShutdownWorker(Task *p_worker)
			{
				OnShutdownWorker();
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////	
			// Coordinator methods
			//////////////////////////////////////////////////////////////////////////////////////////////////	
			bool CRegisterWorker(CoordinatorTask &p_coordinator, int p_rank)
			{
				WorkerTask *workerTask = new WorkerTask();
				workerTask->SetCoordinatorRank(p_coordinator.task->GetWorkerRank());
				workerTask->SetWorkerRank(p_rank);

				p_coordinator.group.TaskList.push_back(workerTask);
				p_coordinator.startup.TaskList.push_back(workerTask);

				return true;
			}

			bool CInitialiseWorker(CoordinatorTask &p_coordinator, int p_rank)
			{
				// Send configuration information to worker, for init
				return TaskCommunicator::Send((void*)m_arguments.c_str(), m_arguments.size(), p_rank, MM_ControlDynamic);
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////	
			// Event callbacks
			//////////////////////////////////////////////////////////////////////////////////////////////////	
			bool OnCoordinatorReceiveControlMessage(CoordinatorTask &p_coordinator, 
				Message &p_message, MPI_Status *p_status, MPI_Request *p_request)
			{
				// Create a control communicator
				ControlCommunicator communicator(p_coordinator.task);

				// Message from master
				if (p_status->MPI_SOURCE == p_coordinator.task->GetMasterRank())
				{
					switch(p_message.Id)
					{
						// Terminate coordinator and worker tasks
						case MT_Terminate:
						{
							std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] : Sending termination message to workers..." << std::endl;

							TerminateMessage terminateMessage;
							p_coordinator.group.Broadcast(p_coordinator.task, terminateMessage);
							p_coordinator.active = false;

							break;
						}

						// Terminate a number of worker tasks
						case MT_Release:
						{
							ReleaseMessage *releaseMessage = (ReleaseMessage*)&p_message;
							int releaseCount = releaseMessage->GetReleaseCount();
							
							TerminateMessage terminateMessage;

							if (releaseCount >= p_coordinator.group.Size())
							{
								std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] : Sending termination message to workers..." << std::endl;

								p_coordinator.group.Broadcast(p_coordinator.task, terminateMessage);
								p_coordinator.active = false;
							} 
							else 
							{
								TaskGroup splitGroup;
								int to = p_coordinator.group.Size() - 1,
									from = p_coordinator.group.Size() - releaseCount;

								std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] : Terminating [" << releaseCount << ":" << from << "-" << to << "] workers..." << std::endl;

								p_coordinator.group.Split(&splitGroup, from, to); 
								splitGroup.Broadcast(p_coordinator.task, terminateMessage);
							}

							break;
						}
					}
				}
				else // Possible control message from worker
				{
					switch(p_message.Id)
					{
						case MT_Register:
						{
							std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] : Coordinator received register request from [" << p_status->MPI_SOURCE << "]" << std::endl;

							// Register worker and put it in startup queue
							std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] : Coordinator registering worker [" << p_status->MPI_SOURCE << "]" << std::endl;
							CRegisterWorker(p_coordinator, p_status->MPI_SOURCE);							

							std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] : Coordinator initialising worker [" << p_status->MPI_SOURCE << "]" << std::endl;
							CInitialiseWorker(p_coordinator, p_status->MPI_SOURCE);							

							break;
						}

						case MT_Acknowledge:
						{
							std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] : Coordinator received acknowledge from [" << p_status->MPI_SOURCE << "]" << std::endl;

							Task *task = p_coordinator.startup.FindTask(p_status->MPI_SOURCE);
							p_coordinator.ready.TaskList.push_back(task);

							std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] : Coordinator updating size of ready group [" << p_coordinator.ready.Size() << "]" << std::endl;

							break;
						}
					}
				}

				return true;
			}

		public:
			virtual bool OnInitialiseCoordinator(ArgumentMap &p_argumentMap) { return true; }
			virtual bool OnShutdownCoordinator(void) { return true; }

			virtual bool OnInitialiseWorker(ArgumentMap &p_argumentMap) { return true; }
			virtual bool OnShutdownWorker(void) { return true; }

			// Codify standard messages between coordinators and workers:
			// 1. Register worker (W->C) (coordinator knows of worker)
			// 2. Initialise worker (C->W) (coordinator sends script name to worker)
			// 3. Worker ready (W->C) (coordinator knows worker can be moved to ready queue)
			// 4. Shutdown worker (C->W) (coordinator asks worker to shutdown)

		public:
			bool Coordinator(CoordinatorTask &p_coordinator)
			{
				m_arguments = "script=cornell.ilm;";

				// Create a task to represent master process
				Task *masterTask = new Task();
				masterTask->SetMasterRank(0);
				masterTask->SetCoordinatorRank(-1);
				masterTask->SetWorkerRank(0);

				// DEBUG
				std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] : Initialising coordinator..." << std::endl;
				// DEBUG

				// Coordinator message loop
				ControlCommunicator controlCommunicator(p_coordinator.task);

				MPI_Request receiveRequest;
				MPI_Status receiveStatus;
				Message receiveMessage;

				// Message loop 
				while (p_coordinator.active)
				{
					// Temp
					boost::this_thread::sleep(boost::posix_time::milliseconds(500));
					// Temp

					// Get the number of allotted workers for this frame
					int workersForFrame = p_coordinator.ready.Size();
				
					if (!controlCommunicator.ReceiveAsync(receiveMessage, MPI_ANY_SOURCE, &receiveRequest, &receiveStatus))
					{
						while(!controlCommunicator.IsRequestComplete(&receiveRequest, &receiveStatus))
						{
							boost::this_thread::sleep(boost::posix_time::milliseconds(250));

							// Do a coordinator frame
							ExecuteCoordinator(p_coordinator);
						}
					}

					OnCoordinatorReceiveControlMessage(p_coordinator, receiveMessage, &receiveStatus, &receiveRequest);
				}

				return true;
			}

			bool Worker(Task *p_worker)
			{
				// Register worker with coordinator
				// ..Coordinator should now keep track of worker
				// ..Argument string should be set
				WRegisterWorker(p_worker);

				// Initialise worker
				// ..Worker should perform any init required
				// ..Coordinator should now know that worker is online
				WInitialiseWorker(p_worker);

				// Worker is online - first step should be handshaking with coordinator
				// std::cout << "Worker online" << std::endl;

				ControlCommunicator workerCommunicator(p_worker);
				Message msg;

				for(bool terminate = false; !terminate; ) 
				{
					// Worker action is synchronous
					//p_worker->ReceiveFromCoordinator(msg);
					workerCommunicator.ReceiveFromCoordinator(msg);

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