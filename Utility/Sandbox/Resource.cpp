//----------------------------------------------------------------------------------------------
//	Filename:	Resource.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include "Resource.h"

//----------------------------------------------------------------------------------------------
Resource::Resource(int p_nResourceID, Type p_resourceType)
	: m_resourceType(p_resourceType)
	, m_nResourceID(p_nResourceID) 
{ } 
//----------------------------------------------------------------------------------------------
int Resource::GetID(void) const { 
	return m_nResourceID; 
}
//----------------------------------------------------------------------------------------------
bool Resource::IsIdle(void) { 
	return m_resourceType == Idle; 
}
//----------------------------------------------------------------------------------------------
bool Resource::IsWorker(void) { 
	return m_resourceType == Worker; 
}
//----------------------------------------------------------------------------------------------
bool Resource::IsCoordinator(void) { 
	return m_resourceType == Coordinator; 
}
//----------------------------------------------------------------------------------------------
/*
void Resource::AssignWorkers(int p_nCoordinatorID, std::vector<Resource*> p_resourceList)
{
	Message_AssignWorker assignWorker;
	assignWorker.CoordinatorID = p_nCoordinatorID;

	for (std::vector<Resource*>::iterator resourceIterator = p_resourceList.begin();
			resourceIterator != p_resourceList.end(); ++resourceIterator)
	{
		Communicator::Send(&assignWorker, sizeof(Message_AssignWorker), (*resourceIterator)->GetID(), Communicator::Controller_Task);
	}
}
//----------------------------------------------------------------------------------------------
void Resource::Idle(void)
{
	std::cout << "Resource is IDLE." << std::endl;

	m_resourceType = Resource::Idle;

	Communicator::Status status;
	Message_AssignWorker assignWorker;

	for(;;)
	{
		// Recieve assignment from controller
		Communicator::Probe(Communicator::CoordinatorRank, Communicator::Controller_Task, &status);
		Communicator::Receive(&assignWorker, Communicator::GetSize(&status), Communicator::CoordinatorRank, Communicator::Controller_Task, &status);
		
		std::cout << "AssignWorker : [ coordinator = " << assignWorker.CoordinatorID << "]" << std::endl;
	}
}
*/