//----------------------------------------------------------------------------------------------
//	Filename:	AsyncTaskPipeline.cpp
//	Author:		Keith Bugeja
//	Date:		03/07/2013
//----------------------------------------------------------------------------------------------
#include <boost/thread.hpp>
//----------------------------------------------------------------------------------------------
#include "AsyncTaskPipeline.h"
#include "Communicator.h"
#include "ServiceManager.h"
//----------------------------------------------------------------------------------------------
AsyncTaskPipeline::AsyncTaskPipeline(ICoordinator *p_pCoordinator, IWorker *p_pWorker)
	: ITaskPipeline(p_pCoordinator, p_pWorker)
{ }
//----------------------------------------------------------------------------------------------
void AsyncTaskPipeline::ComputeThread(AsyncTaskPipeline *p_pTaskPipeline, IWorker *p_pWorker)
{
	while(p_pWorker->IsRunning())
		p_pWorker->Compute();
}
//----------------------------------------------------------------------------------------------
void AsyncTaskPipeline::Execute(ICoordinator *p_pCoordinator)
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
void AsyncTaskPipeline::Execute(IWorker *p_pWorker)
{
	ResourceMessageQueue messageQueue;

	if (!p_pWorker->Register())
		return;

	if (!p_pWorker->Initialise())
		return;

	boost::thread workerComputeHandler(
		boost::bind(&AsyncTaskPipeline::ComputeThread, this, p_pWorker));

	while(p_pWorker->IsRunning())
	{
		p_pWorker->CoordinatorMessages(); 
		p_pWorker->Synchronise();
	}

	workerComputeHandler.join();

	p_pWorker->Shutdown();
}
//----------------------------------------------------------------------------------------------
