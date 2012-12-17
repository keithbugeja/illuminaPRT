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
	}
	else
	{
		// Set kernel, half-kernel and indirect half-kernel
		const int kernelSize = 8; //6;
		const int halfKernelSize = 4; //3;
		const int indirectHalfKernelSize = 1;
		const int indirectFrequency = 0x04; //0x03; // /**/ 0x03; /* 0x01; /**/
		const int indirectFrequencySpread = indirectFrequency; //(indirectFrequency * Maths::Pow(2, maxlevels));

		// Compute tile bounds
		int regionXEnd = p_nRegionX + p_nRegionWidth,
			regionYEnd = p_nRegionY + p_nRegionHeight;

		Vector2 sample = 
			m_pScene->GetSampler()->Get2DSample();

		// No supersampling
		context.SampleIndex = 0;

		if (m_pRadianceBuffer != NULL &&
			(m_pRadianceBuffer->GetWidth() < p_pRadianceBuffer->GetWidth() || 
			 m_pRadianceBuffer->GetHeight() < p_pRadianceBuffer->GetHeight()))
			delete m_pRadianceBuffer;

		if (m_pRadianceBuffer == NULL)
			m_pRadianceBuffer = new RadianceBuffer(p_pRadianceBuffer->GetWidth(), p_pRadianceBuffer->GetHeight());

		// Render ld tile
		for (int srcY = p_nRegionY, dstY = 0; srcY <= regionYEnd + indirectFrequency; srcY += indirectFrequency, dstY++)
		{
			for (int srcX = p_nRegionX, dstX = 0; srcX <= regionXEnd + indirectFrequency; srcX += indirectFrequency, dstX++)
			{
				// pRadianceContext = p_pRadianceBuffer->GetP(dstX, dstY); 
				pRadianceContext = m_pRadianceBuffer->GetP(dstX, dstY);

				// Set integrator context
				context.SurfacePosition.Set((float)srcX, (float)srcY);
				context.NormalisedPosition.Set(context.SurfacePosition.X * rcpWidth, context.SurfacePosition.Y * rcpHeight);

				// Get ray from camera
				m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, sample.U * rcpWidth, sample.V * rcpHeight, pRadianceContext->ViewRay);

				// Compute Indirect
				// context.SurfacePosition.X = (int)(srcX * QuasiRandomSequence::VanDerCorput(context.SurfacePosition.X));
				// context.SurfacePosition.Y = (int)(srcY * QuasiRandomSequence::Sobol2(context.SurfacePosition.Y));
				// std::cout << "[" << context.SurfacePosition.X << ", " << context.SurfacePosition.Y << "]" << std::endl;

				// Compute Radiance
				context.SurfacePosition.Set(srcX / indirectFrequency, srcY / indirectFrequency);
				pRadianceContext->Final = m_pIntegrator->Radiance(&context, m_pScene, pRadianceContext->ViewRay, intersection, pRadianceContext);
			}
		}

		/* 
		Spectrum Li;
		int irradianceSamples;
		int ys, ye, xs, xe;
		float angle = 0.7;

		RadianceContext *pKernelContext,
			*pNeighbourContext,
			*pOutputContext;
		
		int regionHeight = p_nRegionHeight / indirectFrequency, //>> indirectFrequencyScale,
			regionWidth = p_nRegionWidth / indirectFrequency; //>> indirectFrequencyScale;

		// Discontinuity buffer
		RadianceContext *pIndirectContext;
		Spectrum intensity, indirect;

		float weight, 
			gaussWeight,
			intensityWeight, 
			distanceWeight;

		float sigmaS = 12.0f,
			sigmaR = 2.0f,
			sigmaG = 6.0f;

		float mean_s_PDF = Statistics::GaussianPDF(0, 0, sigmaS),
			edge_s_PDF = Statistics::GaussianPDF(sigmaS, 0, sigmaS),
			interval_s_PDF = mean_s_PDF - edge_s_PDF,
			invSigmaS = 1.f / sigmaS;

		float mean_r_PDF = Statistics::GaussianPDF(0, 0, sigmaR),
			edge_r_PDF = Statistics::GaussianPDF(sigmaR, 0, sigmaR),
			interval_r_PDF = mean_r_PDF - edge_r_PDF,
			invSigmaR = 1.f / sigmaR;

		float mean_g_PDF = Statistics::GaussianPDF(0, 0, sigmaG),
			  edge_g_PDF = Statistics::GaussianPDF(sigmaG, 0, sigmaG),
				interval_g_PDF = mean_g_PDF - edge_g_PDF,
				invSigmaG = 1.f / sigmaG;
		/*

		for (int y = 0; y < regionHeight; y++)
		{
			ys = Maths::Max(0, y - indirectHalfKernelSize);
			ye = Maths::Min(regionHeight, y + indirectHalfKernelSize);

			for (int x = 0; x < regionWidth; x++)
			{
				xs = Maths::Max(0, x - indirectHalfKernelSize);
				xe = Maths::Min(regionHeight, x + indirectHalfKernelSize);

				pKernelContext = m_pRadianceBuffer->GetP(x, y);
				pOutputContext = p_pRadianceBuffer->GetP(x, y);
				indirect = 0; weight = 0;

				for (int dy = ys; dy < ye; dy++)
				{
					for (int dx = xs; dx < xe; dx++)
					{
						pNeighbourContext = m_pRadianceBuffer->GetP(dx, dy);

						if (Vector3::Dot(pKernelContext->Normal, pNeighbourContext->Normal) > angle)
						{
							gaussWeight = Maths::Sqrt(Maths::Sqr(dx - x) + Maths::Sqr(dy - y));
							gaussWeight = gaussWeight > sigmaG ? 0.f : interval_g_PDF * (gaussWeight * invSigmaR) + mean_g_PDF;

							distanceWeight = pKernelContext->Distance - pNeighbourContext->Distance;
							distanceWeight *= distanceWeight;
							distanceWeight = distanceWeight > sigmaS ? 0.f : interval_s_PDF * (distanceWeight * invSigmaS) + mean_s_PDF;

							indirect += pNeighbourContext->Indirect * gaussWeight * distanceWeight;
							weight += gaussWeight * distanceWeight;
						}
					}
				}

				if (weight > Maths::Epsilon)
					pOutputContext->Indirect = indirect / weight;
			}
		}
		/* */

		/*
		// Low-pass filter on rows
		float alpha = 0.1f;
		const int maxlevels = 2;

		//for (int levels = 1; levels <= maxlevels; levels++)
		{
			for (int x = 0; x < regionWidth; x++)
			{
				for (int y = 1; y < regionHeight; y++)
				{
					m_pRadianceBuffer->GetP(x, y)->Indirect = (m_pRadianceBuffer->GetP(x, y)->Indirect * alpha) + (m_pRadianceBuffer->GetP(x, y - 1)->Indirect * (1.f - alpha));
				}
			}

			for (int y = 0; y < regionHeight; y++)
			{
				for (int x = 1; x < regionWidth; x++)
				{
					m_pRadianceBuffer->GetP(x, y)->Indirect = (m_pRadianceBuffer->GetP(x, y)->Indirect * alpha) + (m_pRadianceBuffer->GetP(x - 1, y)->Indirect * (1.f - alpha));
				}
			}
		}

		*/

		/*
		for (int y = 0; y < regionHeight; y++)
		{
			ys = Maths::Max(0, y - indirectHalfKernelSize);
			ye = Maths::Min(regionHeight, y + indirectHalfKernelSize);

			for (int x = 0; x < regionWidth; x++)
			{
				xs = Maths::Max(0, x - indirectHalfKernelSize);
				xe = Maths::Min(regionHeight, x + indirectHalfKernelSize);

				pKernelContext = p_pRadianceBuffer->GetP(x, y);
				pOutputContext = m_pRadianceBuffer->GetP(x, y);
				indirect = 0; weight = 0;

				for (int dy = ys; dy < ye; dy++)
				{
					for (int dx = xs; dx < xe; dx++)
					{
						pNeighbourContext = p_pRadianceBuffer->GetP(dx, dy);

						if (Vector3::Dot(pKernelContext->Normal, pNeighbourContext->Normal) > angle)
						{
							gaussWeight = Maths::Sqrt(Maths::Sqr(dx - x) + Maths::Sqr(dy - y));
							gaussWeight = gaussWeight > sigmaG ? 0.f : interval_g_PDF * (gaussWeight * invSigmaR) + mean_g_PDF;

							distanceWeight = pKernelContext->Distance - pNeighbourContext->Distance;
							distanceWeight *= distanceWeight;
							distanceWeight = distanceWeight > sigmaS ? 0.f : interval_s_PDF * (distanceWeight * invSigmaS) + mean_s_PDF;

							indirect += pNeighbourContext->Indirect * gaussWeight * distanceWeight;
							weight += gaussWeight * distanceWeight;
						}
					}
				}

				if (weight > Maths::Epsilon)
					pOutputContext->Indirect = indirect / weight;
			}
		}
		*/ 

		/*
		for (int y = 0; y < regionHeight; y++)
		{
			for (int x = 0; x < regionWidth; x++)
			{
				pOutputContext = m_pRadianceBuffer->GetP(x, y);
				pKernelContext = p_pRadianceBuffer->GetP(x, y);

				pOutputContext->Indirect = pKernelContext->Indirect;
			}
		}
		/* */

		// Direct
		/* */
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
				pixelX = iX / indirectFrequencySpread;
				pixelY = iY / indirectFrequencySpread;

				// If we haven't drawn this
				if (iX % indirectFrequencySpread != 0 || iY % indirectFrequencySpread != 0)
				{
					// Compute direct lighting
					IIntegrator::Direct(&context, m_pScene, pRadianceContext->ViewRay, intersection, pRadianceContext);
				
					// Bilinear interpolation of indirect lighting (upscale)
					t0 = 1.f - Maths::Frac((float)iX / indirectFrequencySpread);
					t1 = 1.f - Maths::Frac((float)iY / indirectFrequencySpread);

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
				
				// Move to adjacent radiance texel
				pRadianceContext++;
			}
		}

		/* */

		/*
		{
			for (int rounds = 0; rounds < 1; rounds++)
			{
				//----------------------------------------------------------------------------------------------
				for (int y = wideKernelSize; y <= p_nRegionHeight - wideKernelSize; y++)
				{
					ys = y - wideKernelSize;
					ye = y + wideKernelSize;

					for (int x = wideKernelSize; x <= p_nRegionWidth - wideKernelSize; x++)
					{
						xs = x - wideKernelSize;
						xe = x + wideKernelSize;

						weight = 0; intensity = 0;

						pKernelContext = p_pRadianceBuffer->GetP(x, y);
						pOutputContext = p_pRadianceBuffer->GetP(x, y);

						Spectrum indirect = m_pRadianceBuffer->GetP(x >> 2, y >> 2)->Indirect;

						for (int dy = ys; dy <= ye; dy++)
						{
							pNeighbourContext = p_pRadianceBuffer->GetP(xs, dy);

							for (int dx = xs; dx <= xe; dx++)
							{
								pIndirectContext = m_pRadianceBuffer->GetP(xs >> 2, ys >> 2);
								distanceWeight = pKernelContext->Distance - pNeighbourContext->Distance;
								
								// Gaussian distance
								//distanceWeight = Statistics::GaussianPDF(distanceWeight * distanceWeight);

								// Gaussian approximation
								distanceWeight *= distanceWeight;
								distanceWeight = distanceWeight > sigmaS ? 0.f : interval_s_PDF * (distanceWeight * invSigmaS) + mean_s_PDF;

								// Gaussian colour
								intensityWeight = Maths::Sqr(dx - x) + Maths::Sqr(dy - y);
								intensityWeight = intensityWeight > sigmaR ? 0.f : interval_r_PDF * (intensityWeight * invSigmaR) + mean_r_PDF;
								
								//intensityWeight = Maths::Sqr(indirect[0] - pNeighbourContext->Indirect[0]) +
								//	Maths::Sqr(indirect[1] - pNeighbourContext->Indirect[1]) +
								//	Maths::Sqr(indirect[2] - pNeighbourContext->Indirect[2]);

								//intensityWeight = intensityWeight > sigmaR ? 0.f : interval_r_PDF * (intensityWeight * invSigmaR) + mean_r_PDF;
								
								weight++;
								intensity += pIndirectContext->Indirect;
								//weight += distanceWeight * intensityWeight;
								//weight += intensityWeight; //distanceWeight;
								//intensity += pIndirectContext->Indirect * distanceWeight * intensityWeight;
								
								//weight += distanceWeight;
								//intensity += pIndirectContext->Indirect * distanceWeight;

								pNeighbourContext++;
							}
						}

						if (weight > Maths::Epsilon)
						{
							pOutputContext->Indirect = intensity / weight;
							pOutputContext->Final = pKernelContext->Direct + pOutputContext->Indirect * pKernelContext->Albedo;
						}
						
						pOutputContext->Indirect = indirect;
						pOutputContext->Final = pOutputContext->Indirect * pKernelContext->Albedo;
					}
				}
			}
		}
		*/

		// Scale indirect
		/*
		// Render quarter tile
		for (int srcY = p_nRegionY, dstY = p_nBufferY; srcY < regionYEnd; srcY += indirectFrequencyIncr, dstY += indirectFrequencyIncr)
		{
			for (int srcX = p_nRegionX, dstX = p_nBufferX; srcX < regionXEnd; srcX += indirectFrequencyIncr, dstX += indirectFrequencyIncr)
			{
				pRadianceContext = p_pRadianceBuffer->GetP(dstX, dstY);

				// Set integrator context
				context.SurfacePosition.Set((float)srcX, (float)srcY);
				context.NormalisedPosition.Set(context.SurfacePosition.X * rcpWidth, context.SurfacePosition.Y * rcpHeight);

				// Get ray from camera
				m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, sample.U * rcpWidth, sample.V * rcpHeight, pRadianceContext->ViewRay);

				// Compute Indirect
				pRadianceContext->Final = m_pIntegrator->Radiance(&context, m_pScene, pRadianceContext->ViewRay, intersection, pRadianceContext);
			}
		} 
		*/

		/*
		Spectrum Li;
		int irradianceSamples;
		int kernelSize = 2 * indirectFrequencyIncr;
		int ys, ye, xs, xe;
		float angle = 0.7;

		RadianceContext *pKernelContext,
			*pNeighbourContext,
			*pOutputContext;
		
		int regionHeight = p_nRegionHeight,
			regionWidth = p_nRegionWidth;

		for (int y = p_nBufferY + kernelSize; y < p_nBufferY + (regionHeight / indirectFrequencyIncr * indirectFrequencyIncr) - kernelSize; y+=indirectFrequencyIncr)
		{
			ys = y - kernelSize;
			ye = y + kernelSize;

			for (int x = p_nBufferX + kernelSize; x < p_nBufferX + (regionWidth / indirectFrequencyIncr * indirectFrequencyIncr) - kernelSize; x+=indirectFrequencyIncr)
			{
				xs = x - kernelSize;
				xe = x + kernelSize;

				irradianceSamples = 0;
				Li = 0.f;

				pKernelContext = pOutputContext = p_pRadianceBuffer->GetP(x, y);

				for (int dy = ys; dy < ye; dy+=indirectFrequencyIncr)
				{
					pNeighbourContext = p_pRadianceBuffer->GetP(xs, dy);

					for (int dx = xs; dx < xe; dx+=indirectFrequencyIncr)
					{
						if (Vector3::Dot(pKernelContext->Normal, pNeighbourContext->Normal) > angle)
						{
							Li += pNeighbourContext->Indirect;
							irradianceSamples++;
						}

						pNeighbourContext+=indirectFrequencyIncr;
					}
				}
			
				// Compute final colour
				if (irradianceSamples) {
					pOutputContext->Indirect = (Li / irradianceSamples);
					// pOutputContext->Final = pKernelContext->Direct + (Li * pKernelContext->Albedo) / irradianceSamples;
					pOutputContext->Flags |= RadianceContext::DF_Processed;
				} 
			}
		}
		*/

		/*
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
			}
		}
		*/

		/*
		// Render quarter tile
		for (int srcY = p_nRegionY, dstY = p_nBufferY; srcY < regionYEnd; srcY += indirectFrequencyIncr, dstY++)
		{
			for (int srcX = p_nRegionX, dstX = p_nBufferX; srcX < regionXEnd; srcX += indirectFrequencyIncr, dstX++)
			{
				pRadianceContext = p_pRadianceBuffer->GetP(dstX, dstY);

				// Set integrator context
				context.SurfacePosition.Set((float)srcX, (float)srcY);
				context.NormalisedPosition.Set(context.SurfacePosition.X * rcpWidth, context.SurfacePosition.Y * rcpHeight);

				// Get ray from camera
				m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, sample.U * rcpWidth, sample.V * rcpHeight, pRadianceContext->ViewRay);

				// Compute Indirect
				pRadianceContext->Final = m_pIntegrator->Radiance(&context, m_pScene, pRadianceContext->ViewRay, intersection, pRadianceContext);
			}
		} 
		/* */

		/*
		// Render quarter tile
		for (int srcY = p_nRegionY, dstY = p_nBufferY; srcY < regionYEnd; srcY += indirectFrequencyIncr, dstY += indirectFrequencyIncr)
		{
			for (int srcX = p_nRegionX, dstX = p_nBufferX; srcX < regionXEnd; srcX += indirectFrequencyIncr, dstX += indirectFrequencyIncr)
			{
				pRadianceContext = p_pRadianceBuffer->GetP(dstX, dstY);

				// Set integrator context
				context.SurfacePosition.Set((float)srcX, (float)srcY);
				context.NormalisedPosition.Set(context.SurfacePosition.X * rcpWidth, context.SurfacePosition.Y * rcpHeight);

				// Get ray from camera
				m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, sample.U * rcpWidth, sample.V * rcpHeight, pRadianceContext->ViewRay);

				// Compute Indirect
				pRadianceContext->Final = m_pIntegrator->Radiance(&context, m_pScene, pRadianceContext->ViewRay, intersection, pRadianceContext);
			}
		} 
		/* */

		/*
		Spectrum Li;
		int irradianceSamples;
		int kernelSize = 2 * indirectFrequencyIncr;
		int ys, ye, xs, xe;
		float angle = 0.7;

		RadianceContext *pKernelContext,
			*pNeighbourContext,
			*pOutputContext;
		
		int regionHeight = p_nRegionHeight,
			regionWidth = p_nRegionWidth;

		for (int y = p_nBufferY + kernelSize; y < p_nBufferY + (regionHeight / indirectFrequencyIncr * indirectFrequencyIncr) - kernelSize; y+=indirectFrequencyIncr)
		{
			ys = y - kernelSize;
			ye = y + kernelSize;

			for (int x = p_nBufferX + kernelSize; x < p_nBufferX + (regionWidth / indirectFrequencyIncr * indirectFrequencyIncr) - kernelSize; x+=indirectFrequencyIncr)
			{
				xs = x - kernelSize;
				xe = x + kernelSize;

				irradianceSamples = 0;
				Li = 0.f;

				pKernelContext = pOutputContext = p_pRadianceBuffer->GetP(x, y);

				for (int dy = ys; dy < ye; dy+=indirectFrequencyIncr)
				{
					pNeighbourContext = p_pRadianceBuffer->GetP(xs, dy);

					for (int dx = xs; dx < xe; dx+=indirectFrequencyIncr)
					{
						if (Vector3::Dot(pKernelContext->Normal, pNeighbourContext->Normal) > angle)
						{
							Li += pNeighbourContext->Indirect;
							irradianceSamples++;
						}

						pNeighbourContext+=indirectFrequencyIncr;
					}
				}
			
				// Compute final colour
				if (irradianceSamples) {
					pOutputContext->Indirect = (Li / irradianceSamples);
					// pOutputContext->Final = pKernelContext->Direct + (Li * pKernelContext->Albedo) / irradianceSamples;
					pOutputContext->Flags |= RadianceContext::DF_Processed;
				} 
			}
		}
		/* */

		/* 
		int regionHeight = p_nRegionHeight / indirectFrequencyIncr,
			regionWidth = p_nRegionWidth / indirectFrequencyIncr;

		for (int y = p_nBufferY + kernelSize; y < p_nBufferY + regionHeight - kernelSize; y++)
		{
			ys = y - kernelSize;
			ye = y + kernelSize;

			for (int x = p_nBufferX + kernelSize; x < p_nBufferX + regionWidth - kernelSize; x++)
			{
				xs = x - kernelSize;
				xe = x + kernelSize;

				irradianceSamples = 0;
				Li = 0.f;

				pKernelContext = pOutputContext = p_pRadianceBuffer->GetP(x, y);

				for (int dy = ys; dy < ye; dy++)
				{
					pNeighbourContext = p_pRadianceBuffer->GetP(xs, dy);

					for (int dx = xs; dx < xe; dx++)
					{
						if (Vector3::Dot(pKernelContext->Normal, pNeighbourContext->Normal) > angle)
						{
							Li += pNeighbourContext->Indirect;
							irradianceSamples++;
						}

						pNeighbourContext++;
					}
				}
			
				// Compute final colour
				if (irradianceSamples) {
					pOutputContext->Final = pKernelContext->Direct + (Li * pKernelContext->Albedo) / irradianceSamples;
					pOutputContext->Flags |= RadianceContext::DF_Processed;
				} 
			}
		}
		*/

		/*
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
			}
		}
		*/

		/*
		Spectrum Li;
		int irradianceSamples;
		int m_nKernelSize = indirectFrequencyIncr;
		int ys, ye, xs, xe;
		float m_fAngle = 0.7;
		
		RadianceContext *pKernelContext,
					*pNeighbourContext,
					*pOutputContext;

		for (int y = p_nRegionY + m_nKernelSize; y < p_nRegionHeight - ((p_nRegionHeight % indirectFrequencyIncr) + m_nKernelSize); y += indirectFrequencyIncr)
		{
			ys = y - m_nKernelSize;
			ye = y + m_nKernelSize;

			for (int x = p_nRegionX + m_nKernelSize; x < p_nRegionWidth - ((p_nRegionWidth % indirectFrequencyIncr) + m_nKernelSize); x += indirectFrequencyIncr)
			{
				xs = x - m_nKernelSize;
				xe = x + m_nKernelSize;

				irradianceSamples = 0;
				Li = 0.f;

				pKernelContext = pOutputContext = p_pRadianceBuffer->GetP(x, y);

				for (int dy = ys; dy < ye; dy += indirectFrequencyIncr)
				{
					pNeighbourContext = p_pRadianceBuffer->GetP(xs, dy);

					for (int dx = xs; dx < xe; dx += indirectFrequencyIncr)
					{
						if (Vector3::Dot(pKernelContext->Normal, pNeighbourContext->Normal) > m_fAngle)
						{
							Li += pNeighbourContext->Indirect;
							irradianceSamples++;
						}

						pNeighbourContext += indirectFrequencyIncr;
					}
				}
			
				// Compute final colour
				if (irradianceSamples) {
					pOutputContext->Final = pKernelContext->Direct + (Li * pKernelContext->Albedo) / irradianceSamples;
					pOutputContext->Flags |= RadianceContext::DF_Processed;
				}
			}
		}
		*/

		/*
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
			}
		}
		/**/
	}
}