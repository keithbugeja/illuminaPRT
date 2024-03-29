//----------------------------------------------------------------------------------------------
//	Filename:	Worker.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include <iostream>
#include <sstream>
//----------------------------------------------------------------------------------------------
#include "Logger.h"
#include "Coordinator.h"
#include "Communicator.h"
#include "ServiceManager.h"
//----------------------------------------------------------------------------------------------
ArgumentMap* ICoordinator::GetArgumentMap(void) {
	return &m_argumentMap;
}
//----------------------------------------------------------------------------------------------
void ICoordinator::SetArguments(const std::string &p_strArguments)
{
	m_strArguments = p_strArguments;
	m_argumentMap.Initialise(m_strArguments);
}
//----------------------------------------------------------------------------------------------
std::string ICoordinator::GetArguments(void) const
{
	return m_strArguments;
}
//----------------------------------------------------------------------------------------------
std::vector<int>& ICoordinator::GetAvailableWorkerList(void)
{
	return m_ready;
}
//----------------------------------------------------------------------------------------------
std::set<int>& ICoordinator::GetRegisteredWorkerList(void)
{
	return m_registered;
}
//----------------------------------------------------------------------------------------------
void ICoordinator::ControllerCommunication(ResourceMessageQueue *p_pMessageQueue)
{
	unsigned char *pCommandBuffer = new unsigned char[8192];
	Communicator::Status status;

	while(IsRunning())
	{
		// if (Communicator::ProbeAsynchronous(Communicator::Controller_Rank, Communicator::Controller_Coordinator, &status))
		while (Communicator::ProbeAsynchronous(Communicator::Controller_Rank, Communicator::Controller_Coordinator, &status))
		{
			Communicator::Receive(pCommandBuffer, Communicator::GetSize(&status), Communicator::Controller_Rank, Communicator::Controller_Coordinator, &status);
			ResourceMessage *pMessage = new ResourceMessage(status.MPI_SOURCE, -1, status.MPI_TAG, pCommandBuffer[0], Communicator::GetSize(&status), pCommandBuffer);

			// Commands tagged as client messages should be processed immediately
			if (pMessage->Command == MessageIdentifiers::ID_Controller_HiPriority)
			{
				OnMessageReceived(pMessage);
				delete pMessage;
			} 
			else
			{
				m_messageQueueMutex.lock();
				p_pMessageQueue->push(pMessage);
				m_messageQueueMutex.unlock();	
			}
		}

		boost::this_thread::sleep(boost::posix_time::millisec(15));
		/* 
		if (...) { ... }
		else
			boost::this_thread::sleep(boost::posix_time::microsec(1000));
		*/
	}

	delete[] pCommandBuffer;
}
//----------------------------------------------------------------------------------------------
void ICoordinator::WorkerCommunication(ResourceMessageQueue *p_pMessageQueue)
{
	int *pCommandBuffer = new int[1024];
	Communicator::Status status;

	while(IsRunning())
	{
		// if (Communicator::ProbeAsynchronous(Communicator::Source_Any, Communicator::Worker_Coordinator, &status))
		while (Communicator::ProbeAsynchronous(Communicator::Source_Any, Communicator::Worker_Coordinator, &status))
		{
			Communicator::Receive(pCommandBuffer, Communicator::GetSize(&status), Communicator::Source_Any, Communicator::Worker_Coordinator, &status);
			ResourceMessage *pMessage = new ResourceMessage(status.MPI_SOURCE, -1, status.MPI_TAG, pCommandBuffer[0], Communicator::GetSize(&status), (unsigned char *)pCommandBuffer);
		
			m_messageQueueMutex.lock();
			p_pMessageQueue->push(pMessage);
			m_messageQueueMutex.unlock();
		}

		boost::this_thread::sleep(boost::posix_time::millisec(15));
		/* 
		if (...) { ... }
		else
			boost::this_thread::sleep(boost::posix_time::microsec(1000));
		*/
	}

	delete[] pCommandBuffer;
}
//----------------------------------------------------------------------------------------------
bool ICoordinator::Initialise(void) 
{ 
	m_bIsRunning = true; 
	return OnInitialise(); 
}
//----------------------------------------------------------------------------------------------
void ICoordinator::Shutdown(void) 
{ 
	m_bIsRunning = false; 
	OnShutdown();
}
//----------------------------------------------------------------------------------------------
bool ICoordinator::IsRunning(void) const { return m_bIsRunning; }
//----------------------------------------------------------------------------------------------
bool ICoordinator::HandleRegister(ResourceMessage* p_pMessage)
{
	// Send accept message to worker
	Message_Coordinator_Worker_Accept acceptMessage;
	acceptMessage.MessageID = MessageIdentifiers::ID_Coordinator_Accept;
	acceptMessage.Size = m_strArguments.length();
	memset(acceptMessage.String, 0, sizeof(acceptMessage.String));
	memcpy(acceptMessage.String, m_strArguments.c_str(), m_strArguments.length());

	if (!Communicator::Send(&acceptMessage, sizeof(Message_Coordinator_Worker_Accept), p_pMessage->OriginID, Communicator::Coordinator_Worker_Reg))
		return false;

	m_registeredMutex.lock();
	m_registered.insert(p_pMessage->OriginID);
	m_registeredMutex.unlock();

	return true;
}
//----------------------------------------------------------------------------------------------
bool ICoordinator::HandleUnregister(ResourceMessage *p_pMessage)
{
	Message_Controller_Resource_Unregister *pUnregisterMessage = 
		(Message_Controller_Resource_Unregister*)p_pMessage->Content;

	for (int workerIdx = 0; workerIdx < pUnregisterMessage->Size; ++workerIdx)
	{
		m_releaseMutex.lock();
		m_release.insert(pUnregisterMessage->Resources[workerIdx]);
		m_releaseMutex.unlock();

		m_registeredMutex.lock();
		m_registered.erase(pUnregisterMessage->Resources[workerIdx]);
		m_registeredMutex.unlock();
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool ICoordinator::EvaluateMessageQueue(ResourceMessageQueue *p_pMessageQueue) 
{
	// Process messages in queue
	while (p_pMessageQueue->size() > 0)
	{
		m_messageQueueMutex.lock();
		ResourceMessage *pMessage = p_pMessageQueue->front();
		p_pMessageQueue->pop();
		m_messageQueueMutex.unlock();

		switch(pMessage->Command)
		{
			// Worker sending in registration
			case MessageIdentifiers::ID_Worker_Register:
			{
				HandleRegister(pMessage); delete pMessage;
				break;
			}

			case MessageIdentifiers::ID_Resource_Unregister:
			{
				HandleUnregister(pMessage); delete pMessage; 
				break;
			}

			case MessageIdentifiers::ID_Controller_LoPriority:
			{
				OnMessageReceived(pMessage); delete pMessage;
				break;
			}

			default:
			{
				std::stringstream message;
				message << "Coordinator :: Unrecognised command [" << pMessage->Command << "] received from [" << pMessage->OriginID << "].";
				ServiceManager::GetInstance()->GetLogger()->Write(message.str(), LL_Info);
			}
		}
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool ICoordinator::Synchronise(void) 
{
	// Set up request and status for non-blocking receive
	Communicator::Status status;

	// Set up buffer for receipt of ready messages from available resources
	Message_Worker_Coordinator_Ready readyMessage;

	// We open receive window for 5 ms (maybe less?)
	double timeOpen = Platform::ToSeconds(Platform::GetTime());
	
	// Clear list of ready resources and start receiving
	for(m_ready.clear();;)
	{
		if (Communicator::ProbeAsynchronous(Communicator::Source_Any, Communicator::Worker_Coordinator_Sync, &status))
		{
			Communicator::Receive(&readyMessage, Communicator::GetSize(&status), status.MPI_SOURCE, status.MPI_TAG);

			if (m_release.empty() == false && 
				m_release.find(status.MPI_SOURCE) != m_release.end())
			{
				Message_Coordinator_Worker_Sync syncMessage;
				syncMessage.MessageID = MessageIdentifiers::ID_Coordinator_Sync;
				syncMessage.Unregister = true;

				Communicator::Send(&syncMessage, sizeof(Message_Coordinator_Worker_Sync), status.MPI_SOURCE, Communicator::Coordinator_Worker_Sync);

				m_releaseMutex.lock();
				m_release.erase(status.MPI_SOURCE);
				m_releaseMutex.unlock();
			} 
			else 
			{
				Message_Coordinator_Worker_Sync syncMessage;
				syncMessage.MessageID = MessageIdentifiers::ID_Coordinator_Sync;
				syncMessage.Unregister = false;

				Communicator::Send(&syncMessage, sizeof(Message_Coordinator_Worker_Sync), status.MPI_SOURCE, Communicator::Coordinator_Worker_Sync);
				
				m_ready.push_back(status.MPI_SOURCE);
			}
		}

		// Window is open for 5 ms
		if (Platform::ToSeconds(Platform::GetTime()) - timeOpen > 0.001 || m_ready.size() == m_registered.size())
			break;
	}

	// If THIS PROCESS is the only remaining process on the release list, kill task
	int coordinatorID = ServiceManager::GetInstance()->GetResourceManager()->Me()->GetID();
	
	m_releaseMutex.lock();

	if (m_release.size() == 1)
	{
		if (m_release.find(coordinatorID) != m_release.end())
		{
			m_release.erase(coordinatorID);
			m_releaseMutex.unlock();

			m_bIsRunning = false;
			return false;
		}
	}

	m_releaseMutex.unlock();

	// Have we met the minimum resource requirement for computation?
	size_t resourceLowerbound;
	if (m_argumentMap.GetArgument("min", resourceLowerbound))
	{
		if (m_ready.size() < resourceLowerbound)
		{
			OnSynchroniseAbort();
			return false;
		}
	}

	if (ServiceManager::GetInstance()->IsVerbose())
	{
		std::stringstream message;
		message << "Coordinator :: Synchronise found [" << m_ready.size() << "] workers ready.";
		ServiceManager::GetInstance()->GetLogger()->Write(message.str(), LL_Info);
	}
	
	return OnSynchronise();
}
//----------------------------------------------------------------------------------------------
bool ICoordinator::Heartbeat(void)
{
	// Set up request and status for non-blocking receive
	Communicator::Status status;

	// Set up buffer for receipt of heartbeat messages from available resources
	Message_Worker_Coordinator_Ready readyMessage;
	Message_Coordinator_Worker_Sync syncMessage;
	syncMessage.MessageID = MessageIdentifiers::ID_Coordinator_Sync;

	// Do we have any heartbeat messages?
	for (m_ready.clear(); Communicator::ProbeAsynchronous(Communicator::Source_Any, Communicator::Worker_Coordinator_Sync, &status);)
	{
		// Set unregistration flag to false
		syncMessage.Unregister = false;

		// Receive message from worker
		Communicator::Receive(&readyMessage, Communicator::GetSize(&status), status.MPI_SOURCE, status.MPI_TAG);
		
		// Lock release queue
		m_releaseMutex.lock();

		// If current message source is on release queue, free 
		// and signal termination / unregistration
		if (m_release.find(status.MPI_SOURCE) != m_release.end())
		{
			syncMessage.Unregister = true;
			m_release.erase(status.MPI_SOURCE);

			// Unlock release queue
			m_releaseMutex.unlock();
		}
		else
		{
			// Unlock release queue
			m_releaseMutex.unlock();

			// Push ready worker
			m_ready.push_back(status.MPI_SOURCE);
		}

		// Send message
		Communicator::Send(&syncMessage, sizeof(Message_Coordinator_Worker_Sync), status.MPI_SOURCE, Communicator::Coordinator_Worker_Sync);
	}

	// Get coordinator id
	int coordinatorID = ServiceManager::GetInstance()->GetResourceManager()->Me()->GetID();

	// Lock release queue
	m_releaseMutex.lock();

	// If coordinator is scheduled for release ...
	if (m_release.find(coordinatorID) != m_release.end())
	{
		std::cout << "SIGTERM received for Task Coordinator" << std::endl;
		std::cout << "Release commits left : " << m_release.size() << std::endl;
		std::cout << "Registered workers : " << m_registered.size() << std::endl;
		
		// ... free only if no more processes are registered 
		// and the release list has been emptied of all but
		// coordinator, then terminate.
		if (m_registered.empty() && m_release.size() == 1)
		{
			m_release.clear();
			m_releaseMutex.unlock();

			return m_bIsRunning = false;
		}
	}

	// Unlock release queue
	m_releaseMutex.unlock();

	// Every 1/10th of a second for now
	// boost::this_thread::sleep(boost::posix_time::microsec(100));

	// Return heartbeat
	return OnHeartbeat();
}
//----------------------------------------------------------------------------------------------
bool ICoordinator::Compute(void) 
{ 
	return true; 
}
//----------------------------------------------------------------------------------------------
