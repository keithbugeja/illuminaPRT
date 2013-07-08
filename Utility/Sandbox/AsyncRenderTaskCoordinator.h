//----------------------------------------------------------------------------------------------
//	Filename:	AsyncRenderTaskCoordinator.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include "Environment.h"
#include "Coordinator.h"
#include "RenderTaskCommon.h"
//----------------------------------------------------------------------------------------------
class AsyncRenderTaskCoordinator
	: public ICoordinator
{
protected:
	RenderTaskContext m_renderTaskContext;
	SerialisableRenderTile *m_pRenderTile;

	SandboxEnvironment *m_pSandbox;

	Environment *m_pEnvironment;
	EngineKernel *m_pEngineKernel;

	IIntegrator *m_pIntegrator;
	IRenderer *m_pRenderer;
	ICamera *m_pCamera;
	ISpace *m_pSpace;

	RadianceBuffer *m_pRadianceBuffer;

	IPostProcess *m_pBilateralFilter, 
		*m_pDiscontinuityBuffer,
		*m_pReconstructionBuffer,
		*m_pTonemapFilter; 

	MotionBlur *m_pMotionBlurFilter;

protected:
	// Tile buffer to store results from workers
	std::vector<SerialisableRenderTile*> m_renderTileBuffer;

	// Condition variable to synchronise with decompression thread
	boost::condition_variable m_decompressionQueueCV;

	// Number of packed tiles waiting for decompression
	volatile int m_nTilesPacked;

	// Producer/consumer indices for tile decompression
	int m_nProducerIndex,
		m_nConsumerIndex;

	// Unique task identifier within system
	int m_nTaskId;

	// User name and job name
	std::string m_strUserName,
		m_strJobName;

	double m_bootTime,
		m_checkPointTime;

protected:
	// Video streaming override
	bool m_bOverrideStream;
	std::string m_strIP, 
		m_strPort;

protected:
	// Current camera path
	Path m_cameraPath;
	PathEx m_cameraPathEx;
	float m_animationTime, 
		m_animationTimeDelta;

	// Camera parameters : observer position and target
	Vector3 m_observerPosition,
		m_observerTarget;

	// Move flags (one flag for each compass direction)
	int m_moveFlag[4],
		m_seed;

	// Flags for resetting accumulation buffer
	bool m_bResetAccumulation,
		m_bResetWorkerSeed;

protected:
	AsynchronousFileSink *m_pSink;
	LoggerChannel *m_pChannel;

protected:
	static void InputThreadHandler(AsyncRenderTaskCoordinator *p_pCoordinator);
	static void DecompressionThreadHandler(AsyncRenderTaskCoordinator *p_pCoordinator);
	static void ComputeThreadHandler(AsyncRenderTaskCoordinator *p_pCoordinator);

public:
	bool Compute(void);

	bool OnInitialise(void);
	void OnShutdown(void);
	bool OnHeartbeat(void);
	bool OnHeartbeatAbort(void);
	bool OnMessageReceived(ResourceMessage *p_pMessage);
};
//----------------------------------------------------------------------------------------------
