//----------------------------------------------------------------------------------------------
//	Filename:	AsyncRenderTaskWorker.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include "Worker.h"
#include "Environment.h"
#include "RenderTaskCommon.h"
//----------------------------------------------------------------------------------------------
class AsyncRenderTaskWorker
	: public IWorker
{
	RenderTaskContext m_renderTaskContext;
	SerialisableRenderTile *m_pRenderTile,
		*m_pRimmedRenderTile;

	SandboxEnvironment *m_pSandbox;

	Environment *m_pEnvironment;
	EngineKernel *m_pEngineKernel;

	IIntegrator *m_pIntegrator;
	IRenderer *m_pRenderer;
	ICamera *m_pCamera;
	ISpace *m_pSpace;

	IPostProcess *m_pBilateralFilter, 
		*m_pDiscontinuityBuffer,
		*m_pReconstructionBuffer,
		*m_pToneMapper;		

	unsigned int m_unSamplerSeed;

	int m_nTaskId;

	int m_nBorderSize;
	unsigned int m_uiFilterFlags;

	bool m_bFramePrepare;

protected:
	bool ComputeUniform(void);
	bool ComputeVariable(void);
	bool ComputeTilePackets(int p_nStepSize);

public:
	bool Compute(void);

	bool OnInitialise(void);
	void OnShutdown(void);
	bool OnHeartbeat(void);
	bool OnCoordinatorMessages(void *p_pMessage);
};
//----------------------------------------------------------------------------------------------