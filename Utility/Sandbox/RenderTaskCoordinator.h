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
	float m_fTime;
public:
	void AddVertex(Vector3 &p_pVertex) { m_vertexList.push_back(p_pVertex); }
	bool IsEmpty(void) { return m_vertexList.empty(); }
	void Clear(void) { m_vertexList.clear(); Reset(); } 
	void Reset(void) { m_fTime = 0; }
	void Move(float p_fDeltaT) { m_fTime += p_fDeltaT; }
	Vector3 GetPosition(float p_fTime)
	{
		if (m_vertexList.size() < 4)
			return Vector3::Zero;

		int lbId = 1,
			ubId = m_vertexList.size() - 2,
			ptId[4];

		ptId[1] = p_fTime * ubId + lbId;
		ptId[0] = ptId[1] - 1;
		ptId[2] = ptId[1] + 1;
		ptId[3] = ptId[2] + 1;

		float segment = 1.f/ubId;

		float interval = Maths::Frac(p_fTime / segment);

		// std::cout << "Time: " << p_fTime << "Id : " << ptId[1] << ", Interval : " << interval << std::endl;

		// return ((m_vertexList[ptId[2]] - m_vertexList[ptId[1]]) * interval) + m_vertexList[ptId[1]];

		/* */
		return Illumina::Core::Spline::LaGrange(m_vertexList, p_fTime);

		return Illumina::Core::Spline::Hermite(m_vertexList[ptId[1]], m_vertexList[ptId[2]], 
			Vector3::Normalize(m_vertexList[ptId[1]] - m_vertexList[ptId[0]]), Vector3::Normalize(m_vertexList[ptId[3]] - m_vertexList[ptId[2]]),
			2.0f, interval);
		/* */
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
