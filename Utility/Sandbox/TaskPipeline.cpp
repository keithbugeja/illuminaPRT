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
	std::stringstream messageLog;
	Logger *logger = ServiceManager::GetInstance()->GetLogger();

	if (p_nCoordinatorID == p_nResourceID)
	{
		messageLog << "TaskPipeline :: Starting coordinator with arguments [" << p_strArguments << "].";
		logger->Write(messageLog.str(), LL_Info);

		m_pCoordinator->SetArguments(p_strArguments);
		
		Execute(m_pCoordinator);
	}
	else
	{
		messageLog << "TaskPipeline :: Starting new worker for coordinator [" << p_nCoordinatorID << "].";
		logger->Write(messageLog.str(), LL_Info);

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
