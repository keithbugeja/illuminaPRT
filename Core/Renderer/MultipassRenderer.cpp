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
MultipassRenderer::MultipassRenderer(Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, int p_nSampleCount,
	int p_nDBSize, float p_fDBDist, float p_fDBCos)
	: IRenderer(p_pScene, p_pIntegrator, p_pDevice, p_pFilter)
	, m_nSampleCount(p_nSampleCount)
	, m_nDBSize(p_nDBSize)
	, m_fDBDist(p_fDBDist)
	, m_fDBCos(p_fDBCos)
{ }
//----------------------------------------------------------------------------------------------
MultipassRenderer::MultipassRenderer(const std::string &p_strName, Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, int p_nSampleCount,
	int p_nDBSize, float p_fDBDist, float p_fDBCos)
	: IRenderer(p_strName, p_pScene, p_pIntegrator, p_pDevice, p_pFilter)
	, m_nSampleCount(p_nSampleCount)
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

	m_pGeometryBuffer = new Intersection[m_nWidth * m_nHeight];

	return true;
}
//----------------------------------------------------------------------------------------------
bool MultipassRenderer::Shutdown(void)
{
	delete[] m_pGeometryBuffer;

	return true;
}
//----------------------------------------------------------------------------------------------
void MultipassRenderer::ComputeIntersectionPass(Intersection *p_pGeometryBuffer, int p_nTileX, int p_nTileY, int p_nTileWidth, int p_nTileHeight)
{
	double start = Platform::GetTime();

	IntegratorContext context;
	Intersection *pIntersection = p_pGeometryBuffer;
		
	// No supersampling
	context.SampleIndex = 0;

	float rcpWidth = 1.f / m_nWidth,
		rcpHeight = 1.f / m_nHeight;

	for (int y = p_nTileY; y < p_nTileHeight; ++y)
	{
		for (int x = p_nTileX; x < p_nTileWidth; ++x)
		{
			context.SurfacePosition.Set(x + 0.5f, y + 0.5f);
			context.NormalisedPosition.Set(context.SurfacePosition.X * rcpWidth, context.SurfacePosition.Y * rcpHeight);

			m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, 0.5f, 0.5f, pIntersection->EyeRay);
			pIntersection->Valid = m_pScene->Intersects(pIntersection->EyeRay, *pIntersection);

			// Move geometry buffer pointer
			++pIntersection;
		}
	}

	double end = Platform::GetTime();
	std::cout << "Intersection pass : " << Platform::ToSeconds(end - start) << "s" << std::endl;
}
//----------------------------------------------------------------------------------------------
void MultipassRenderer::ComputeShadingPass(Intersection *p_pGeometryBuffer, int p_nTileX, int p_nTileY, int p_nTileWidth, int p_nTileHeight)
{
	// Start shading
	double start = Platform::GetTime();

	Intersection *pIntersection = p_pGeometryBuffer;
	IntegratorContext context;
	Spectrum Li(0), Le(0);
		
	// No supersampling
	context.SampleIndex = 0;

	float rcpWidth = 1.f / m_nWidth,
		rcpHeight = 1.f / m_nHeight;

	for (int y = p_nTileY; y < p_nTileHeight; ++y)
	{
		for (int x = p_nTileX; x < p_nTileWidth; ++x)
		{
			context.SurfacePosition.Set(x + 0.5f, y + 0.5f);
			context.NormalisedPosition.Set(context.SurfacePosition.X * rcpWidth, context.SurfacePosition.Y * rcpHeight);

			Li = m_pIntegrator->Radiance(&context, m_pScene, *pIntersection);
			
			++pIntersection;
		}
	}

	double end = Platform::GetTime();
	std::cout << "Shading pass : " << Platform::ToSeconds(end - start) << "s" << std::endl;
	
	// Start post-processing
	start = Platform::GetTime();

	for (int y = p_nTileY + m_nDBSize; y < p_nTileHeight - m_nDBSize; ++y)
	{
		for (int x = p_nTileX + m_nDBSize; x < p_nTileWidth - m_nDBSize; ++x)
		{
			const int indexSrc = x + y * p_nTileWidth;
			const Vector3 &normal = p_pGeometryBuffer[indexSrc].Surface.ShadingBasisWS.W;
			const Vector3 &depth = p_pGeometryBuffer[indexSrc].Surface.PointWS;

			float contrib = 1; 
			Le = p_pGeometryBuffer[indexSrc].Indirect;

			for (int dy = -m_nDBSize; dy <= m_nDBSize; dy++)
			{
				for (int dx = -m_nDBSize; dx <= m_nDBSize; dx++)
				{
					// Central contribution already accumulated
					if (dx == 0 && dy == 0) continue;
					
					// Get index of contributor
					const int index = (x + dx) + (y + dy) * p_nTileWidth;

					// If a valid intersection, compute
					if (!p_pGeometryBuffer[index].Valid) continue;

					if (
						(Vector3::DistanceSquared(depth, p_pGeometryBuffer[index].Surface.PointWS) < m_fDBDist) &&
						(Vector3::Dot(normal, p_pGeometryBuffer[index].Surface.ShadingBasisWS.W) > m_fDBCos)
					)
					{
						Le += p_pGeometryBuffer[index].Indirect; 
						contrib++;						
					}
				}
			}

			Li = p_pGeometryBuffer[indexSrc].Direct + 
				(Le * p_pGeometryBuffer[indexSrc].Reflectance) / contrib;

			m_pDevice->Set(m_nWidth - (x + 1), m_nHeight - (y + 1), Li);
		}
	}

	end = Platform::GetTime();
	std::cout << "Discontinuity pass : " << Platform::ToSeconds(end - start) << "s" << std::endl;
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
	
	//boost::progress_display renderProgress(height);
	//m_pIntegrator->Prepare(m_pScene);

	ComputeIntersectionPass(m_pGeometryBuffer, 0, 0, width, height);
	ComputeShadingPass(m_pGeometryBuffer, 0, 0, width, height);
	
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

	if(!updateIO) { std::cout << "Persisting frame..." << std::endl; m_pDevice->EndFrame(); }
}
//----------------------------------------------------------------------------------------------