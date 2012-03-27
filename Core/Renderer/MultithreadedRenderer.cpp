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
MultithreadedRenderer::MultithreadedRenderer(Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, RadianceBuffer *p_pRadianceBuffer, int p_nSampleCount)
	: BaseRenderer(p_pScene, p_pIntegrator, p_pDevice, p_pFilter, p_pRadianceBuffer, p_nSampleCount)
{ }
//----------------------------------------------------------------------------------------------
MultithreadedRenderer::MultithreadedRenderer(const std::string &p_strName, Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, RadianceBuffer *p_pRadianceBuffer, int p_nSampleCount)
	: BaseRenderer(p_strName, p_pScene, p_pIntegrator, p_pDevice, p_pFilter, p_pRadianceBuffer, p_nSampleCount)
{ }
//----------------------------------------------------------------------------------------------
void MultithreadedRenderer::Render(void)
{
	/*
	const int updateRate = 5;
	static int updateIO = updateRate;

	if (++updateIO >= updateRate) 
		updateIO = 0;

	BOOST_ASSERT(m_pScene != NULL && m_pIntegrator != NULL && m_pFilter != NULL && 
		m_pScene->GetCamera() != NULL && m_pScene->GetSpace() != NULL && m_pScene->GetSampler() != NULL);

	int height = m_pDevice->GetHeight(),
		width = m_pDevice->GetWidth();

	if (!updateIO) m_pDevice->BeginFrame();
	
	boost::progress_display renderProgress(height);

	m_pIntegrator->Prepare(m_pScene);

	#pragma omp parallel for schedule(static, 8) num_threads(4)
	for (int y = 0; y < height; ++y)
	{
		Vector2 *pSampleBuffer = new Vector2[m_nSampleCount];

		Intersection intersection;
		IntegratorContext context;

		for (int x = 0; x < width; x++)
		{
			// Prepare ray samples
			m_pScene->GetSampler()->Get2DSamples(pSampleBuffer, m_nSampleCount);
			(*m_pFilter)(pSampleBuffer, m_nSampleCount);

			// Radiance
			Spectrum Li = 0;

			for (int sample = 0; sample < m_nSampleCount; sample++)
			{
				context.SampleIndex = sample;
				context.SurfacePosition.Set(x + pSampleBuffer[sample].U, y + pSampleBuffer[sample].V);
				context.NormalisedPosition.Set(context.SurfacePosition.X / width, context.SurfacePosition.Y / height);

				Ray ray = m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, pSampleBuffer[sample].U, pSampleBuffer[sample].V);

				Li += m_pIntegrator->Radiance(&context, m_pScene, ray, intersection);
			}

			Li = Li / m_nSampleCount;
			
			m_pDevice->Set(width - (x + 1), height - (y + 1), Li);
		}

		delete[] pSampleBuffer;
		
		++renderProgress;
	}

	if(!updateIO) m_pDevice->EndFrame();
	*/
}
//----------------------------------------------------------------------------------------------