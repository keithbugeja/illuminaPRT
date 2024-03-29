//----------------------------------------------------------------------------------------------
//	Filename:	RenderTaskPipeline.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include "SyncTaskPipeline.h"
#include "Environment.h"
#include "RenderTaskCoordinator.h"
#include "RenderTaskWorker.h"
//----------------------------------------------------------------------------------------------
class RenderTaskPipeline
	: public SyncTaskPipeline
{
	RenderTaskCoordinator m_coordinator;
	RenderTaskWorker m_worker;

public:
	RenderTaskPipeline(void)
		: SyncTaskPipeline(&m_coordinator, &m_worker)
	{ }
};