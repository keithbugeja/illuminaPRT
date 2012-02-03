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
MultipassRenderer::MultipassRenderer(Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, int p_nSampleCount)
	: IRenderer(p_pScene, p_pIntegrator, p_pDevice, p_pFilter)
	, m_nSampleCount(p_nSampleCount)
{ }
//----------------------------------------------------------------------------------------------
MultipassRenderer::MultipassRenderer(const std::string &p_strName, Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, int p_nSampleCount)
	: IRenderer(p_strName, p_pScene, p_pIntegrator, p_pDevice, p_pFilter)
	, m_nSampleCount(p_nSampleCount)
{ }
//----------------------------------------------------------------------------------------------
void MultipassRenderer::ComputeIntersectionPass(Intersection *p_pGeometryBuffer, int p_nTileX, int p_nTileY, int p_nTileWidth, int p_nTileHeight)
{
	IntegratorContext context;
	Intersection *pIntersection = p_pGeometryBuffer;
		
	int height = m_pDevice->GetHeight(),
		width = m_pDevice->GetWidth();

	for (int y = p_nTileY; y < p_nTileHeight; ++y)
	{
		for (int x = p_nTileX; x < p_nTileWidth; ++x)
		{
			context.SampleIndex = 0;
			context.SurfacePosition.Set(x + 0.5f, y + 0.5f);
			context.NormalisedPosition.Set(context.SurfacePosition.X / width, context.SurfacePosition.Y / height);

			pIntersection->EyeRay = m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, 0.5f, 0.5f);
			pIntersection->Valid = m_pScene->Intersects(pIntersection->EyeRay, *pIntersection);

			// Move geometry buffer pointer
			++pIntersection;
		}
	}
}
//----------------------------------------------------------------------------------------------
void MultipassRenderer::ComputeShadingPass(Intersection *p_pGeometryBuffer, int p_nTileX, int p_nTileY, int p_nTileWidth, int p_nTileHeight)
{
	Intersection *pIntersection = p_pGeometryBuffer;
	IntegratorContext context;

	Spectrum Li(0), Le(0);

	int height = m_pDevice->GetHeight(),
		width = m_pDevice->GetWidth();

	for (int y = p_nTileY; y < p_nTileHeight; ++y)
	{
		for (int x = p_nTileX; x < p_nTileWidth; ++x)
		{
			context.SampleIndex = 0;
			context.SurfacePosition.Set(x + 0.5f, y + 0.5f);
			context.NormalisedPosition.Set(context.SurfacePosition.X / width, context.SurfacePosition.Y / height);

			Li = m_pIntegrator->Radiance(&context, m_pScene, *pIntersection);
			
			// m_pDevice->Set(width - (x + 1), height - (y + 1), Li);

			// Move geometry buffer pointer
			++pIntersection;
		}
	}

	int size = 2;

	for (int y = p_nTileY + size; y < p_nTileHeight - size; ++y)
	{
		for (int x = p_nTileX + size; x < p_nTileWidth - size; ++x)
		{
			const int indexSrc = x + y * p_nTileWidth;
			const Vector3 &normal = p_pGeometryBuffer[indexSrc].Surface.ShadingBasisWS.W;
			const Vector3 &depth = p_pGeometryBuffer[indexSrc].Surface.PointWS;

			float contrib = 0; Le = 0;

			for (int dy = -size; dy <= size; dy++)
			{
				for (int dx = -size; dx <= size; dx++)
				{
					const int index = (x + dx) + (y + dy) * p_nTileWidth;
					if (!p_pGeometryBuffer[index].Valid) continue;

					if (/*(Vector3::DistanceSquared(depth, p_pGeometryBuffer[index].Surface.PointWS) < 1.f) &&*/
						(Vector3::Dot(normal, p_pGeometryBuffer[index].Surface.ShadingBasisWS.W) > 0.75f))
					{
						Le += p_pGeometryBuffer[index].Indirect;
						contrib += Vector3::Dot(normal, p_pGeometryBuffer[index].Surface.ShadingBasisWS.W);
					}
				}
			}

			if (contrib <= 1)
			{
				Li = p_pGeometryBuffer[indexSrc].Direct + p_pGeometryBuffer[indexSrc].Indirect; // * p_pGeometryBuffer[indexSrc].Reflectance;
			}
			else
			{
				Li = p_pGeometryBuffer[indexSrc].Direct + Le / contrib;//(Le /* * p_pGeometryBuffer[indexSrc].Reflectance*/) / contrib;
			}

			/*
			for (int y = rc->sy; y < rc->ey; y++)
			{
				for (int x = rc->sx; x < rc->ex; x++)
				{
					const int index = y * width + x;
					Vec3 curNorm = dsc->hc[index].sNormal,
						 curDepth = dsc->hc[index].hitPoint;
					float totalWeight = 0.0f;

					// Zero buffer
					buffer[index].Set(0.0f, 0.0f, 0.0f);

					for (int j = -size; j <= size; j++)
					{
						for (int i = -size; i <= size; i++)
						{
							const int indexbox = ((y + j) * width) + (x + i);
							if (dsc->hc[index].bsdf == NULL) 
								continue;

							if ((curDepth - dsc->hc[indexbox].hitPoint).MagnitudeSquare() < 1.0f &&
								(curNorm.Dot(dsc->hc[indexbox].sNormal) > 0.75))// &&
								//dsc->hc[index].bsdf->light == dsc->hc[indexbox].bsdf->light)) 
							{
								buffer[index] += b[indexbox];
								totalWeight += 1.0f;
							}
						}
					}

					if (totalWeight != 0.0f)
						buffer[index] /= totalWeight;
					else
					   buffer[index] = b[index];
				}
			}
			*/

			m_pDevice->Set(width - (x + 1), height - (y + 1), Li);
		}
	}
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

	Intersection *pGeometryBuffer = new Intersection[width * height];

	if (!updateIO) m_pDevice->BeginFrame();
	
	boost::progress_display renderProgress(height);

	m_pIntegrator->Prepare(m_pScene);

	ComputeIntersectionPass(pGeometryBuffer, 0, 0, width, height);
	ComputeShadingPass(pGeometryBuffer, 0, 0, width, height);
	
	/*
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
	*/

	delete[] pGeometryBuffer;

	if(!updateIO) { std::cout << "Persisting frame..." << std::endl; m_pDevice->EndFrame(); }
}
//----------------------------------------------------------------------------------------------