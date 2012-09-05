//----------------------------------------------------------------------------------------------
//	Filename:	RenderTaskWorker.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include "Worker.h"
#include "Environment.h"
#include "RenderTaskCommon.h"
//----------------------------------------------------------------------------------------------
class RenderTaskWorker
	: public IWorker
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

	IPostProcess *m_pDiscontinuityBuffer,
		*m_pReconstructionBuffer,
		*m_pToneMapper;		

protected:
	bool ComputeUniform(void);
	bool ComputeVariable(void);
	bool ComputeTilePackets(int p_nStepSize);

public:
	bool Compute(void);

	bool OnInitialise(void);
	void OnShutdown(void);
	bool OnSynchronise(void);
	bool OnCoordinatorMessages(void *p_pMessage);
};
//----------------------------------------------------------------------------------------------