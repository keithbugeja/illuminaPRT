//----------------------------------------------------------------------------------------------
//	Filename:	BasicRenderer.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "boost/progress.hpp"

#include "Renderer/BasicRenderer.h"
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
BasicRenderer::BasicRenderer(Scene *p_pScene, ICamera *p_pCamera, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, int p_nSampleCount)
	: m_pIntegrator(p_pIntegrator)
	, m_pCamera(p_pCamera)
	, m_pDevice(p_pDevice)
	, m_pFilter(p_pFilter)
	, m_pScene(p_pScene)
	, m_nSampleCount(p_nSampleCount)
{ }
//----------------------------------------------------------------------------------------------
void BasicRenderer::Render(void)
{
	int height = m_pDevice->GetHeight(),
		width = m_pDevice->GetWidth();

	m_pDevice->BeginFrame();
	
	boost::progress_display renderProgress(height);

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