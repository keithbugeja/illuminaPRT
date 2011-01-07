//----------------------------------------------------------------------------------------------
//	Filename:	BasicRenderer.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <vector>

#include "boost/progress.hpp"
#include "boost/mpi.hpp"

#include "Renderer/DistributedRenderer.h"
#include "Integrator/Integrator.h"
#include "Geometry/Intersection.h"
#include "Spectrum/Spectrum.h"
#include "Camera/Camera.h"
#include "Device/Device.h"
#include "Staging/Scene.h"

#include "Sampler/Sampler.h"
#include "Filter/Filter.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
DistributedRenderer::DistributedRenderer(Scene *p_pScene, ICamera *p_pCamera, IIntegrator *p_pIntegrator, 
					IDevice *p_pDevice, IFilter *p_pFilter, int p_nSampleCount, int p_nTileWidth, int p_nTileHeight)
	: m_pScene(p_pScene)
	, m_pCamera(p_pCamera)
	, m_pIntegrator(p_pIntegrator)
	, m_pDevice(p_pDevice)
	, m_pFilter(p_pFilter)
	, m_nSampleCount(p_nSampleCount)
	, m_nTileWidth(p_nTileWidth)
	, m_nTileHeight(p_nTileHeight)
{ 
}
//----------------------------------------------------------------------------------------------
bool DistributedRenderer::Initialise(void)
{
	m_pMPIEnvironment = new mpi::environment();
	m_pMPICommunicator = new mpi::communicator();

	m_pMPICommunicator->barrier();

	return true;
}
//----------------------------------------------------------------------------------------------
bool DistributedRenderer::Shutdown(void)
{
	delete m_pMPICommunicator;
	delete m_pMPIEnvironment;

	return true;
}
//----------------------------------------------------------------------------------------------
void DistributedRenderer::Render(void)
{
	const int workItemRequest = 0x0020;
	const int workItemPackage = 0x0010;
	const int terminate = 0x0001;

	int height = m_pDevice->GetHeight(),
		width = m_pDevice->GetWidth();

	int tileCount = (width / m_nTileWidth) * (height / m_nTileHeight);

	// Synchronise all processes
	m_pMPICommunicator->barrier();

	// master or slave?
	if (m_pMPICommunicator->rank() == 0)
	{
		int dummy;

		std::vector<int> m_taskQueue;

		// generate tiles for rendering
		for (int taskId = 0; taskId < tileCount; ++taskId)
		{
			m_taskQueue.push_back(taskId);
		}

		while(m_taskQueue.size())
		{
			mpi::status status = m_pMPICommunicator->recv(mpi::any_source, workItemRequest);
			
			dummy = m_taskQueue.back();
			m_taskQueue.pop_back();

			m_pMPICommunicator->send(status.source(), workItemPackage, dummy);
		}

		for (int rank = 1; rank < m_pMPICommunicator->size(); rank++)
			m_pMPICommunicator->send(rank, terminate);
	}
	else
	{
		while (true)
		{
			int dummy = -1;

			m_pMPICommunicator->send(0, workItemRequest);
			mpi::status status = m_pMPICommunicator->recv(0, mpi::any_tag, dummy);

			if (status.tag() == terminate)
				break;

			// render something
			std::cout<<"Rendering tile " << dummy << "..." << std::endl;
		}
	}

	m_pMPICommunicator->barrier();

	// Server has work unit queue
	// Server is ready to furnish slaves with more work
	// Slaves request work and store it in local ready list
	
	// Barrier - Work queue expended 
	// Slaves send results back to server
	// Server composits final image


	m_pDevice->BeginFrame();
	
	boost::progress_display renderProgress(height);

	m_pMPICommunicator->size();

	//#pragma omp parallel for schedule(guided)
	for (int y = 0; y < height; ++y)
	{
		Vector2 *pSampleBuffer = new Vector2[m_nSampleCount];

		Intersection intersection;

		for (int x = 0; x < width; x++)
		{
			// Prepare ray samples
			m_pScene->GetSampler()->Get2DSamples(pSampleBuffer, m_nSampleCount);
			(*m_pFilter)(pSampleBuffer, m_nSampleCount);

			// Radiance
			Spectrum Li = 0;

			for (int sample = 0; sample < m_nSampleCount; sample++)
			{
				Ray ray = m_pCamera->GetRay(((float)x) / width, ((float)y) / height, pSampleBuffer[sample].U, pSampleBuffer[sample].V);
				Li += m_pIntegrator->Radiance(m_pScene, ray, intersection);
			}

			Li = Li / m_nSampleCount;
			
			m_pDevice->Set(width - (x + 1), height - (y + 1), Li);
		}

		delete[] pSampleBuffer;
		
		++renderProgress;
	}

	m_pDevice->EndFrame();
}
//----------------------------------------------------------------------------------------------