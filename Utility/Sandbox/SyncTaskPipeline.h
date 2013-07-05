//----------------------------------------------------------------------------------------------
//	Filename:	SyncTaskPipeline.h
//	Author:		Keith Bugeja
//	Date:		03/07/2013
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include "TaskPipeline.h"
//----------------------------------------------------------------------------------------------
class SyncTaskPipeline 
	: public ITaskPipeline
{
protected:
	using ITaskPipeline::m_pCoordinator;
	using ITaskPipeline::m_pWorker;

protected:
	SyncTaskPipeline(ICoordinator *p_pCoordinator,
		IWorker *p_pWorker);

public:
	void Execute(ICoordinator *p_pCoordinator);
	void Execute(IWorker *p_pWorker);
};