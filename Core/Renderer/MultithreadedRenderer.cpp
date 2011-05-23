//----------------------------------------------------------------------------------------------
//	Filename:	BasicRenderer.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "boost/progress.hpp"

#include "Renderer/MultithreadedRenderer.h"
#include "Integrator/Integrator.h"
#include "Geometry/Intersection.h"
#include "Spectrum/Spectrum.h"
#include "Camera/Camera.h"
#include "Device/Device.h"
#include "Scene/Scene.h"

#include "Sampler/Sampler.h"
#include "Filter/Filter.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
MultithreadedRenderer::MultithreadedRenderer(Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, int p_nSampleCount)
	: IRenderer(p_pScene, p_pIntegrator, p_pDevice, p_pFilter)
	, m_nSampleCount(p_nSampleCount)
{ }
//----------------------------------------------------------------------------------------------
MultithreadedRenderer::MultithreadedRenderer(const std::string &p_strName, Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, int p_nSampleCount)
	: IRenderer(p_strName, p_pScene, p_pIntegrator, p_pDevice, p_pFilter)
	, m_nSampleCount(p_nSampleCount)
{ }
//----------------------------------------------------------------------------------------------
void MultithreadedRenderer::Render(void)
{
	BOOST_ASSERT(m_pScene != NULL && m_pIntegrator != NULL && m_pFilter != NULL && 
		m_pScene->GetCamera() != NULL && m_pScene->GetSpace() != NULL && m_pScene->GetSampler() != NULL);

	int height = m_pDevice->GetHeight(),
		width = m_pDevice->GetWidth();

	m_pDevice->BeginFrame();
	
	boost::progress_display renderProgress(height);

	m_pIntegrator->Prepare(m_pScene);

	#pragma omp parallel for schedule(guided)
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
				Ray ray = m_pScene->GetCamera()->GetRay((x + pSampleBuffer[sample].U) / width, (y + pSampleBuffer[sample].V) / height, pSampleBuffer[sample].U, pSampleBuffer[sample].V);
				//Ray ray = m_pCamera->GetRay((x + pSampleBuffer[sample].U) / width, (y + pSampleBuffer[sample].V) / height, pSampleBuffer[sample].U, pSampleBuffer[sample].V);
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