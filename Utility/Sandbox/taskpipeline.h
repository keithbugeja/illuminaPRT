#pragma once

#include "../../Core/System/ArgumentMap.h"
#include "task.h"

namespace Illumina
{
	namespace Core
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////	
		//////////////////////////////////////////////////////////////////////////////////////////////////	
		class ITaskPipeline
		{
		public:
			//////////////////////////////////////////////////////////////////////////////////////////////////	
			//////////////////////////////////////////////////////////////////////////////////////////////////	
			struct CoordinatorTask
			{
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

				bool active,
					terminating;

				int terminateCount;

				// Constructors
				CoordinatorTask(void)
					: active(true)
					, terminating(false)
					, terminateCount(0)
				{ }

				CoordinatorTask(Task *p_coordinator)
					: active(true)
					, terminating(false)
					, terminateCount(0)
					, task(p_coordinator)
				{ }
			};

		//////////////////////////////////////////////////////////////////////////////////////////////////	
		//////////////////////////////////////////////////////////////////////////////////////////////////	
		private:
			bool m_verbose;
		
			// Keep original argument string
			std::string m_arguments;

			// Store argument map for easy access to parameters
			ArgumentMap m_argumentMap;

		protected:
			bool IsVerbose(void) { return m_verbose; }

		//////////////////////////////////////////////////////////////////////////////////////////////////	
		//////////////////////////////////////////////////////////////////////////////////////////////////	
		protected:
			//////////////////////////////////////////////////////////////////////////////////////////////////	
			// Worker methods
			//////////////////////////////////////////////////////////////////////////////////////////////////	
			bool WRegisterWorker(Task *p_worker)
			{
				// Create a channel communicator over ChannelWorkerStatic
				WorkerCommunicator communicator(p_worker);

				// Register worker with coordinator
				RegisterMessage registerMessage; 

				std::cout << "Sending register message..." << std::endl;

				communicator.SendToCoordinator((IMessage*)&registerMessage);

				std::cout << "Register sent..." << std::endl;

				// Receive configuration information
				char buffer[512];
				MPI_Status probeStatus;

				int coordinatorRank = p_worker->GetCoordinatorRank(),
					packetSize;

				TaskCommunicator::Probe(coordinatorRank, MM_ChannelWorkerDynamic_0, &probeStatus);
				TaskCommunicator::Receive(buffer, packetSize = TaskCommunicator::GetSize(&probeStatus, MPI_BYTE), coordinatorRank, MM_ChannelWorkerDynamic_0);

				// Make sure configuration string is properly terminated
				buffer[packetSize] = 0;

				// Display config string
				std::cout << "[" << p_worker->GetRank() << "] :: Worker has received argument map : [" << buffer << "]." << std::endl;

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
				WorkerCommunicator communicator(p_worker);

				// Call pipeline-specific initialisation
				OnInitialiseWorker(m_argumentMap);
				
				// Inform the coordinator initialisation is complete
				AcknowledgeMessage acknowledge;
				communicator.SendToCoordinator((IMessage*)&acknowledge);

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
				Task *workerTask = new Task();
				workerTask->SetCoordinatorRank(p_coordinator.task->GetWorkerRank());
				workerTask->SetWorkerRank(p_rank);

				p_coordinator.group.TaskList.push_back(workerTask);
				p_coordinator.startup.TaskList.push_back(workerTask);

				return true;
			}

			bool CInitialiseWorker(CoordinatorTask &p_coordinator, int p_rank)
			{
				// Send configuration information to worker, for init
				bool result = TaskCommunicator::Send((void*)m_arguments.c_str(), m_arguments.size(), p_rank, MM_ChannelWorkerDynamic_0);

				return result;
			}

			bool CSynchroniseWorker(CoordinatorTask &p_coordinator)
			{
				SynchroniseMessage synchroniseMessage;

				std::cout << "Synchronising workers..." << std::endl;
				p_coordinator.ready.Broadcast(p_coordinator.task, (IMessage*)&synchroniseMessage, MM_ChannelWorkerStatic);
				std::cout << "Synchronise sent..." << std::endl;

				return true;
			}

			int CReleaseWorker(CoordinatorTask &p_coordinator, int p_releaseCount)
			{
				// Termination message to send workers
				TerminateMessage terminateMessage;

				// Get a snapshot of size of ready queue
				int readyCount = p_coordinator.ready.Size();
				
				if (readyCount == 0) 
					return 0;

				if (readyCount > p_releaseCount)
				{
					// NOTE: We might have a memory leak here ... nobody is managing discarded tasks!
					TaskGroup releaseGroup;
					
					// Split ready queue
					p_coordinator.ready.Split(&releaseGroup, 1, p_releaseCount);

					// Remove tasks from coordinator's group
					for (std::vector<Task*>::iterator taskIterator = releaseGroup.TaskList.begin();
						 taskIterator != releaseGroup.TaskList.end(); ++taskIterator)
					{
						p_coordinator.group.Remove(*taskIterator);
					}

					// Broadcast release
					releaseGroup.Broadcast(p_coordinator.task, (IMessage*)&terminateMessage, MM_ChannelWorkerStatic);

					// DEBUG OUT
					std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] :: Coordinator broadcast [TERMINATE] to [" << p_releaseCount << "] units. "<< std::endl;
					// DEBUG OUT

					return p_releaseCount;
				}
				else
				{
					// We can service the request from the RDY and STRT queues
					if (readyCount + p_coordinator.startup.Size() >= p_releaseCount)
					{
						// NOTE: We might have a memory leak here ... nobody is managing discarded tasks!

						// Remove tasks from coordinator's group
						for (std::vector<Task*>::iterator taskIterator = p_coordinator.ready.TaskList.begin();
							 taskIterator != p_coordinator.ready.TaskList.end(); ++taskIterator)
						{
							p_coordinator.group.Remove(*taskIterator);
						}

						p_coordinator.ready.Broadcast(p_coordinator.task, (IMessage*)&terminateMessage, MM_ChannelWorkerStatic);
						int released = p_coordinator.ready.Size(); p_coordinator.ready.TaskList.clear();
								
						std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] :: Coordinator broadcast [TERMINATE] to [" << readyCount << "] units. "<< std::endl;
						std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] :: Coordinator deferring [TERMINATE] broadcast to [" << p_releaseCount - readyCount << "] units. "<< std::endl;

						return released;
					}
					else
					{
						// Start shutting down... terminate ready queue first
						p_coordinator.ready.Broadcast(p_coordinator.task, (IMessage*)&terminateMessage, MM_ChannelWorkerStatic);
						int released = p_coordinator.ready.Size(); p_coordinator.ready.TaskList.clear();

						if (p_coordinator.startup.Size() > 0)
						{
							// Enter shutdown procedure if units are still being initialised
							p_coordinator.terminating = true;

							std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] :: Coordinator broadcast [TERMINATE]. Waiting for tasks in [INIT] to complete." << std::endl;
						}
						else
						{
							// Otherwise quit coordinator
							p_coordinator.active = false;

							std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] :: Coordinator broadcast [TERMINATE]. Taskgroup shutting down." << std::endl;
						}

						return released;
					}
				}
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////	
			// Event callbacks
			//////////////////////////////////////////////////////////////////////////////////////////////////
			bool OnCoordinatorReceiveControlMessage_M(CoordinatorTask &p_coordinator, 
				Message &p_message, MPI_Status *p_status, MPI_Request *p_request)
			{
				switch(p_message.Id)
				{
					// Init task termination
					case MT_Terminate:
					{	
						std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] :: Coordinator received [TERMINATE]." << std::endl;
						
						p_coordinator.terminateCount = p_coordinator.group.Size();
						p_coordinator.terminating = true;
						
						break;
					}

					// Terminate a number of worker tasks
					case MT_Release:
					{
						ReleaseMessage *releaseMessage = (ReleaseMessage*)&p_message;
						p_coordinator.terminateCount += releaseMessage->GetReleaseCount();

						std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] :: Coordinator received [RELEASE] :: PE [" << releaseMessage->GetReleaseCount() << "], RDY [" << p_coordinator.ready.Size() << "], INIT [" << p_coordinator.startup.Size() << "]." << std::endl;
														
						break;
					}

					default:
					{
						OnCoordinatorReceiveMasterMessage(p_coordinator, p_message, p_status, p_request);
						break;
					}
				}

				return true;
			}
			
			bool OnCoordinatorReceiveControlMessage_W(CoordinatorTask &p_coordinator, 
				Message &p_message, MPI_Status *p_status, MPI_Request *p_request)
			{
				switch(p_message.Id)
				{
					// Register worker and put it in startup queue
					case MT_Register:
					{
						std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] :: Coordinator registering worker [" << p_status->MPI_SOURCE << "]." << std::endl;
						CRegisterWorker(p_coordinator, p_status->MPI_SOURCE);							

						std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] :: Coordinator initialising worker [" << p_status->MPI_SOURCE << "]." << std::endl;
						CInitialiseWorker(p_coordinator, p_status->MPI_SOURCE);							
						break;
					}

					// Acknowledge worker has completed startup and move to ready queue
					case MT_Acknowledge:
					{
						std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] :: Coordinator received acknowledge from [" << p_status->MPI_SOURCE << "]." << std::endl;

						Task *task = p_coordinator.startup.FindTask(p_status->MPI_SOURCE);
						p_coordinator.startup.Remove(task);
						p_coordinator.ready.TaskList.push_back(task);

						std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] :: Coordinator updating size of ready group [" << p_coordinator.ready.Size() << "]." << std::endl;
						break;
					}

					default:
					{
						OnCoordinatorReceiveWorkerMessage(p_coordinator, p_message, p_status, p_request);
						break;
					}
				}

				return true;
			}

			bool OnCoordinatorReceiveControlMessage(CoordinatorTask &p_coordinator, 
				Message &p_message, MPI_Status *p_status, MPI_Request *p_request)
			{
				// Message from master
				if (p_status->MPI_SOURCE == p_coordinator.task->GetMasterRank())
					return OnCoordinatorReceiveControlMessage_M(p_coordinator, p_message, p_status, p_request);
				else
					return OnCoordinatorReceiveControlMessage_W(p_coordinator, p_message, p_status, p_request);
			}

		protected:
			virtual bool OnInitialiseCoordinator(ArgumentMap &p_argumentMap) { return true; }
			virtual bool OnShutdownCoordinator(void) { return true; }

			virtual bool OnInitialiseWorker(ArgumentMap &p_argumentMap) { return true; }
			virtual bool OnShutdownWorker(void) { return true; }

			virtual void OnCoordinatorReceiveMasterMessage(CoordinatorTask &p_coordinator, Message &p_message, MPI_Status *p_status, MPI_Request *p_request) { }
			virtual void OnCoordinatorReceiveWorkerMessage(CoordinatorTask &p_coordinator, Message &p_message, MPI_Status *p_status, MPI_Request *p_request) { }

			virtual void OnWorkerReceiveCoordinatorMessage(Task *p_worker, Message &p_message) { }

		public:
			ITaskPipeline(std::string &p_arguments, bool p_verbose = true)
				: m_arguments(p_arguments)
				, m_verbose(p_verbose)
			{ 
				m_argumentMap.Initialise(m_arguments);
			}

			virtual ~ITaskPipeline(void) { }

			bool Coordinator(CoordinatorTask &p_coordinator)
			{
				// Trigger coordinator initialisation event
				OnInitialiseCoordinator(m_argumentMap);

				// Create a task to represent master process
				Task *masterTask = new Task();
				masterTask->SetMasterRank(0);
				masterTask->SetCoordinatorRank(-1);
				masterTask->SetWorkerRank(0);

				// DEBUG
				std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] :: Initialising coordinator." << std::endl;
				// DEBUG

				// Setup master and worker communication channels
				MasterCommunicator masterCommunicator(p_coordinator.task);
				WorkerCommunicator workerCommunicator(p_coordinator.task);

				// Prepare message, request and status buffers
				MPI_Request masterRequest, workerRequest;
				MPI_Status masterStatus, workerStatus;
				Message masterMessage, workerMessage;

				// Message flags
				bool masterMessageIn = true, 
					workerMessageIn = true;

				// Message loop 
				while (p_coordinator.active)
				{
					// Try to process as many pending messages as are available
					do 
					{
						workerMessageIn = workerCommunicator.ProbeAsynchronous(MPI_ANY_SOURCE, MM_ChannelWorkerStatic, &workerStatus);
						masterMessageIn = masterCommunicator.ProbeAsynchronous(p_coordinator.task->GetMasterRank(), MM_ChannelMasterStatic, &masterStatus);

						if (workerMessageIn) {
							workerCommunicator.Receive(&workerMessage, workerStatus.MPI_SOURCE);
							OnCoordinatorReceiveControlMessage(p_coordinator, workerMessage, &workerStatus, &workerRequest);
						}

						if (masterMessageIn) {
							masterCommunicator.Receive(&masterMessage, p_coordinator.task->GetMasterRank());
							OnCoordinatorReceiveControlMessage(p_coordinator, masterMessage, &masterStatus, &masterRequest);
						}
					} while (masterMessageIn || workerMessageIn);

					// Process termination requests
					if (p_coordinator.terminateCount > 0)
					{
						p_coordinator.terminateCount -= CReleaseWorker(p_coordinator, p_coordinator.terminateCount);
						
						if (p_coordinator.terminateCount == 0 && p_coordinator.terminating)
						{
							p_coordinator.active = false;
							continue;
						}
					}

					// Execute context pipeline until any messages (from workers or master) are available...
					while(!masterMessageIn && !workerMessageIn)
					{
						workerMessageIn = workerCommunicator.ProbeAsynchronous(MPI_ANY_SOURCE, MM_ChannelWorkerStatic, &workerStatus);
						masterMessageIn = masterCommunicator.ProbeAsynchronous(p_coordinator.task->GetMasterRank(), MM_ChannelMasterStatic, &masterStatus);

						// Prepare workers for work
						CSynchroniseWorker(p_coordinator);

						// Do a coordinator frame
						std::cout << "[" << p_coordinator.task->GetRank() << "] :: Coordinator starting execution of pipeline." << std::endl;
						ExecuteCoordinator(p_coordinator);
						std::cout << "[" << p_coordinator.task->GetRank() << "] :: Coordinator completed execution of pipeline." << std::endl;
					}

					// DEBUG
					std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] :: Coordinator message flags are [MASTER : " << masterMessageIn << ", WORKER : " << workerMessageIn << "]." << std::endl;
					// DEBUG
				}


				/*
				// Message loop 
				while (p_coordinator.active)
				{
					// Set up asynchronous receives for coordinator
					if (workerMessageIn) {
						OnCoordinatorReceiveControlMessage(p_coordinator, workerMessage, &workerStatus, &workerRequest);
						workerMessageIn = workerCommunicator.ReceiveAsynchronous(&workerMessage, MPI_ANY_SOURCE, &workerRequest, &workerStatus);
					}

					if (masterMessageIn) {
						OnCoordinatorReceiveControlMessage(p_coordinator, masterMessage, &masterStatus, &masterRequest);
						masterMessageIn = masterCommunicator.ReceiveAsynchronous(&masterMessage, p_coordinator.task->GetMasterRank(), &masterRequest, &masterStatus);
					}

					// Get the number of allotted workers for this frame
					int workersForFrame = p_coordinator.ready.Size();

					// Process termination requests
					if (p_coordinator.terminateCount > 0)
					{
						p_coordinator.terminateCount -= CReleaseWorker(p_coordinator, p_coordinator.terminateCount);
						
						if (p_coordinator.terminateCount == 0 && p_coordinator.terminating)
						{
							p_coordinator.active = false;
							continue;
						}
					}

					// Wait for asynchronous receive to complete
					while(!masterMessageIn && !workerMessageIn)
					{
						workerMessageIn = workerCommunicator.IsRequestComplete(&workerRequest, &workerStatus);
						masterMessageIn = masterCommunicator.IsRequestComplete(&masterRequest, &masterStatus);

						// Prepare workers for work
						CSynchroniseWorker(p_coordinator);

						// boost::this_thread::sleep(boost::posix_time::milliseconds(250));

						std::cout << "[" << p_coordinator.task->GetRank() << "] :: Coordinator starting execution of pipeline." << std::endl;

						// Do a coordinator frame
						ExecuteCoordinator(p_coordinator);

						std::cout << "[" << p_coordinator.task->GetRank() << "] :: Coordinator completed execution of pipeline." << std::endl;
					}

					// DEBUG
					std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] :: Coordinator message flags are [MASTER : " << masterMessageIn << ", WORKER : " << workerMessageIn << "]." << std::endl;
					// DEBUG
				}
				*/

				// Trigger coordinator shutdown event
				OnShutdownCoordinator();

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

				WorkerCommunicator workerCommunicator(p_worker);
				Message msg;

				for(bool terminate = false; !terminate; ) 
				{
					std::cout << "[" << p_worker->GetRank() << "] :: Worker online waiting for coordinator message." << std::endl;

					// Worker action is synchronous
					workerCommunicator.ReceiveFromCoordinator(&msg);

					std::cout << "[" << p_worker->GetRank() << "] :: Worker received coordinator message with Id = [" << msg.Id << "]" << std::endl;

					switch(msg.Id)
					{
						case MT_Synchronise:
						{
							std::cout << "[" << p_worker->GetRank() << "] :: Worker starting execution of pipeline." << std::endl;

							ExecuteWorker(p_worker);

							std::cout << "[" << p_worker->GetRank() << "] :: Worker completed execution of pipeline." << std::endl;
							break;
						}

						case MT_Terminate:
						{
							// We are terminating this worker
							terminate = true;
							break;
						}

						default:
							OnWorkerReceiveCoordinatorMessage(p_worker, msg);
							break;
					}
				}

				// Trigger worker shutdown event
				OnShutdownWorker();

				return true;
			}

		protected:
			virtual bool ExecuteCoordinator(CoordinatorTask &p_coordinator) { return true; }
			virtual bool ExecuteWorker(Task *p_worker) { return true; }
		};
	}
}