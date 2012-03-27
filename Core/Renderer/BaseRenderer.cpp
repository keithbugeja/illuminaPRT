//----------------------------------------------------------------------------------------------
//	Filename:	BaseRenderer.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Renderer/BasicRenderer.h"
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
BaseRenderer::BaseRenderer(const std::string &p_strName, Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, RadianceBuffer* p_pRadianceBuffer, int p_nSampleCount)
	: IRenderer(p_strName, p_pScene, p_pIntegrator, p_pDevice, p_pFilter, p_pRadianceBuffer)
	, m_nSampleCount(p_nSampleCount)
{ }
//----------------------------------------------------------------------------------------------
BaseRenderer::BaseRenderer(Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, RadianceBuffer* p_pRadianceBuffer, int p_nSampleCount)
	: IRenderer(p_pScene, p_pIntegrator, p_pDevice, p_pFilter, p_pRadianceBuffer)
	, m_nSampleCount(p_nSampleCount)
{ }
//----------------------------------------------------------------------------------------------
void BaseRenderer::Render(void)
{
	BOOST_ASSERT(m_pRadianceBuffer != NULL);

	BOOST_ASSERT(m_pRadianceBuffer->GetWidth() == m_pDevice->GetWidth() &&
		m_pRadianceBuffer->GetHeight() == m_pDevice->GetHeight());

	RenderRegion(m_pRadianceBuffer, 0, 0, m_pRadianceBuffer->GetWidth(), m_pRadianceBuffer->GetHeight());
}
//----------------------------------------------------------------------------------------------
void BaseRenderer::Render(RadianceBuffer *p_pRadianceBuffer)
{
	BOOST_ASSERT(p_pRadianceBuffer != NULL);

	BOOST_ASSERT(p_pRadianceBuffer->GetWidth() == m_pDevice->GetWidth() &&
		p_pRadianceBuffer->GetHeight() == m_pDevice->GetHeight());

	RenderRegion(p_pRadianceBuffer, 0, 0, p_pRadianceBuffer->GetWidth(), p_pRadianceBuffer->GetHeight());
}
//----------------------------------------------------------------------------------------------
void BaseRenderer::RenderRegion(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight) 
{
	BOOST_ASSERT(m_pRadianceBuffer != NULL);

	BOOST_ASSERT(m_pRadianceBuffer->GetWidth() == m_pDevice->GetWidth() &&
		m_pRadianceBuffer->GetHeight() == m_pDevice->GetHeight());

	RenderRegion(m_pRadianceBuffer, p_nRegionX, p_nRegionY, p_nRegionWidth, p_nRegionHeight);
}
//----------------------------------------------------------------------------------------------
void BaseRenderer::RenderRegion(RadianceBuffer *p_pRadianceBuffer, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, int p_nBufferX, int p_nBufferY) 
{
	BOOST_ASSERT(m_pScene != NULL && m_pIntegrator != NULL && m_pFilter != NULL && m_pScene->GetCamera() != NULL && 
		m_pScene->GetSpace() != NULL && m_pScene->GetSampler() != NULL);

	RadianceContext *pRadianceContext,
		accumulator;

	Intersection intersection;
	IntegratorContext context;

	// Compute tile bounds
	int regionXEnd = p_nRegionX + p_nRegionWidth,
		regionYEnd = p_nRegionY + p_nRegionHeight;

	// Compute dimension reciprocals for normalisation
	float rcpWidth = 1.f / m_pDevice->GetWidth(),
		rcpHeight = 1.f / m_pDevice->GetHeight(),
		rcpSampleCount = 1.f / m_nSampleCount;

	// Handle supersampling independently
	if (m_nSampleCount > 1)
	{
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
			}
		}

		delete[] pSampleBuffer;
	}
	else
	{
		// No supersampling
		context.SampleIndex = 0;

		// Render tile
		for (int srcY = p_nRegionY, dstY = p_nBufferY; srcY < regionYEnd; ++srcY, ++dstY)
		{
			for (int srcX = p_nRegionX, dstX = p_nBufferX; srcX < regionXEnd; ++srcX, ++dstX)
			{
				// Get radiance context
				pRadianceContext = p_pRadianceBuffer->GetP(dstX, dstY);

				// Set integrator context
				context.SurfacePosition.Set(srcX, srcY);
				context.NormalisedPosition.Set(context.SurfacePosition.X * rcpWidth, context.SurfacePosition.Y * rcpHeight);

				// Get ray from camera
				m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, 0.5f * rcpWidth, 0.5f * rcpHeight, pRadianceContext->ViewRay);
				
				// Get radiance
				pRadianceContext->Final = m_pIntegrator->Radiance(&context, m_pScene, pRadianceContext->ViewRay, intersection, pRadianceContext);
			}
		}
	}
}
//----------------------------------------------------------------------------------------------
void BaseRenderer::RenderTile(RadianceBuffer *p_pRadianceBuffer, int p_nTileIndex, int p_nTileWidth, int p_nTileHeight) { }
//----------------------------------------------------------------------------------------------
void BaseRenderer::Commit(void) 
{ 
	Commit(m_pRadianceBuffer);
}
//----------------------------------------------------------------------------------------------
void BaseRenderer::Commit(RadianceBuffer *p_pRadianceBuffer) 
{ 
	m_pDevice->BeginFrame();
	m_pDevice->WriteRadianceBufferToDevice(0, 0, p_pRadianceBuffer->GetWidth(), p_pRadianceBuffer->GetHeight(), p_pRadianceBuffer, 0, 0);
	m_pDevice->EndFrame();
}
//----------------------------------------------------------------------------------------------
