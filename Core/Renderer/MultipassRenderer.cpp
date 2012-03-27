//----------------------------------------------------------------------------------------------
//	Filename:	MultipassRenderer.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "boost/progress.hpp"

#include "Renderer/MultipassRenderer.h"
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
MultipassRenderer::MultipassRenderer(Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, 
	RadianceBuffer *p_pRadianceBuffer, int p_nSampleCount, bool p_bCombined, int p_nDBSize, float p_fDBDist, float p_fDBCos)
	: BaseRenderer(p_pScene, p_pIntegrator, p_pDevice, p_pFilter, p_pRadianceBuffer, p_nSampleCount)
	, m_bUseCombinedPass(p_bCombined)
	, m_nDBSize(p_nDBSize)
	, m_fDBDist(p_fDBDist)
	, m_fDBCos(p_fDBCos)
{ }
//----------------------------------------------------------------------------------------------
MultipassRenderer::MultipassRenderer(const std::string &p_strName, Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, 
	IFilter *p_pFilter, RadianceBuffer *p_pRadianceBuffer, int p_nSampleCount, bool p_bCombined, int p_nDBSize, float p_fDBDist, float p_fDBCos)
	: BaseRenderer(p_strName, p_pScene, p_pIntegrator, p_pDevice, p_pFilter, p_pRadianceBuffer, p_nSampleCount)
	, m_bUseCombinedPass(p_bCombined)
	, m_nDBSize(p_nDBSize)
	, m_fDBDist(p_fDBDist)
	, m_fDBCos(p_fDBCos)
{ }
//----------------------------------------------------------------------------------------------
/*
MultipassRenderer::MultipassRenderer(Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, int p_nSampleCount)
	: IRenderer(p_pScene, p_pIntegrator, p_pDevice, p_pFilter)
	, m_nSampleCount(p_nSampleCount)
{ }
//----------------------------------------------------------------------------------------------
MultipassRenderer::MultipassRenderer(const std::string &p_strName, Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, int p_nSampleCount)
	: IRenderer(p_strName, p_pScene, p_pIntegrator, p_pDevice, p_pFilter)
	, m_nSampleCount(p_nSampleCount)
{ }
*/
//----------------------------------------------------------------------------------------------
bool MultipassRenderer::Initialise(void)
{
	BOOST_ASSERT(m_pDevice != NULL);

	m_nWidth = m_pDevice->GetWidth();
	m_nHeight = m_pDevice->GetHeight();

	m_pRadianceBuffer = new RadianceBuffer(m_nWidth, m_nHeight);

	return true;
}
//----------------------------------------------------------------------------------------------
bool MultipassRenderer::Shutdown(void)
{
	delete m_pRadianceBuffer;

	return true;
}
/*
//----------------------------------------------------------------------------------------------
void MultipassRenderer::RenderRegion(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, 
	RadianceBuffer *p_pRadianceBuffer, int p_nBufferX, int p_nBufferY)
{
	BOOST_ASSERT(m_pScene != NULL && m_pIntegrator != NULL && m_pFilter != NULL && 
		m_pScene->GetCamera() != NULL && m_pScene->GetSpace() != NULL && m_pScene->GetSampler() != NULL);

	RenderRegionToBuffer(p_nRegionX, p_nRegionY, p_nRegionWidth, p_nRegionHeight, p_pRadianceBuffer, p_nBufferX, p_nBufferY);
}
//----------------------------------------------------------------------------------------------
void MultipassRenderer::RenderRegion(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight)
{
	BOOST_ASSERT(m_pScene != NULL && m_pIntegrator != NULL && m_pFilter != NULL && 
		m_pScene->GetCamera() != NULL && m_pScene->GetSpace() != NULL && m_pScene->GetSampler() != NULL);

	RenderRegionToBuffer(p_nRegionX, p_nRegionY, p_nRegionWidth, p_nRegionHeight, m_pRadianceBuffer, p_nRegionX, p_nRegionY);
	WriteRadianceBufferToDevice(p_nRegionX, p_nRegionY, p_nRegionWidth, p_nRegionHeight, m_pRadianceBuffer, p_nRegionX, p_nRegionY);
}  
//----------------------------------------------------------------------------------------------
void MultipassRenderer::Render(RadianceBuffer *p_pRadianceBuffer, int p_nBufferX, int p_nBufferY)
{
	BOOST_ASSERT(m_pScene != NULL && m_pIntegrator != NULL && m_pFilter != NULL && 
		m_pScene->GetCamera() != NULL && m_pScene->GetSpace() != NULL && m_pScene->GetSampler() != NULL);

	int height = m_pDevice->GetHeight(),
		width = m_pDevice->GetWidth();

	RenderRegionToBuffer(0, 0, width, height, p_pRadianceBuffer, p_nBufferX, p_nBufferY);
}
//----------------------------------------------------------------------------------------------
void MultipassRenderer::Render(void)
{
	const int updateRate = 5;
	static int updateIO = updateRate;

	if (++updateIO >= updateRate) 
		updateIO = 0;

	BOOST_ASSERT(m_pScene != NULL && m_pIntegrator != NULL && m_pFilter != NULL && 
		m_pScene->GetCamera() != NULL && m_pScene->GetSpace() != NULL && m_pScene->GetSampler() != NULL);

	int height = m_pDevice->GetHeight(),
		width = m_pDevice->GetWidth();

	if (!updateIO) m_pDevice->BeginFrame();
	
	RenderRegionToBuffer(0, 0, width, height, m_pRadianceBuffer, 0, 0);
	WriteRadianceBufferToDevice(0, 0, width, height, m_pRadianceBuffer, 0, 0);

	if(!updateIO) { m_pDevice->EndFrame(); }
}
//----------------------------------------------------------------------------------------------
void MultipassRenderer::RenderRegionToBuffer(int p_nRegionX, int p_nRegionY, 
	int p_nRegionWidth, int p_nRegionHeight, RadianceBuffer *p_pRadianceBuffer, 
	int p_nBufferX, int p_nBufferY)
{
	// If no radiance buffer is specified, use in-built one
	BOOST_ASSERT(p_pRadianceBuffer != NULL);

	RadianceContext *pRadianceContext,
		accumulator;

	Intersection intersection;
	IntegratorContext context;

	// Compute tile bounds
	int regionXEnd = p_nRegionX + p_nRegionWidth,
		regionYEnd = p_nRegionY + p_nRegionHeight;

	// Compute dimension reciprocals for normalisation
	float rcpWidth = 1.f / m_nWidth,
		rcpHeight = 1.f / m_nHeight,
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
				// Reset sampler
				// m_pScene->GetSampler()->Reset();

				// Get radiance context
				pRadianceContext = &(p_pRadianceBuffer->Get(dstX, dstY));

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
				// Reset sampler
				// m_pScene->GetSampler()->Reset();

				// Get radiance context
				pRadianceContext = &(p_pRadianceBuffer->Get(dstX, dstY));

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
*/
//----------------------------------------------------------------------------------------------
void MultipassRenderer::WriteRadianceBufferToDevice(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, RadianceBuffer *p_pRadianceBuffer, int p_nDeviceX, int p_nDeviceY)
{
	RadianceContext *pRadianceContext;

	//----------------------------------------------------------------------------------------------
	for (int srcY = p_nRegionY, dstY = p_nDeviceY; srcY < p_pRadianceBuffer->GetHeight(); ++srcY, ++dstY)
	{
		for (int srcX = p_nRegionX, dstX = p_nDeviceX; srcX < p_pRadianceBuffer->GetWidth(); ++srcX, ++dstX)
		{
			m_pDevice->Set(m_nWidth - (dstX + 1), m_nHeight - (dstY + 1), p_pRadianceBuffer->Get(srcX, srcY).Final);
		}
	}
}
//----------------------------------------------------------------------------------------------
void MultipassRenderer::PostProcess(RadianceBuffer *p_pRadianceBuffer)
{
	RadianceContext *pRadianceContext = p_pRadianceBuffer->GetBuffer();

	//----------------------------------------------------------------------------------------------
	for (int y = 0; y < m_nHeight; ++y)
	{
		const int ys = Maths::Max(y - m_nDBSize, 0);
		const int ye = Maths::Min(y + m_nDBSize, m_nHeight);

		for (int x = 0; x < m_nWidth; ++x)
		{
			if (!pRadianceContext->Flag)
			{
				const int xs = Maths::Max(x - m_nDBSize, 0);
				const int xe = Maths::Min(x + m_nDBSize, m_nWidth);

				Spectrum Li = 0.f;
				int irradianceSamples = 1;

				for (int dy = ys; dy < ye; dy++)
				{
					RadianceContext *pInnerContext = &(p_pRadianceBuffer->Get(xs, dy));

					for (int dx = xs; dx < xe; dx++)
					{
						if (
							//(Vector3::DistanceSquared(pRadianceContext->Position, pInnerContext->Position) < m_fDBDist) &&
							(Vector3::Dot(pRadianceContext->Normal, pInnerContext->Normal) > m_fDBCos))
						{
							Li += pInnerContext->Indirect;
							irradianceSamples++;
						}

						pInnerContext++;
					}
				}
			
				pRadianceContext->Final = pRadianceContext->Direct + (Li * pRadianceContext->Albedo) / irradianceSamples;
				// pRadianceContext->Final = pRadianceContext->Direct + pRadianceContext->Indirect;
			}
			else
			{
				pRadianceContext->Final = pRadianceContext->Direct + pRadianceContext->Final * pRadianceContext->Albedo;
			}

			// Set value in device
			m_pDevice->Set(m_nWidth - (x + 1), m_nHeight - (y + 1), pRadianceContext->Final);

			// Move to next context element
			pRadianceContext++;
		}
	}
}
//----------------------------------------------------------------------------------------------
void MultipassRenderer::PostProcessRegion(RadianceBuffer *p_pRadianceBuffer)
{
	RadianceContext *pContext = p_pRadianceBuffer->GetBuffer();

	int nHeight = p_pRadianceBuffer->GetHeight();
	int nWidth = p_pRadianceBuffer->GetWidth();

	for (int index = 0; index < p_pRadianceBuffer->GetArea(); ++index, ++pContext)
		pContext->Flag = 0;

	//----------------------------------------------------------------------------------------------
	for (int y = m_nDBSize; y < nHeight - m_nDBSize; ++y)
	{
		const int ys = y - m_nDBSize;
		const int ye = y + m_nDBSize;

		for (int x = m_nDBSize; x < nWidth - m_nDBSize; ++x)
		{
			Spectrum Li = 0.f;
			int irradianceSamples = 1.f;

			const int xs = x - m_nDBSize;
			const int xe = x + m_nDBSize;

			pContext = &(p_pRadianceBuffer->Get(x, y));

			for (int dy = ys; dy < ye; dy++)
			{
				RadianceContext *pInnerContext = &(p_pRadianceBuffer->Get(xs, dy));

				for (int dx = xs; dx < xe; dx++)
				{
					if (
					   //(Vector3::DistanceSquared(pRadianceContext->Position, pInnerContext->Position) < m_fDBDist) &&
						(Vector3::Dot(pContext->Normal, pInnerContext->Normal) > m_fDBCos))
					{
						Li += pInnerContext->Indirect;
						irradianceSamples++;
					}

					pInnerContext++;
				}
			}
			
			// Compute final colour
			pContext->Final = Li / irradianceSamples;
			pContext->Flag = 1;
		}
	}
}
