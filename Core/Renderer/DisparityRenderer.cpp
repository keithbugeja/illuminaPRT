//----------------------------------------------------------------------------------------------
//	Filename:	DisparityRenderer.cpp
//	Author:		Keith Bugeja
//	Date:		27/04/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Renderer/DisparityRenderer.h"
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
DisparityRenderer::DisparityRenderer(const std::string &p_strName, Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, RadianceBuffer *p_pRadianceBuffer, int p_nSampleCount)
	: BaseRenderer(p_strName, p_pScene, p_pIntegrator, p_pDevice, p_pFilter, p_pRadianceBuffer, p_nSampleCount)
{ }
//----------------------------------------------------------------------------------------------
DisparityRenderer::DisparityRenderer(Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, RadianceBuffer *p_pRadianceBuffer, int p_nSampleCount)
	: BaseRenderer(p_pScene, p_pIntegrator, p_pDevice, p_pFilter, p_pRadianceBuffer, p_nSampleCount)
{ }
//----------------------------------------------------------------------------------------------
void DisparityRenderer::RenderRegion(RadianceBuffer *p_pRadianceBuffer, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, int p_nBufferX, int p_nBufferY) 
{
	BOOST_ASSERT(m_pScene != NULL && m_pIntegrator != NULL && m_pFilter != NULL && m_pScene->GetCamera() != NULL && 
		m_pScene->GetSpace() != NULL && m_pScene->GetSampler() != NULL);

	RadianceContext *pRadianceContext,
		accumulator;

	Intersection intersection;
	IntegratorContext context;
    
	// Compute dimension reciprocals for normalisation
	float rcpWidth = 1.f / m_pDevice->GetWidth(),
        rcpHeight = 1.f / m_pDevice->GetHeight();
    
	double startTime = Platform::GetTime();

	// Handle supersampling independently
	if (m_nSampleCount > 1)
	{
		/*
		// Get sample stream
		Vector2 *pSampleBuffer = new Vector2[m_nSampleCount];

		// Render tile
		for (int srcY = p_nRegionY, dstY = p_nBufferY; srcY < regionYEnd; ++srcY, ++dstY)
		{
			for (int srcX = p_nRegionX, dstX = p_nBufferX; srcX < regionXEnd; ++srcX, ++dstX)
			{
				// Get radiance context
				pRadianceContext = p_pRadianceBuffer->GetP(dstX, dstY);

				// Reset accumulator
				accumulator.Final = 
					accumulator.Indirect =
					accumulator.Direct = 
					accumulator.Albedo = 0.f;

				// Get samples and filter them
				m_pScene->GetSampler()->Get2DSamples(pSampleBuffer, m_nSampleCount);
				(*m_pFilter)(pSampleBuffer, m_nSampleCount);

				// Set context (used in IGI interleaved sampling)
				context.SurfacePosition.Set(srcX, srcY);
				context.NormalisedPosition.Set(context.SurfacePosition.X * rcpWidth, context.SurfacePosition.Y * rcpHeight);

				// Super sample
				for (context.SampleIndex = 0; context.SampleIndex < m_nSampleCount; context.SampleIndex++)
				{
					// Fetch a ray from camera (should use ray differentials to speed this up)
					m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, 
						pSampleBuffer[context.SampleIndex].U * rcpWidth, pSampleBuffer[context.SampleIndex].V * rcpHeight, pRadianceContext->ViewRay);
				
					// Get radiance
					accumulator.Final += m_pIntegrator->Radiance(&context, m_pScene, pRadianceContext->ViewRay, intersection, pRadianceContext);

					// Accumulate components
					accumulator.Direct += pRadianceContext->Direct;
					accumulator.Indirect += pRadianceContext->Indirect;
					accumulator.Albedo += pRadianceContext->Albedo;
				}

				// Set final values
				pRadianceContext->Final = accumulator.Final * rcpSampleCount;
				pRadianceContext->Direct = accumulator.Direct * rcpSampleCount;
				pRadianceContext->Indirect = accumulator.Indirect * rcpSampleCount;
				pRadianceContext->Albedo = accumulator.Albedo * rcpSampleCount;
				
				//pRadianceContext->Final.Set(Maths::Abs(pRadianceContext->Normal.Element[0]),
				//	Maths::Abs(pRadianceContext->Normal.Element[1]), 
				//	Maths::Abs(pRadianceContext->Normal.Element[2]));
				
			}
		}

		delete[] pSampleBuffer;
		*/
	}
	else
	{
		const int indirectFrequency = 0x03; //0x01;
		const int indirectFrequencyMask = 0xFFFFFFFC; //0xFFFFFFFE;

		// Compute tile bounds
		int regionXEnd = p_nRegionX + p_nRegionWidth,
			regionYEnd = p_nRegionY + p_nRegionHeight;

		Vector2 sample = 
			m_pScene->GetSampler()->Get2DSample();

		// No supersampling
		context.SampleIndex = 0;

		// Render tile
		for (int srcY = p_nRegionY, dstY = p_nBufferY, iY = 0; srcY < regionYEnd; ++srcY, ++dstY, iY++)
		{
			for (int srcX = p_nRegionX, dstX = p_nBufferX, iX = 0; srcX < regionXEnd; ++srcX, ++dstX, iX++)
			{
				// Get radiance context
				pRadianceContext = p_pRadianceBuffer->GetP(dstX, dstY);

				// Set integrator context
				context.SurfacePosition.Set((float)srcX, (float)srcY);
				context.NormalisedPosition.Set(context.SurfacePosition.X * rcpWidth, context.SurfacePosition.Y * rcpHeight);

				// Get ray from camera
				m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, sample.U * rcpWidth, sample.V * rcpHeight, pRadianceContext->ViewRay);

				//m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, 0.5f * rcpWidth, 0.5f * rcpHeight, pRadianceContext->ViewRay);
				// pRadianceContext->Final = m_pIntegrator->Radiance(&context, m_pScene, pRadianceContext->ViewRay, intersection, pRadianceContext);

				// Get radiance
				if (((iX & indirectFrequency) | (iY & indirectFrequency)) != 0)
				{
					IIntegrator::Direct(&context, m_pScene, pRadianceContext->ViewRay, intersection, pRadianceContext);

					pRadianceContext->Indirect =
						p_pRadianceBuffer->GetP(p_nBufferX + (iX & indirectFrequencyMask), p_nBufferY + (iY & indirectFrequencyMask))->Indirect;
					
					pRadianceContext->Final = pRadianceContext->Indirect * pRadianceContext->Albedo + pRadianceContext->Direct;
				}
				else
				{
					pRadianceContext->Final = m_pIntegrator->Radiance(&context, m_pScene, pRadianceContext->ViewRay, intersection, pRadianceContext);
				}	

				//pRadianceContext->Direct = 0;
				//pRadianceContext->Final = pRadianceContext->Indirect * pRadianceContext->Albedo;
			}
		}
		/**/
	}
}