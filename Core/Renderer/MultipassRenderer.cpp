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

static int g_magicSquare[] = {
	0, 108, 35, 135, 7, 146, 36, 134, 11, 204, 37, 133, 1, 194, 95, 38,
	145, 39, 171, 40, 164, 99, 136, 41, 220, 94, 203, 132, 202, 42, 231, 195,
	123, 165, 16, 254, 100, 255, 17, 219, 101, 222, 131, 18, 192, 103, 115, 19,
	170, 92, 172, 43, 97, 161, 162, 98, 221, 44, 232, 104, 201, 193, 45, 196,
	12, 166, 96, 218, 2, 122, 90, 160, 13, 111, 91, 130, 8, 114, 198, 197,
	167, 46, 173, 120, 121, 47, 159, 93, 233, 110, 190, 102, 200, 48, 113, 105,
	49, 229, 20, 50, 214, 215, 21, 147, 51, 112, 22, 191, 52, 142, 199, 23,
	168, 228, 53, 119, 216, 54, 213, 148, 234, 55, 188, 56, 250, 249, 57, 251,
	4, 240, 150, 219, 9, 212, 151, 158, 14, 107, 189, 149, 3, 141, 143, 106,
	169, 58, 236, 59, 235, 60, 211, 61, 136, 187, 152, 62, 140, 248, 63, 252,
	64, 227, 24, 237, 118, 20, 25, 183, 253, 137, 26, 139, 247, 65, 144, 27,
	15, 124, 238, 66, 5, 127, 67, 184, 6, 186, 138, 68, 10, 157, 244, 69,
	220, 70, 125, 239, 71, 128, 241, 72, 185, 73, 175, 109, 246, 74, 116, 156,
	75, 206, 28, 126, 209, 74, 29, 182, 77, 225, 30, 174, 78, 243, 117, 31,
	205, 78, 207, 80, 208, 192, 240, 81, 224, 82, 176, 83, 245, 153, 84, 155,
	85, 179, 86, 180, 32, 181, 87, 223, 178, 88, 33, 177, 242, 89, 154, 34 };

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
MultipassRenderer::MultipassRenderer(Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, int p_nSampleCount,
	bool p_bCombined, int p_nDBSize, float p_fDBDist, float p_fDBCos)
	: IRenderer(p_pScene, p_pIntegrator, p_pDevice, p_pFilter)
	, m_nSampleCount(p_nSampleCount)
	, m_bUseCombinedPass(p_bCombined)
	, m_nDBSize(p_nDBSize)
	, m_fDBDist(p_fDBDist)
	, m_fDBCos(p_fDBCos)
{ }
//----------------------------------------------------------------------------------------------
MultipassRenderer::MultipassRenderer(const std::string &p_strName, Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, int p_nSampleCount,
	bool p_bCombined, int p_nDBSize, float p_fDBDist, float p_fDBCos)
	: IRenderer(p_strName, p_pScene, p_pIntegrator, p_pDevice, p_pFilter)
	, m_nSampleCount(p_nSampleCount)
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
