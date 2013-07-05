//----------------------------------------------------------------------------------------------
//	Filename:	SyncTaskPipeline.cpp
//	Author:		Keith Bugeja
//	Date:		03/07/2013
//----------------------------------------------------------------------------------------------
#include <boost/thread.hpp>
//----------------------------------------------------------------------------------------------
#include "SyncTaskPipeline.h"
#include "Communicator.h"
#include "ServiceManager.h"
//----------------------------------------------------------------------------------------------
SyncTaskPipeline::SyncTaskPipeline(ICoordinator *p_pCoordinator, IWorker *p_pWorker)
	: ITaskPipeline(p_pCoordinator, p_pWorker)
{ }
//----------------------------------------------------------------------------------------------
void SyncTaskPipeline::Execute(ICoordinator *p_pCoordinator)
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
		/* Uncomment if timings for the various synchronisation / 
		 * computation stages are required.
		 */
		/* 
		double start = Platform::ToSeconds(Platform::GetTime());

		p_pCoordinator->EvaluateMessageQueue(&messageQueue);
		double evaluateQueue = Platform::ToSeconds(Platform::GetTime());

		bool bSynchronise = p_pCoordinator->Synchronise();
		double synchronise = Platform::ToSeconds(Platform::GetTime());

		if (bSynchronise)
		{
			std::cout << "Message queue evaluation time [" << evaluateQueue - start << "s]" << std::endl;
			std::cout << "Synchronisation time [" << synchronise - evaluateQueue << "s]" << std::endl;

			p_pCoordinator->Compute();
			double compute = Platform::ToSeconds(Platform::GetTime());
			std::cout << "Compute time [" << compute - synchronise << "s]" << std::endl;
			
			// output computation time
			double end = Platform::ToSeconds(Platform::GetTime());
			std::cout << "Total time [" << end - start << "s]" << std::endl;
		}
		*/

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
void SyncTaskPipeline::Execute(IWorker *p_pWorker)
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
