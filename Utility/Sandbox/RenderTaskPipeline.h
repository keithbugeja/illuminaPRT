#pragma once

#include "TaskPipeline.h"

class RenderTaskPipeline
	: public ITaskPipeline
{
	ICoordinator m_coordinator;
	IWorker m_worker;

public:
	RenderTaskPipeline(void)
		: ITaskPipeline(&m_coordinator, &m_worker)
	{ }
};