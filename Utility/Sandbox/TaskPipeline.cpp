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
	if (p_nCoordinatorID == p_nResourceID)
	{
		m_pCoordinator->SetArguments(p_strArguments);

		Execute(m_pCoordinator);
	}
	else
	{
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
		p_pCoordinator->Synchronise();
		p_pCoordinator->Compute();
	}

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
		if (!p_pWorker->IsRunning()) break;

		p_pWorker->Synchronise();
		p_pWorker->Compute();
	}

	p_pWorker->Shutdown();
}
//----------------------------------------------------------------------------------------------
