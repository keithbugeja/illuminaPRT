//----------------------------------------------------------------------------------------------
//	Filename:	RenderTaskCoordinator.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include <Maths/Maths.h>
#include <Geometry/Spline.h>
#include "Coordinator.h"
#include "Environment.h"

#include "RenderTaskCommon.h"
//----------------------------------------------------------------------------------------------
class Path
{
protected:
	std::vector<Vector3> m_vertexList;
	std::vector<float> m_pivotList;
	float m_fTime;
public:
	bool IsEmpty(void) { return m_vertexList.empty(); }
	void Clear(void) { m_vertexList.clear(); m_pivotList.clear(); Reset(); } 
	void Reset(void) { m_fTime = 0; }
	void Move(float p_fDeltaT) { m_fTime += p_fDeltaT; }

	void AddVertex(const Vector3 &p_pVertex) 
	{ 
		m_vertexList.push_back(p_pVertex); 
	}

	void PreparePath(void)
	{
		Illumina::Core::Interpolator::ComputePivots(m_vertexList, m_pivotList);
	}

	Vector3 GetPosition(float p_fTime)
	{
		if (m_vertexList.size() <= 2)
			return Vector3::Zero;

		return Illumina::Core::Interpolator::Lagrange(m_vertexList, m_pivotList, p_fTime);
	}

	Vector3 GetPosition(void) 
	{
		return GetPosition(m_fTime);
	}
};

//----------------------------------------------------------------------------------------------
class RenderTaskCoordinator
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

	RadianceBuffer *m_pRadianceBuffer,
		*m_pRadianceAccumulationBuffer,
		*m_pRadianceHistoryBuffer;

	IPostProcess *m_pBilateralFilter, 
		*m_pDiscontinuityBuffer,
		*m_pReconstructionBuffer,
		*m_pDragoTone; 

	MotionBlur *m_pMotionBlurFilter;
	AccumulationBuffer *m_pAccumulationBuffer;
	HistoryBuffer *m_pHistoryBuffer;

	Path m_cameraPath;

protected:
	std::vector<SerialisableRenderTile*> m_renderTileBuffer;

	boost::condition_variable m_decompressionQueueCV;

	int m_nProducerIndex,
		m_nConsumerIndex;

	volatile int m_nTilesPacked;

protected:
	int m_moveFlag[4];
	Vector3 m_observerPosition,
		m_observerTarget;

	bool m_bResetAccumulation,
		m_bResetWorkerSeed;

protected:
	static void InputThreadHandler(RenderTaskCoordinator *p_pCoordinator);
	static void DecompressionThreadHandler(RenderTaskCoordinator *p_pCoordinator);

public:
	bool Compute(void);

	bool OnInitialise(void);
	void OnShutdown(void);
	bool OnSynchronise(void);
	bool OnSynchroniseAbort(void);
	bool OnMessageReceived(ResourceMessage *p_pMessage);
};
//----------------------------------------------------------------------------------------------
