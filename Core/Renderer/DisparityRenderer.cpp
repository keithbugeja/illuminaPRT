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

	Intersection intersection;
	IntegratorContext context;

	RadianceContext *pRadianceContext;
	
	// Reciprocal of device width and height, for unit normalisation
	float rcpWidth = 1.f / m_pDevice->GetWidth(),
		  rcpHeight = 1.f / m_pDevice->GetHeight();

	// Compute tile bounds
	int regionXEnd = p_nRegionX + p_nRegionWidth,
		regionYEnd = p_nRegionY + p_nRegionHeight;

	// Get 2d sample
	Vector2 sample = 
		m_pScene->GetSampler()->Get2DSample();

	// No supersampling
	context.SampleIndex = 0;
	context.SampleCount = 1;

	// If radiance buffer is available and doesn't fit required dimensions, delete
	if (m_pRadianceBuffer != NULL &&
		(m_pRadianceBuffer->GetWidth() < p_pRadianceBuffer->GetWidth() || 
		 m_pRadianceBuffer->GetHeight() < p_pRadianceBuffer->GetHeight()))
		Safe_Delete(m_pRadianceBuffer);

	if (m_pRadianceBuffer == NULL)
		m_pRadianceBuffer = new RadianceBuffer(p_pRadianceBuffer->GetWidth(), p_pRadianceBuffer->GetHeight());

	int step = 4;

	// Determine indirect frequency
	for (int srcY = p_nRegionY, dstY = 0; srcY <= regionYEnd; srcY += step, dstY++)
	{
		for (int srcX = p_nRegionX, dstX = 0; srcX <= regionXEnd; srcX += step, dstX++)
		{
			pRadianceContext = m_pRadianceBuffer->GetP(dstX, dstY);

			// Set integrator context
			context.SurfacePosition.Set((float)srcX, (float)srcY);
			context.NormalisedPosition.Set(context.SurfacePosition.X * rcpWidth, context.SurfacePosition.Y * rcpHeight);

			// Get ray from camera
			m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, sample.U * rcpWidth, sample.V * rcpHeight, pRadianceContext->ViewRay);

			// Compute Radiance
			context.SurfacePosition.Set((float)(srcX / step), (float)(srcY / step));
			pRadianceContext->Final = m_pIntegrator->Radiance(&context, m_pScene, pRadianceContext->ViewRay, intersection, pRadianceContext);
		}
	}

	RadianceContext *rad00, *rad01;
	Spectrum i0, i1, i2, i3;
	int pixelX, pixelY;
	float t0, t1;

	for (int srcY = p_nRegionY, dstY = p_nBufferY, iY = 0; 
			srcY < regionYEnd; ++srcY, ++dstY, ++iY)
	{
		// Get radiance context
		pRadianceContext = p_pRadianceBuffer->GetP(p_nBufferX, dstY);

		for (int srcX = p_nRegionX, iX = 0; 
				srcX < regionXEnd; ++srcX, ++iX)
		{
			// Set integrator context
			context.SurfacePosition.Set((float)srcX, (float)srcY);
			context.NormalisedPosition.Set(context.SurfacePosition.X * rcpWidth, context.SurfacePosition.Y * rcpHeight);

			// Get ray from camera
			m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, sample.U * rcpWidth, sample.V * rcpHeight, pRadianceContext->ViewRay);

			// Get pixel coordinates	
			pixelX = iX / step;
			pixelY = iY / step;

			// If we haven't drawn this
			if (iX % step != 0 || iY % step != 0)
			{
				// Compute direct lighting
				IIntegrator::Direct(&context, m_pScene, pRadianceContext->ViewRay, intersection, pRadianceContext);
				
				// Bilinear interpolation of indirect lighting (upscale)
				t0 = 1.f - Maths::Frac((float)iX / step);
				t1 = 1.f - Maths::Frac((float)iY / step);

				rad00 = m_pRadianceBuffer->GetP(pixelX, pixelY);
				rad01 = m_pRadianceBuffer->GetP(pixelX, pixelY + 1);

				i0 = rad00->Indirect; rad00++;
				i1 = rad00->Indirect;
				i2 = rad01->Indirect; rad01++;
				i3 = rad01->Indirect;

				pRadianceContext->Indirect = 
					(((i0 * t0) + (i1 * (1 - t0))) * t1) + 
					(((i2 * t0) + (i3 * (1 - t0))) * (1 - t1)); 
			}
			else
			{
				// Use values from precomputed buffer
				rad00 = m_pRadianceBuffer->GetP(pixelX, pixelY);

				pRadianceContext->Albedo = rad00->Albedo;
				pRadianceContext->Direct = rad00->Direct;
				pRadianceContext->Indirect = rad00->Indirect;
				pRadianceContext->Distance = rad00->Distance;
			}

			// Compute final radiance value
			pRadianceContext->Final = pRadianceContext->Indirect * pRadianceContext->Albedo + pRadianceContext->Direct;
			// pRadianceContext->Final = pRadianceContext->Indirect;

			// Move to adjacent radiance texel
			pRadianceContext++;
		}
	}
}
