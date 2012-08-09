//----------------------------------------------------------------------------------------------
//	Filename:	RenderTaskCoordinator.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include "Coordinator.h"
#include "Environment.h"

#include "RenderTaskCommon.h"
//----------------------------------------------------------------------------------------------
class RenderTaskCoordinator
	: public ICoordinator
{
	RenderTaskContext m_renderTaskContext;
	SerialisableRenderTile *m_pRenderTile;

	SandboxEnvironment *m_pSandbox;

	Environment *m_pEnvironment;
	EngineKernel *m_pEngineKernel;

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

	bool OnInitialise(void);
	void OnShutdown(void);
	bool OnSynchronise(void);
	bool OnSynchroniseAbort(void);
	bool OnMessageReceived(ResourceMessage *p_pMessage);
};
//----------------------------------------------------------------------------------------------
