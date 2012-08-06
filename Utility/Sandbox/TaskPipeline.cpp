//----------------------------------------------------------------------------------------------
//	Filename:	TaskPipeline.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include <boost/thread.hpp>
//----------------------------------------------------------------------------------------------
#include "TaskPipeline.h"
#include "Communicator.h"
#include "ServiceManager.h"
//----------------------------------------------------------------------------------------------
ITaskPipeline::ITaskPipeline(ICoordinator *p_pCoordinator, IWorker *p_pWorker)
	: m_pCoordinator(p_pCoordinator)
	, m_pWorker(p_pWorker)
{ }
//----------------------------------------------------------------------------------------------
void ITaskPipeline::Execute(const std::string &p_strArguments, int p_nResourceID, int p_nCoordinatorID)
{
	bool bVerbose = 
		ServiceManager::GetInstance()->IsVerbose();

	if (p_nCoordinatorID == p_nResourceID)
	{
		std::stringstream message;
		message << "Task Pipeline executing coordinator with arguments [" << p_strArguments << "]";
		Logger::Message(message.str(), bVerbose);

		m_pCoordinator->SetArguments(p_strArguments);

		Execute(m_pCoordinator);
	}
	else
	{
		std::stringstream message;
		message << "Task Pipeline executing worker for coordinator [" << p_nCoordinatorID << "]";
		Logger::Message(message.str(), bVerbose);

		m_pWorker->SetCoordinatorID(p_nCoordinatorID);

		Execute(m_pWorker);
	}
}
//----------------------------------------------------------------------------------------------
void ITaskPipeline::Execute(ICoordinator *p_pCoordinator)
{
	ResourceMessageQueue messageQueue;

	if (!p_pCoordinator->Initialise())
		return;

	boost::thread coordinatorControllerHandler(
		boost::bind(&ICoordinator::ControllerCommunication, p_pCoordinator, &messageQueue));

	boost::thread coordinatorWorkerHandler(
		boost::bind(&ICoordinator::WorkerCommunication, p_pCoordinator, &messageQueue));

	while(p_pCoordinator->IsRunning())
	{
		p_pCoordinator->EvaluateMessageQueue(&messageQueue);

		if (p_pCoordinator->Synchronise())
			p_pCoordinator->Compute();
	}

	// join to threads
	coordinatorControllerHandler.join();
	coordinatorWorkerHandler.join();

	p_pCoordinator->Shutdown();
}
//----------------------------------------------------------------------------------------------
void ITaskPipeline::Execute(IWorker *p_pWorker)
{
	ResourceMessageQueue messageQueue;

	if (!p_pWorker->Register())
		return;

	if (!p_pWorker->Initialise())
		return;

	while(p_pWorker->IsRunning())
	{
		p_pWorker->CoordinatorMessages(); 
		
		if (p_pWorker->Synchronise())
			p_pWorker->Compute();
	}

	p_pWorker->Shutdown();
}
//----------------------------------------------------------------------------------------------
