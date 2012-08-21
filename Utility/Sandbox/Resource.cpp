//----------------------------------------------------------------------------------------------
//	Filename:	Resource.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include <iostream>

//----------------------------------------------------------------------------------------------
#include "Resource.h"
#include "Communicator.h"
#include "ServiceManager.h"
#include "MessageQueue.h"

//----------------------------------------------------------------------------------------------
Resource::Resource(int p_nResourceID, State p_resourceState)
	: m_resourceState(p_resourceState)
	, m_nResourceID(p_nResourceID) 
{ } 
//----------------------------------------------------------------------------------------------
int Resource::GetID(void) const { 
	return m_nResourceID; 
}
//----------------------------------------------------------------------------------------------
bool Resource::IsIdle(void) { 
	return m_resourceState == ST_Idle; 
}
//----------------------------------------------------------------------------------------------
bool Resource::IsWorker(void) { 
	return m_resourceState == ST_Worker; 
}
//----------------------------------------------------------------------------------------------
bool Resource::IsCoordinator(void) { 
	return m_resourceState == ST_Coordinator; 
}
//----------------------------------------------------------------------------------------------
void Resource::Terminate(std::vector<Resource*> p_resourceList)
{
	Message_Controller_Resource_Terminate messageTerminate;
	messageTerminate.MessageID = MessageIdentifiers::ID_Resource_Terminate;

	for (std::vector<Resource*>::iterator resourceIterator = p_resourceList.begin();
			resourceIterator != p_resourceList.end(); ++resourceIterator)
	{
		Communicator::Send(&messageTerminate, sizeof(Message_Controller_Resource_Terminate), (*resourceIterator)->GetID(), Communicator::Controller_Task);
	}
}
//----------------------------------------------------------------------------------------------
void Resource::Unregister(int p_nCoordinatorID, std::vector<Resource*> p_resourceList)
{
	Message_Controller_Resource_Unregister unregisterMessage;
	unregisterMessage.MessageID = MessageIdentifiers::ID_Resource_Unregister;

	int idx = 0;

	for (std::vector<Resource*>::iterator resourceIterator = p_resourceList.begin();
			resourceIterator != p_resourceList.end(); ++resourceIterator)
	{
		unregisterMessage.Resources[idx++] = (*resourceIterator)->GetID();
	}

	unregisterMessage.Size = p_resourceList.size();

	std::cout << "Unregister Message size = [" << unregisterMessage.Size << "]" << std::endl;

	Communicator::Send(&unregisterMessage, sizeof(Message_Controller_Resource_Unregister), p_nCoordinatorID, Communicator::Controller_Coordinator);
}
//----------------------------------------------------------------------------------------------
void Resource::Register(const std::string &p_strArgs, int p_nCoordinatorID, std::vector<Resource*> p_resourceList)
{
	Message_Controller_Resource_Register messageRegister;
	messageRegister.MessageID = MessageIdentifiers::ID_Resource_Register;
	messageRegister.CoordinatorID = p_nCoordinatorID;
	messageRegister.Size = p_strArgs.length();
	memset(messageRegister.String, 0, sizeof(messageRegister.String));
	memcpy(messageRegister.String, p_strArgs.c_str(), p_strArgs.length());

	for (std::vector<Resource*>::iterator resourceIterator = p_resourceList.begin();
			resourceIterator != p_resourceList.end(); ++resourceIterator)
	{
		Communicator::Send(&messageRegister, sizeof(Message_Controller_Resource_Register), (*resourceIterator)->GetID(), Communicator::Controller_Task);
	}
}
//----------------------------------------------------------------------------------------------
void Resource::Send(const std::string &p_strMessage, int p_nCoordinatorID, bool p_bHighPriority)
{
	Message_Controller_Resource_Generic messageGeneric;
	
	messageGeneric.MessageID = p_bHighPriority 
		? MessageIdentifiers::ID_Controller_HiPriority 
		: MessageIdentifiers::ID_Controller_LoPriority;

	messageGeneric.Size = p_strMessage.length();
	memset(messageGeneric.String, 0, sizeof(messageGeneric.String));
	memcpy(messageGeneric.String, p_strMessage.c_str(), p_strMessage.length());

	Communicator::Send(&messageGeneric, sizeof(Message_Controller_Resource_Generic), p_nCoordinatorID, Communicator::Controller_Coordinator);
}
//----------------------------------------------------------------------------------------------
void Resource::Start(ITaskPipeline *p_pTaskPipeline)
{
	std::cout << "Resource [" << GetID() << "] online." << std::endl;

	// Resource starts in idle state
	m_resourceState = Resource::ST_Idle;

	// Set up blocking receive operation
	Communicator::Status status;
	int *pCommandBuffer = new int[1024];

	for(bool bRunning = true; bRunning;)
	{
		// Recieve assignment from controller
		Communicator::Probe(Communicator::Controller_Rank, Communicator::Controller_Task, &status);
		Communicator::Receive(pCommandBuffer, Communicator::GetSize(&status), Communicator::Controller_Rank, Communicator::Controller_Task, &status);

		// ASSERT : *pCommandBuffer = ID_Resource_Register || *pCommandBuffer = ID_Resource_Terminate
		switch (*pCommandBuffer)
		{
			case MessageIdentifiers::ID_Resource_Register:
			{
				std::cerr << "[Resource " << GetID() << "] : Received register command [" << *pCommandBuffer << "]." << std::endl;

				Message_Controller_Resource_Register *pMessage = (Message_Controller_Resource_Register*)pCommandBuffer;
				
				// Set resource state
				m_resourceState = (GetID() == pMessage->CoordinatorID)
					? ST_Coordinator
					: ST_Worker;

				// Extract argument string
				std::string args(pMessage->String, pMessage->Size);

				// Execite pipeline
				p_pTaskPipeline->Execute(args, GetID(), pMessage->CoordinatorID);

				// Set resource state back to idle
				m_resourceState = ST_Idle;

				break;
			}

			case MessageIdentifiers::ID_Resource_Terminate:
				bRunning = false;
				break;

			default:
				std::cerr << "[Resource " << GetID() << "] : Received unrecognised command [" << *pCommandBuffer << "]." << std::endl;
				break;
		}
	}

	delete[] pCommandBuffer;
}
//----------------------------------------------------------------------------------------------
void Resource::Stop(void)
{
}
//----------------------------------------------------------------------------------------------
