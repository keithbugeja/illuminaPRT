//----------------------------------------------------------------------------------------------
//	Filename:	Worker.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include <boost/thread.hpp>

#include "Worker.h"
#include "MessageQueue.h"
#include "Communicator.h"
#include "ServiceManager.h"
//----------------------------------------------------------------------------------------------
ArgumentMap* IWorker::GetArgumentMap(void) {
	return &m_argumentMap;
}
//----------------------------------------------------------------------------------------------
void IWorker::SetCoordinatorID(int p_nCoordinatorID) {
	m_nCoordinatorID = p_nCoordinatorID;
}
//----------------------------------------------------------------------------------------------
int IWorker::GetCoordinatorID(void) const {
	return m_nCoordinatorID;
}
//----------------------------------------------------------------------------------------------
bool IWorker::IsRunning(void) const { 
	return m_bIsRunning; 
}
//----------------------------------------------------------------------------------------------
bool IWorker::Initialise(void) 
{ 
	m_bIsRunning = true;
	return OnInitialise(); 
}
//----------------------------------------------------------------------------------------------
void IWorker::Shutdown(void) 
{ 
	m_bIsRunning = false; 
	OnShutdown();
}
//----------------------------------------------------------------------------------------------
bool IWorker::CoordinatorMessages(void)
{
	return true;
}
//----------------------------------------------------------------------------------------------
bool IWorker::Synchronise(void) 
{
	bool bVerbose = 
		ServiceManager::GetInstance()->IsVerbose();

	// Ready message to signal availability in next computation cycle
	Message_Worker_Coordinator_Ready readyMessage;
	readyMessage.MessageID = MessageIdentifiers::ID_Worker_Ready;
	Communicator::Send(&readyMessage, sizeof(Message_Worker_Coordinator_Ready), GetCoordinatorID(), Communicator::Worker_Coordinator_Sync);

	// Receive synchronise from coordinator
	Message_Coordinator_Worker_Sync syncMessage;
	Communicator::Receive(&syncMessage, sizeof(Message_Coordinator_Worker_Sync), GetCoordinatorID(), Communicator::Coordinator_Worker_Sync);

	// If ordered to unregister, exit immediately.
	if (syncMessage.Unregister)
	{
		std::stringstream message;
		message << "Synchronise :: Releasing worker [" << ServiceManager::GetInstance()->GetResourceManager()->Me()->GetID() << "]";
		Logger::Message(message.str(), bVerbose);

		m_bIsRunning = false;
		return false;
	}

	return OnSynchronise(); 
}
//----------------------------------------------------------------------------------------------
bool IWorker::Compute(void) 
{
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000 / 60));
	return true; 
}
//----------------------------------------------------------------------------------------------
bool IWorker::Register(void) 
{
	bool bVerbose = 
		ServiceManager::GetInstance()->IsVerbose();

	// Send registration message to coordinator (on standard comm-worker channel)
	Message_Worker_Coordinator_Register message;
	message.MessageID = MessageIdentifiers::ID_Worker_Register;

	if (!Communicator::Send(&message, sizeof(Message_Worker_Coordinator_Register), GetCoordinatorID(), Communicator::Worker_Coordinator))
	{
		Logger::Message("Register :: Failed sending registration message!", bVerbose, Logger::Error);
		return false;
	}

	Communicator::Status status;
	int pCommandBuffer[1024];

	// Receive response message (on coordinator-worker-reg channel)
	Communicator::Probe(GetCoordinatorID(), Communicator::Coordinator_Worker_Reg, &status);
	Communicator::Receive(pCommandBuffer, Communicator::GetSize(&status), GetCoordinatorID(), Communicator::Coordinator_Worker_Reg, &status);

	if (pCommandBuffer[0] != MessageIdentifiers::ID_Coordinator_Accept)
	{
		Logger::Message("Register :: Coordinator rejected registration message!", bVerbose, Logger::Error);
		return false;
	}

	// Retrieve initialisation arguments
	Message_Coordinator_Worker_Accept *pAccept = (Message_Coordinator_Worker_Accept*)pCommandBuffer;
	std::string argumentString(pAccept->String, pAccept->Size);
	m_argumentMap.Initialise(argumentString);

	{
		std::stringstream message;
		message << "Registration complete. Initialising Worker with [" << argumentString << "]";
		Logger::Message(message.str(), bVerbose);
	}

	return true; 
}
//----------------------------------------------------------------------------------------------
