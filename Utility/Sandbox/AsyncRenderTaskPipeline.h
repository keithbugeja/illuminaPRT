//----------------------------------------------------------------------------------------------
//	Filename:	AsyncRenderTaskPipeline.h
//	Author:		Keith Bugeja
//	Date:		03/07/2013
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include "AsyncSyncTaskPipeline.h"
#include "Environment.h"
#include "RenderTaskCoordinator.h"
#include "RenderTaskWorker.h"
//----------------------------------------------------------------------------------------------
class AsyncRenderTaskPipeline
	: public AsyncTaskPipeline
{
	AsyncRenderTaskCoordinator m_coordinator;
	AsyncRenderTaskWorker m_worker;

public:
	AsyncRenderTaskPipeline(void)
		: AsyncTaskPipeline(&m_coordinator, &m_worker)
	{ }
};