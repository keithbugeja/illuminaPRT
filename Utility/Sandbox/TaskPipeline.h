//----------------------------------------------------------------------------------------------
//	Filename:	TaskPipeline.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include "MessageQueue.h"
#include "Coordinator.h"
#include "Worker.h"
//----------------------------------------------------------------------------------------------
class ITaskPipeline
{
protected:
	ICoordinator *m_pCoordinator;
	IWorker *m_pWorker;

protected:
	ITaskPipeline(ICoordinator *p_pCoordinator,
		IWorker *p_pWorker);

public:
	virtual void Execute(const std::string &p_strArguments, int p_nResourceID, int p_nCoordinatorID);
	virtual void Execute(ICoordinator *p_pCoordinator) = 0;
	virtual void Execute(IWorker *p_pWorker) = 0;
};