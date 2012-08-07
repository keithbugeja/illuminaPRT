//----------------------------------------------------------------------------------------------
//	Filename:	RenderTaskPipeline.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include "TaskPipeline.h"
#include "Environment.h"
//----------------------------------------------------------------------------------------------
class RenderTaskCoordinator
	: public ICoordinator
{
	SandboxEnvironment *m_pSandbox;

	Environment *m_pEnvironment;
	EngineKernel *m_pEngineKernel;

	// Initialise device
	IIntegrator *m_pIntegrator;
	IRenderer *m_pRenderer;
	ICamera *m_pCamera;
	ISpace *m_pSpace;

	RadianceBuffer *m_pRadianceBuffer,
		*m_pRadianceAccumulationBuffer;

	IPostProcess *m_pDiscontinuityBuffer,
		*m_pReconstructionBuffer,
		*m_pDragoTone; 

	AccumulationBuffer *m_pAccumulationBuffer;

public:
	bool Compute(void);

	// User hooks for init, shutdown, sync and message-in
	bool OnInitialise(void);
	void OnShutdown(void);
	bool OnSynchronise(void);
	bool OnMessageReceived(ResourceMessage *p_pMessage);
};
//----------------------------------------------------------------------------------------------
class RenderTaskWorker
	: public IWorker
{
	SandboxEnvironment *m_pSandbox;

	Environment *m_pEnvironment;
	EngineKernel *m_pEngineKernel;

	IIntegrator *m_pIntegrator;
	IRenderer *m_pRenderer;
	ICamera *m_pCamera;
	ISpace *m_pSpace;

	IPostProcess *m_pDiscontinuityBuffer,
		*m_pReconstructionBuffer;		

public:
	bool Compute(void);

	// User handlers for init, shutdown and sync events
	bool OnInitialise(void);
	void OnShutdown(void);
	bool OnSynchronise(void);
	bool OnCoordinatorMessages(void *p_pMessage);
};
//----------------------------------------------------------------------------------------------
class RenderTaskPipeline
	: public ITaskPipeline
{
	RenderTaskCoordinator m_coordinator;
	RenderTaskWorker m_worker;

	//ICoordinator m_coordinator;
	//IWorker m_worker;

public:
	RenderTaskPipeline(void)
		: ITaskPipeline(&m_coordinator, &m_worker)
	{ }
};