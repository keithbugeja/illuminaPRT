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
void ICoordinator::ControllerCommunication(ResourceMessageQueue *p_pMessageQueue)
{
	unsigned char *pCommandBuffer = new unsigned char[8192];
	Communicator::Status status;

	while(IsRunning())
	{
		Communicator::Probe(Communicator::Controller_Rank, Communicator::Controller_Coordinator, &status);
		Communicator::Receive(pCommandBuffer, Communicator::GetSize(&status), Communicator::Controller_Rank, Communicator::Controller_Coordinator, &status);

		ResourceMessage *pMessage = new ResourceMessage(status.MPI_SOURCE, -1, status.MPI_TAG, pCommandBuffer[0], Communicator::GetSize(&status), pCommandBuffer);
		p_pMessageQueue->push(pMessage);
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
		Communicator::Probe(Communicator::Source_Any, Communicator::Worker_Coordinator, &status);
		Communicator::Receive(pCommandBuffer, Communicator::GetSize(&status), Communicator::Source_Any, Communicator::Worker_Coordinator, &status);

		ResourceMessage *pMessage = new ResourceMessage(status.MPI_SOURCE, -1, status.MPI_TAG, pCommandBuffer[0], Communicator::GetSize(&status), (unsigned char *)pCommandBuffer);
		p_pMessageQueue->push(pMessage);
	}

	delete[] pCommandBuffer;
}
//----------------------------------------------------------------------------------------------
bool ICoordinator::Initialise(void) { m_bIsRunning = true; return true; }
//----------------------------------------------------------------------------------------------
void ICoordinator::Shutdown(void) { m_bIsRunning = false; }
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

	// Add to list of registered resources
	m_registered.push_back(p_pMessage->OriginID);

	return true;
}
//----------------------------------------------------------------------------------------------
bool ICoordinator::HandleUnregister(ResourceMessage *p_pMessage)
{
	Message_Controller_Resource_Unregister *pUnregisterMessage = 
		(Message_Controller_Resource_Unregister*)p_pMessage->Content;

	Message_Coordinator_Worker_Unregister unregisterMessage;
	unregisterMessage.MessageID = MessageIdentifiers::ID_Coordinator_Unregister;

	// Fetch ID of coordinator
	int coordinatorID = ServiceManager::GetInstance()->GetResourceManager()->Me()->GetID();

	for (int workerIdx = 0; workerIdx < pUnregisterMessage->Size; ++workerIdx)
	{
		std::cout << "Resource to free : [" << pUnregisterMessage->Resources[workerIdx] << "]" << std::endl;

		if (pUnregisterMessage->Resources[workerIdx] != coordinatorID)
			Communicator::Send(&unregisterMessage, sizeof(Message_Coordinator_Worker_Unregister), pUnregisterMessage->Resources[workerIdx], Communicator::Coordinator_Worker);
		else
			m_bIsRunning = false;
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool ICoordinator::EvaluateMessageQueue(ResourceMessageQueue *p_pMessageQueue) 
{
	// Process messages in queue
	while (p_pMessageQueue->size() > 0)
	{
		ResourceMessage *pMessage = p_pMessageQueue->front();
		p_pMessageQueue->pop();

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

			default:
			{
				std::stringstream message;
				message << "Unrecognised command [" << pMessage->Command << "] received from [" << pMessage->OriginID << "]";
				Logger::Message(message.str(), ServiceManager::GetInstance()->IsVerbose());
			}
		}
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool ICoordinator::Synchronise(void) 
{
	Communicator::Request request;
	Communicator::Status status;

	Message_Worker_Coordinator_Ready readyMessage;
	Message_Coordinator_Worker_Sync syncMessage;
	syncMessage.MessageID = MessageIdentifiers::ID_Coordinator_Sync;

	m_ready.clear();

	// We open receive window for 5 ms (maybe less?)
	double timeOpen = Platform::ToSeconds(Platform::GetTime());
	for(;;)
	{
		if (Communicator::ProbeAsynchronous(Communicator::Source_Any, Communicator::Worker_Coordinator_Sync, &status))
		{
			Communicator::Receive(&readyMessage, Communicator::GetSize(&status), status.MPI_SOURCE, status.MPI_TAG);
			Communicator::Send(&syncMessage, sizeof(Message_Coordinator_Worker_Sync), status.MPI_SOURCE, Communicator::Coordinator_Worker_Sync);

			m_ready.push_back(status.MPI_SOURCE);
		}

		// Window is open for 5 ms
		if (Platform::ToSeconds(Platform::GetTime()) - timeOpen > 0.0005)
			break;
	}

	std::cout << "Workers ready : [" << m_ready.size() << "]" << std::endl;

	return OnSynchronise(); 
}
//----------------------------------------------------------------------------------------------
bool ICoordinator::Compute(void) 
{ 
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000 / 60));

	return true; 
}
//----------------------------------------------------------------------------------------------
