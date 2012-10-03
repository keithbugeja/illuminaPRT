//----------------------------------------------------------------------------------------------
//	Filename:	TimeConstrainedRenderer.cpp
//	Author:		Keith Bugeja
//	Date:		27/04/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Renderer/TimeConstrainedRenderer.h"
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
TimeConstrainedRenderer::TimeConstrainedRenderer(const std::string &p_strName, Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, RadianceBuffer *p_pRadianceBuffer, int p_nSampleCount)
	: BaseRenderer(p_strName, p_pScene, p_pIntegrator, p_pDevice, p_pFilter, p_pRadianceBuffer, p_nSampleCount)
{ }
//----------------------------------------------------------------------------------------------
TimeConstrainedRenderer::TimeConstrainedRenderer(Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, RadianceBuffer *p_pRadianceBuffer, int p_nSampleCount)
	: BaseRenderer(p_pScene, p_pIntegrator, p_pDevice, p_pFilter, p_pRadianceBuffer, p_nSampleCount)
{ }
//----------------------------------------------------------------------------------------------
void TimeConstrainedRenderer::RenderRegion(RadianceBuffer *p_pRadianceBuffer, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, int p_nBufferX, int p_nBufferY) 
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
		/*
		// Compute tile bounds
		int regionXEnd = p_nRegionX + p_nRegionWidth,
			regionYEnd = p_nRegionY + p_nRegionHeight;

		Vector2 sample = 
			m_pScene->GetSampler()->Get2DSample();

		// No supersampling
		context.SampleIndex = 0;

		const int ci = 0x01;
		const int nci = 0xFFFFFFFE;

		// Render tile
		for (int srcY = p_nRegionY, dstY = p_nBufferY; srcY < regionYEnd; ++srcY, ++dstY)
		{
			for (int srcX = p_nRegionX, dstX = p_nBufferX; srcX < regionXEnd; ++srcX, ++dstX)
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
				if (((srcX & ci) | (srcY & ci)) != 0)
				{
					IIntegrator::Direct(&context, m_pScene, pRadianceContext->ViewRay, intersection, pRadianceContext);

					pRadianceContext->Indirect = 
						p_pRadianceBuffer->GetP(dstX & nci, dstY & nci)->Indirect;
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

		/*
		RadianceContext *rc;
		Spectrum indirect;
		int kernelEntry = 4,
			kernelSize = kernelEntry * 8;
		
		/**/

		/*
		for (int srcY = p_nRegionY + kernelSize, dstY = p_nBufferY + kernelSize; srcY < regionYEnd - kernelSize; srcY+=kernelEntry, dstY+=kernelEntry)
		{
			for (int srcX = p_nRegionX + kernelSize, dstX = p_nBufferX + kernelSize; srcX < regionXEnd - kernelSize; srcX+=kernelEntry, dstX+=kernelEntry)
			{
				// Get radiance context
				pRadianceContext = p_pRadianceBuffer->GetP(dstX, dstY);
				indirect = p_pRadianceBuffer->GetP(dstX, dstY)->Indirect;

				float weight = 0;
				Spectrum _indirect = 0;
				
				for (int ky = -kernelSize; ky < kernelSize; ky+=kernelEntry)
				{
					for (int kx = -kernelSize; kx < kernelSize; kx+=kernelEntry)
					{
						// compute edge details
						rc = p_pRadianceBuffer->GetP(dstX + kx, dstY + ky);
						//float dist = Maths::Min(0.99f, Maths::Abs(pRadianceContext->Distance - rc->Distance));

						if (Vector3::Dot(rc->Normal, pRadianceContext->Normal) > 0.75)
						{
							//float dist = 
							//	Statistics::GaussianPDF(Maths::Sqr(pRadianceContext->Distance - rc->Distance), 0, 2.0f);

							//weight+=dist;
							//_indirect = _indirect + rc->Indirect * dist;

							weight++;
							_indirect = _indirect + rc->Indirect;
						}
					}
				}

				pRadianceContext->Indirect = pRadianceContext->Albedo *_indirect / weight;
				pRadianceContext->Final = pRadianceContext->Indirect + pRadianceContext->Direct;
			}
		}
		/**/

		/*
		// Naive upscaling
		for (int srcY = p_nRegionY, dstY = p_nBufferY; srcY < regionYEnd; ++srcY, ++dstY)
		{
			for (int srcX = p_nRegionX, dstX = p_nBufferX; srcX < regionXEnd; ++srcX, ++dstX)
			{
				pRadianceContext = p_pRadianceBuffer->GetP(dstX, dstY);
				pRadianceContext->Indirect = 
					p_pRadianceBuffer->GetP(dstX & 0xFFFFFFFE, dstY & 0xFFFFFFFE)->Indirect;
				pRadianceContext->Final = pRadianceContext->Indirect * pRadianceContext->Albedo + pRadianceContext->Direct;
			}
		}
		*/

		// Naive BLF
		/*
		kernelSize = 4;
		float sr = 4, ss = 1;
		int iters=1;

		for (int srcY = p_nRegionY + kernelSize, dstY = p_nBufferY + kernelSize; srcY < regionYEnd - kernelSize; ++srcY, ++dstY)
		{
			for (int srcX = p_nRegionX + kernelSize, dstX = p_nBufferX + kernelSize; srcX < regionXEnd - kernelSize; ++srcX, ++dstX)
			{
				// Get radiance context
				pRadianceContext = p_pRadianceBuffer->GetP(dstX, dstY);
				indirect = pRadianceContext->Indirect;

				float weight = 0;
				Spectrum _indirect = 0;
				
				for (int ky = -kernelSize; ky < kernelSize; ++ky)
				{
					for (int kx = -kernelSize; kx < kernelSize; ++kx)
					{
						// compute edge details
						rc = p_pRadianceBuffer->GetP(dstX + kx, dstY + ky);
						float dist = Maths::Sqr(pRadianceContext->Distance - rc->Distance);
						dist=Statistics::GaussianPDF(dist, 0, ss);

						rc = p_pRadianceBuffer->GetP(dstX + kx, dstY + ky);
						
						//float y1 = 0.2126 * pRadianceContext->Indirect[0] +
						//	0.7152 * pRadianceContext->Indirect[1] +
						//	0.0722 * pRadianceContext->Indirect[2];

						//float y2 = 0.2126 * rc->Indirect[0] +
						//	0.7152 * rc->Indirect[1] +
						//	0.0722 * rc->Indirect[2];
						
						//float cDist = Maths::Sqr(y1-y2);
						float cDist = 1;

						//float cDist = Maths::Sqr(pRadianceContext->Indirect[0] - rc->Indirect[0]) + 
						//	Maths::Sqr(pRadianceContext->Indirect[1] - rc->Indirect[1]) + 
						//	Maths::Sqr(pRadianceContext->Indirect[2] - rc->Indirect[2]);

						//cDist=Statistics::GaussianPDF(cDist, 0, sr);

						weight += dist*cDist;//(1-dist) * (1-cDist);
						_indirect = _indirect + rc->Indirect * dist * cDist; //(1-dist) * (1-cDist);
					}
				}

				//pRadianceContext->Indirect = pRadianceContext->Albedo * _indirect / weight;
				pRadianceContext->Indirect = _indirect / weight;
				// pRadianceContext->Direct = 0;
				pRadianceContext->Final = pRadianceContext->Indirect + pRadianceContext->Direct;
			}
		}
		/**/

		/*
		for (int srcY = p_nRegionY + kernelSize, dstY = p_nBufferY + kernelSize; srcY < regionYEnd - kernelSize; ++srcY, ++dstY)
		{
			for (int srcX = p_nRegionX + kernelSize, dstX = p_nBufferX + kernelSize; srcX < regionXEnd - kernelSize; ++srcX, ++dstX)
			{
				// Get radiance context
				pRadianceContext = p_pRadianceBuffer->GetP(dstX, dstY);
				indirect = p_pRadianceBuffer->GetP(dstX & 0xFFFFFFF8, dstY & 0xFFFFFFF8)->Indirect;

				float weight = 0;
				Spectrum _indirect = 0;
				
				for (int ky = -kernelSize; ky < kernelSize; ++ky)
				{
					for (int kx = -kernelSize; kx < kernelSize; ++kx)
					{
						// compute edge details
						rc = p_pRadianceBuffer->GetP(dstX + kx, dstY + ky);
						float dist = Maths::Min(0.95f, Maths::Abs(pRadianceContext->Distance - rc->Distance));

						rc = p_pRadianceBuffer->GetP((dstX + kx) & 0xFFFFFFF8, (dstY + ky) & 0xFFFFFFF8);
						float cDist = Maths::Min(0.95f, 
							Maths::Sqr(pRadianceContext->Indirect[0] - rc->Indirect[0]) + 
							Maths::Sqr(pRadianceContext->Indirect[1] - rc->Indirect[1]) + 
							Maths::Sqr(pRadianceContext->Indirect[2] - rc->Indirect[2]));
						
						weight += (1-dist) * (1-cDist);
						_indirect = _indirect + rc->Indirect * (1-dist) * (1-cDist);

					}
				}

				pRadianceContext->Indirect = pRadianceContext->Albedo * _indirect / weight;
				pRadianceContext->Final = pRadianceContext->Indirect; // + pRadianceContext->Direct;
			}
		}
		/**/

		/*
		for (int srcY = p_nRegionY + kernelSize; srcY < regionYEnd - kernelSize; ++srcY)
		{
			for (int srcX = p_nRegionX + kernelSize; srcX < regionXEnd - kernelSize; ++srcX)
			{
				RadianceContext *r = m_pRadianceBuffer->GetP(srcX, srcY);
				float weight = 0;
				Spectrum L = 0.5f;

				for (int ky = -kernelSize; ky < kernelSize; ky++)
				{
					for (int kx = -kernelSize; kx < kernelSize; kx++)
					{
						// compute edge details
						pRadianceContext = p_pRadianceBuffer->GetP(srcX + kx, srcY + ky);
						float dist = Maths::Min(0.95f, Maths::Abs(pRadianceContext->Distance - r->Distance));

						pRadianceContext = p_pRadianceBuffer->GetP((srcX + kx) & 0xFFFFFFF8, 
							(srcY + ky) & 0xFFFFFFF8);
						float cDist = Maths::Min(0.95f, 
							Maths::Sqr(pRadianceContext->Indirect[0] - r->Indirect[0]) + 
							Maths::Sqr(pRadianceContext->Indirect[1] - r->Indirect[1]) + 
							Maths::Sqr(pRadianceContext->Indirect[2] - r->Indirect[2]));
						
						weight += (1-dist) * (1-cDist);
						L = L + r->Indirect * (1-dist) * (1-cDist);

						//pRadianceContext = p_pRadianceBuffer->GetP(srcX + kx, srcY + ky);
						//float weight2 = (pRadianceContext->Distance - r->Distance)
					}
				}
				r->Indirect = L / weight;
				r->Final = r->Direct + r->Indirect * r->Albedo;
			}
		}
		*/

		/*
		// Naive upscaling
		for (int srcY = p_nRegionY, dstY = p_nBufferY; srcY < regionYEnd; ++srcY, ++dstY)
		{
			for (int srcX = p_nRegionX, dstX = p_nBufferX; srcX < regionXEnd; ++srcX, ++dstX)
			{
				// Get radiance context
				pRadianceContext = p_pRadianceBuffer->GetP(dstX, dstY);
				if ((srcX & 0x07) | (srcY & 0x07))
				{
					indirect = p_pRadianceBuffer->GetP(dstX & 0xFFFFFFF8, dstY & 0xFFFFFFF8)->Indirect;
					pRadianceContext->Indirect = indirect;
					pRadianceContext->Final = indirect * pRadianceContext->Albedo + pRadianceContext->Direct;
				}

			}
		}
		/**/

		/* FOR SCIENCE!! */	
		Vector2 sample;

		// No supersampling
		context.SampleIndex = 0;

		int requiredSamples = p_nRegionWidth * p_nRegionHeight;
		int maxSamples = requiredSamples;

		float threshold = 0.1f;
		int thresholdArea = 0;

		// Rasterise pixels
		for (; requiredSamples > 0; requiredSamples--)
		{
			double currentTime = Platform::GetTime();
				
			if (Platform::ToSeconds(currentTime - startTime) > m_fRenderBudget)
				break;

			sample.X = p_nRegionWidth * QuasiRandomSequence::VanDerCorput(maxSamples - requiredSamples);
			sample.Y = p_nRegionHeight * QuasiRandomSequence::Sobol2(maxSamples - requiredSamples);

			int srcX = (int)(sample.X + p_nRegionX),
				srcY = (int)(sample.Y + p_nRegionY);

			int dstX = (int)(sample.X + p_nBufferX),
				dstY = (int)(sample.Y + p_nBufferY);

			// Get sub-pixel position
			sample = m_pScene->GetSampler()->Get2DSample();

			// Get radiance context
			pRadianceContext = p_pRadianceBuffer->GetP(dstX, dstY);

			// Set integrator context
			context.SurfacePosition.Set((float)srcX, (float)srcY);
			context.NormalisedPosition.Set(context.SurfacePosition.X * rcpWidth, context.SurfacePosition.Y * rcpHeight);

			// Get ray from camera
			m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, sample.U * rcpWidth, sample.V * rcpHeight, pRadianceContext->ViewRay);
				
			// Get radiance
			//pRadianceContext->Final = m_pIntegrator->Radiance(&context, m_pScene, pRadianceContext->ViewRay, intersection, pRadianceContext);
				// IIntegrator::Direct(&context, m_pScene, pRadianceContext->ViewRay, intersection, pRadianceContext);

			pRadianceContext->Final = IIntegrator::Direct(&context, m_pScene, pRadianceContext->ViewRay, intersection, pRadianceContext);			
			pRadianceContext->Flags |= RadianceContext::DF_Computed;

			if ((pRadianceContext->Final[0] + pRadianceContext->Final[1] + pRadianceContext->Final[2]) * 0.33f < threshold)
			{
				pRadianceContext->Flags |= RadianceContext::DF_MaskEnabled;
				thresholdArea++;
			}

			//pRadianceContext->Direct = 0;
			//pRadianceContext->Final = 0;
		}

		bool outOfTime = requiredSamples > 0;

		// Update remaining samples flag to show they were not processed
		for(; requiredSamples > 0; requiredSamples--)
		{
			sample.X = p_nRegionWidth * QuasiRandomSequence::VanDerCorput(maxSamples - requiredSamples) + p_nBufferX;
			sample.Y = p_nRegionHeight * QuasiRandomSequence::Sobol2(maxSamples - requiredSamples) + p_nBufferY;

			pRadianceContext = p_pRadianceBuffer->GetP((int)sample.X, (int)sample.Y);
			pRadianceContext->Flags = 0;
		}

		// If time budget has been exceded, return
		if (outOfTime) return;

		requiredSamples = maxSamples;

		// Rasterise indirect
		for (; requiredSamples > 0; requiredSamples--)
		{
			double currentTime = Platform::GetTime();
				
			if (Platform::ToSeconds(currentTime - startTime) > m_fRenderBudget)
				break;

			sample.X = p_nRegionWidth * QuasiRandomSequence::VanDerCorput(maxSamples - requiredSamples);
			sample.Y = p_nRegionHeight * QuasiRandomSequence::Sobol2(maxSamples - requiredSamples);

			int srcX = (int)(sample.X + p_nRegionX),
				srcY = (int)(sample.Y + p_nRegionY);

			int dstX = (int)(sample.X + p_nBufferX),
				dstY = (int)(sample.Y + p_nBufferY);

			// Get sub-pixel position
			sample = m_pScene->GetSampler()->Get2DSample();

			// Get radiance context
			pRadianceContext = p_pRadianceBuffer->GetP(dstX, dstY);

			// Set integrator context
			context.SurfacePosition.Set((float)srcX, (float)srcY);
			context.NormalisedPosition.Set(context.SurfacePosition.X * rcpWidth, context.SurfacePosition.Y * rcpHeight);

			// Get ray from camera
			m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, sample.U * rcpWidth, sample.V * rcpHeight, pRadianceContext->ViewRay);
				
			// Get radiance
			if (pRadianceContext->Flags & RadianceContext::DF_MaskEnabled)
			{
				pRadianceContext->Final = m_pIntegrator->Radiance(&context, m_pScene, pRadianceContext->ViewRay, intersection, pRadianceContext);
				pRadianceContext->Flags |= RadianceContext::DF_Computed;
			}
		}

		/* FOR SCIENCE!! */
		/*
		Vector2 sample;

		// No supersampling
		context.SampleIndex = 0;

		int requiredSamples = p_nRegionWidth * p_nRegionHeight;
		int maxSamples = requiredSamples;

		// Rasterise pixels
		for (; requiredSamples > 0; requiredSamples--)
		{
			double currentTime = Platform::GetTime();
				
			if (Platform::ToSeconds(currentTime - startTime) > m_fRenderBudget)
				break;

			sample.X = p_nRegionWidth * QuasiRandomSequence::VanDerCorput(maxSamples - requiredSamples);
			sample.Y = p_nRegionHeight * QuasiRandomSequence::Sobol2(maxSamples - requiredSamples);

			int srcX = (int)(sample.X + p_nRegionX),
				srcY = (int)(sample.Y + p_nRegionY);

			int dstX = (int)(sample.X + p_nBufferX),
				dstY = (int)(sample.Y + p_nBufferY);

			// Get sub-pixel position
			sample = m_pScene->GetSampler()->Get2DSample();

			// Get radiance context
			pRadianceContext = p_pRadianceBuffer->GetP(dstX, dstY);

			// Set integrator context
			context.SurfacePosition.Set((float)srcX, (float)srcY);
			context.NormalisedPosition.Set(context.SurfacePosition.X * rcpWidth, context.SurfacePosition.Y * rcpHeight);

			// Get ray from camera
			m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, sample.U * rcpWidth, sample.V * rcpHeight, pRadianceContext->ViewRay);
				
			// Get radiance
			pRadianceContext->Final = m_pIntegrator->Radiance(&context, m_pScene, pRadianceContext->ViewRay, intersection, pRadianceContext);
				// IIntegrator::Direct(&context, m_pScene, pRadianceContext->ViewRay, intersection, pRadianceContext);

			pRadianceContext->Flags |= RadianceContext::DF_Computed;
		}

		// Update remaining samples flag to show they were not processed
		while(requiredSamples-- >= 0)
		{
			sample.X = p_nRegionWidth * QuasiRandomSequence::VanDerCorput(maxSamples - requiredSamples) + p_nBufferX;
			sample.Y = p_nRegionHeight * QuasiRandomSequence::Sobol2(maxSamples - requiredSamples) + p_nBufferY;

			pRadianceContext = p_pRadianceBuffer->GetP((int)sample.X, (int)sample.Y);
			pRadianceContext->Flags &= ~RadianceContext::DF_Computed;
		}
		/**/
	}
}