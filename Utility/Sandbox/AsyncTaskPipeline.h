//----------------------------------------------------------------------------------------------
//	Filename:	AsyncTaskPipeline.h
//	Author:		Keith Bugeja
//	Date:		03/07/2013
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include "TaskPipeline.h"
//----------------------------------------------------------------------------------------------
class AsyncTaskPipeline
	: public ITaskPipeline
{
protected:
	using ITaskPipeline::m_pCoordinator;
	using ITaskPipeline::m_pWorker;

protected:
	AsyncTaskPipeline(ICoordinator *p_pCoordinator,
		IWorker *p_pWorker);

	//static void ComputeThread(AsyncTaskPipeline *p_pTaskPipeline, ICoordinator *p_pCoodinator);
	static void ComputeThread(AsyncTaskPipeline *p_pTaskPipeline, IWorker *p_pWorker);

public:
	void Execute(ICoordinator *p_pCoordinator);
	void Execute(IWorker *p_pWorker);
};