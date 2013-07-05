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
