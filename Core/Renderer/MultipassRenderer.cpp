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
void MultipassRenderer::RenderRegion(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight)
{
	BOOST_ASSERT(m_pScene != NULL && m_pIntegrator != NULL && m_pFilter != NULL && 
		m_pScene->GetCamera() != NULL && m_pScene->GetSpace() != NULL && m_pScene->GetSampler() != NULL);

	ComputeCombinedPass(m_pGeometryBuffer, p_nRegionX, p_nRegionY, p_nRegionWidth, p_nRegionHeight);
}
//----------------------------------------------------------------------------------------------
void MultipassRenderer::ComputeDirectPass(IntegratorContext *p_pContext, Scene *p_pScene, Intersection &p_intersection)
{
	IMaterial *pMaterial = NULL;

	Vector3 wIn, wOut,
		wInLocal, wOutLocal; 

	if (p_intersection.Valid)
	{
		if (p_intersection.HasMaterial()) 
		{
			// Get material for intersection primitive
			pMaterial = p_intersection.GetMaterial();

			wOut = -Vector3::Normalize(p_intersection.EyeRay.Direction);

			if (!p_intersection.IsEmissive())
			{
				// Sample direct lighting
				p_intersection.Direct = IIntegrator::SampleAllLights(p_pScene, p_intersection, 
					p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, 
					wOut, p_pScene->GetSampler(), p_intersection.GetLight(), 1);

				p_intersection.Reflectance = pMaterial->Rho(wOut, p_intersection.Surface);
				p_intersection.Indirect = 0;
			}
			else
			{
				p_intersection.Direct = p_intersection.GetLight()->Radiance(p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, wOut);
				p_intersection.Indirect = 0.f;
			}
		}
	}
	else
	{
		for (size_t lightIndex = 0; lightIndex < p_pScene->LightList.Size(); ++lightIndex)
			p_intersection.Direct = p_pScene->LightList[lightIndex]->Radiance(-p_intersection.EyeRay);
	}
}
//----------------------------------------------------------------------------------------------
void MultipassRenderer::ComputeCombinedPass(Intersection *p_pGeometryBuffer, int p_nTileX, int p_nTileY, int p_nTileWidth, int p_nTileHeight)
{
	// double start = Platform::GetTime();

	Intersection *pIntersection;
	IntegratorContext context;

	// No supersampling
	context.SampleIndex = 0;

	// Compute tile bounds
	int endTileX = p_nTileX + p_nTileWidth;
	int endTileY = p_nTileY + p_nTileHeight;
		
	// Compute dimension reciprocals for normalisation
	float rcpWidth = 1.f / m_nWidth,
		rcpHeight = 1.f / m_nHeight;

	for (int y = p_nTileY; y < endTileY; ++y)
	{
		const int line = y * m_nWidth;

		for (int x = p_nTileX; x < endTileX; ++x)
		{
			m_pScene->GetSampler()->Reset();

			pIntersection = p_pGeometryBuffer + line + x;
			pIntersection->Final = 0.f;

			for (int sampleIndex = 0; sampleIndex < m_nSampleCount; sampleIndex++)
			{
				Vector2 sample = m_pScene->GetSampler()->Get2DSample();

				context.SurfacePosition.Set(x + sample.X, y + sample.Y);
				context.NormalisedPosition.Set(context.SurfacePosition.X * rcpWidth, context.SurfacePosition.Y * rcpHeight);

				m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, 0.5f * rcpWidth, 0.5f * rcpHeight, pIntersection->EyeRay);
				pIntersection->Final += m_pIntegrator->Radiance(&context, m_pScene, pIntersection->EyeRay, *pIntersection);
			}

			// m_pDevice->Set(m_nWidth - (x + 1), m_nHeight - (y + 1), pIntersection->Final / m_nSampleCount);
		}
	}

	// double end = Platform::GetTime();
	// std::cout << "Intersection pass : " << Platform::ToSeconds(end - start) << "s" << std::endl;

	Spectrum Le;

	for (int y = p_nTileY; y < endTileY; ++y)
	{
		for (int x = p_nTileX; x < endTileX; ++x)
		{
			const int indexSrc = x + y * m_nWidth;
			const Vector3 &normal = p_pGeometryBuffer[indexSrc].Surface.ShadingBasisWS.W;
			const Vector3 &depth = p_pGeometryBuffer[indexSrc].Surface.PointWS;

			float contrib = 1;
			Le = 0.f;

			//float contrib = 1; 
			//Le = p_pGeometryBuffer[indexSrc].Indirect;

			for (int dy = Maths::Max(y - m_nDBSize, p_nTileY); dy < Maths::Min(y + m_nDBSize, endTileY); dy++)
			{
				for (int dx = Maths::Max(x - m_nDBSize, p_nTileX); dx < Maths::Min(x + m_nDBSize, endTileX); dx++)
				{
					const int index = dx + dy * m_nWidth;

					//if (p_pGeometryBuffer[index].Valid)
					{
						if ((Vector3::DistanceSquared(depth, p_pGeometryBuffer[index].Surface.PointWS) < m_fDBDist) &&
							(Vector3::Dot(normal, p_pGeometryBuffer[index].Surface.ShadingBasisWS.W) > m_fDBCos))
						{
							Le += p_pGeometryBuffer[index].Indirect;// * Vector3::Dot(normal, p_pGeometryBuffer[index].Surface.ShadingBasisWS.W); 
							contrib++;						
						}
					}
				}
			}

			//for (int dy = -m_nDBSize; dy <= m_nDBSize; dy++)
			//{
			//	for (int dx = -m_nDBSize; dx <= m_nDBSize; dx++)
			//	{
			//		// Central contribution already accumulated
			//		if (dx == 0 && dy == 0) continue;
			//		
			//		// Get index of contributor
			//		int xd = Maths::Min(Maths::Max(x + dx, p_nTileX), endTileX);
			//		int yd = Maths::Min(Maths::Max(y + dy, p_nTileY), endTileY);
			//		
			//		const int index = xd + yd * m_nWidth;
			//		//const int index = (x + dx) + (y + dy) * m_nWidth;

			//		// If a valid intersection, compute
			//		if (!p_pGeometryBuffer[index].Valid) continue;

			//		if (
			//			(Vector3::DistanceSquared(depth, p_pGeometryBuffer[index].Surface.PointWS) < m_fDBDist) &&
			//			(Vector3::Dot(normal, p_pGeometryBuffer[index].Surface.ShadingBasisWS.W) > m_fDBCos)
			//		)
			//		{
			//			Le += p_pGeometryBuffer[index].Indirect;// * Vector3::Dot(normal, p_pGeometryBuffer[index].Surface.ShadingBasisWS.W); 
			//			contrib++;						
			//		}
			//	}
			//}

			p_pGeometryBuffer[indexSrc].Final = p_pGeometryBuffer[indexSrc].Direct + 
				(Le * p_pGeometryBuffer[indexSrc].Reflectance) / contrib;
			
			// p_pGeometryBuffer[indexSrc].Final = (Le * p_pGeometryBuffer[indexSrc].Reflectance) / contrib;
			m_pDevice->Set(m_nWidth - (x + 1), m_nHeight - (y + 1), p_pGeometryBuffer[indexSrc].Final);
		}
	}
}
//----------------------------------------------------------------------------------------------
void MultipassRenderer::ComputeSeparatePasses(Intersection *p_pGeometryBuffer, int p_nTileX, int p_nTileY, int p_nTileWidth, int p_nTileHeight)
{
	// double start = Platform::GetTime();

	Intersection *pIntersection;
	IntegratorContext context;

	// No supersampling
	context.SampleIndex = 0;

	// Compute tile bounds
	int endTileX = p_nTileX + p_nTileWidth;
	int endTileY = p_nTileY + p_nTileHeight;
		
	// Compute dimension reciprocals for normalisation
	float rcpWidth = 1.f / m_nWidth,
		rcpHeight = 1.f / m_nHeight;

	for (int y = p_nTileY; y < endTileY; ++y)
	{
		const int line = y * m_nWidth;

		for (int x = p_nTileX; x < endTileX; ++x)
		{
			pIntersection = p_pGeometryBuffer + line + x;

			context.SurfacePosition.Set(x + 0.5f, y + 0.5f);
			context.NormalisedPosition.Set(context.SurfacePosition.X * rcpWidth, context.SurfacePosition.Y * rcpHeight);

			m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, 0.5f * rcpWidth, 0.5f * rcpHeight, pIntersection->EyeRay);
			pIntersection->Valid = m_pScene->Intersects(pIntersection->EyeRay, *pIntersection);
			m_pDevice->Set(m_nWidth - (x + 1), m_nHeight - (y + 1), pIntersection->Final / m_nSampleCount);
		}
	}

	// double end = Platform::GetTime();
	// std::cout << "Intersection pass : " << Platform::ToSeconds(end - start) << "s" << std::endl;
}
//----------------------------------------------------------------------------------------------
void MultipassRenderer::ComputeIntersectionPass(Intersection *p_pGeometryBuffer, int p_nTileX, int p_nTileY, int p_nTileWidth, int p_nTileHeight)
{
	// double start = Platform::GetTime();
	
	/*
	Intersection *pIntersection;
	IntegratorContext context;

	// No supersampling
	context.SampleIndex = 0;

	// Compute tile bounds
	int endTileX = p_nTileX + p_nTileWidth;
	int endTileY = p_nTileY + p_nTileHeight;
		
	// Compute dimension reciprocals for normalisation
	float rcpWidth = 1.f / m_nWidth,
		rcpHeight = 1.f / m_nHeight;

	for (int y = p_nTileY; y < endTileY; ++y)
	{
		const int line = y * m_nWidth;

		context.SurfacePosition.Y = y + 0.5f;
		context.NormalisedPosition.Y = context.SurfacePosition.Y * rcpHeight;

		for (int x = p_nTileX; x < endTileX; x++)
		{
			pIntersection = p_pGeometryBuffer + line + x;
			
			context.SurfacePosition.X = x + 0.5f;
			context.NormalisedPosition.X = context.SurfacePosition.X * rcpWidth;

			m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, 0.5f, 0.5f, pIntersection->EyeRay);
			pIntersection->Valid = m_pScene->Intersects(pIntersection->EyeRay, *pIntersection);
		}
	}
	*/
	// double end = Platform::GetTime();
	// std::cout << "Intersection pass : " << Platform::ToSeconds(end - start) << "s" << std::endl;
}
//----------------------------------------------------------------------------------------------
void MultipassRenderer::ComputeShadingPass(Intersection *p_pGeometryBuffer, int p_nTileX, int p_nTileY, int p_nTileWidth, int p_nTileHeight)
{
	// Start shading
	// double start = Platform::GetTime();

	Intersection *pIntersection;
	IntegratorContext context;
	Spectrum Li(0), Le(0);
			
	// No supersampling
	context.SampleIndex = 0;

	int endTileX = p_nTileX + p_nTileWidth;
	int endTileY = p_nTileY + p_nTileHeight;

	float rcpWidth = 1.f / m_nWidth,
		rcpHeight = 1.f / m_nHeight;

	for (int y = p_nTileY; y < endTileY; y++)
	{
		const int line = y * m_nWidth;

		for (int x = p_nTileX; x < endTileX; x++)
		{
			pIntersection = p_pGeometryBuffer + line + x;

			/* 
			context.SurfacePosition.X = x + 0.5f;
			context.SurfacePosition.Y = y + 0.5f;

			context.NormalisedPosition.X = x * rcpWidth;
			context.NormalisedPosition.Y = y * rcpHeight;
			
			m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, 0.f, 0.f, pIntersection->EyeRay);
			pIntersection->Valid = m_pScene->Intersects(pIntersection->EyeRay, *pIntersection);
			m_pIntegrator->Radiance(&context, m_pScene, *pIntersection);

			// m_pDevice->Set(m_nWidth - (x + 1), m_nHeight - (y + 1), pIntersection->Indirect);
			/* */

			/**/
			m_pScene->GetSampler()->Reset();

			context.SurfacePosition.X = x * 2;
			context.SurfacePosition.Y = y * 2;

			context.NormalisedPosition.X = x * rcpWidth;
			context.NormalisedPosition.Y = y * rcpHeight;
			
			m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, 0.25f * rcpWidth, 0.25f * rcpHeight, pIntersection->EyeRay);
			pIntersection->Valid = m_pScene->Intersects(pIntersection->EyeRay, *pIntersection);
			m_pIntegrator->Radiance(&context, m_pScene, *pIntersection);
			
			Li = pIntersection->Direct;
			Le = pIntersection->Indirect;
			
			context.SurfacePosition.X = x * 2 + 1;
			context.SurfacePosition.Y = y * 2;
			
			m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, 0.75f * rcpWidth, 0.25f * rcpHeight, pIntersection->EyeRay);
			pIntersection->Valid = m_pScene->Intersects(pIntersection->EyeRay, *pIntersection);
			m_pIntegrator->Radiance(&context, m_pScene, *pIntersection);

			Li += pIntersection->Direct;
			Le += pIntersection->Indirect;

			context.SurfacePosition.X = x * 2;
			context.SurfacePosition.Y = y * 2 + 1;
			
			m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, 0.25f * rcpWidth, 0.75f * rcpHeight, pIntersection->EyeRay);
			pIntersection->Valid = m_pScene->Intersects(pIntersection->EyeRay, *pIntersection);
			m_pIntegrator->Radiance(&context, m_pScene, *pIntersection);

			Li += pIntersection->Direct;
			Le += pIntersection->Indirect;

			context.SurfacePosition.X = x * 2 + 1;
			context.SurfacePosition.Y = y * 2 + 1;
			
			m_pScene->GetCamera()->GetRay(context.NormalisedPosition.X, context.NormalisedPosition.Y, 0.75f * rcpWidth, 0.75f * rcpHeight, pIntersection->EyeRay);
			pIntersection->Valid = m_pScene->Intersects(pIntersection->EyeRay, *pIntersection);
			m_pIntegrator->Radiance(&context, m_pScene, *pIntersection);

			Li += pIntersection->Direct;
			Le += pIntersection->Indirect;

			pIntersection->Direct = Li * 0.25;
			pIntersection->Indirect = Le * 0.25;

			// m_pDevice->Set(m_nWidth - (x + 1), m_nHeight - (y + 1), (Le) * 0.25f);
			/* */
		}
	}

	// double end = Platform::GetTime();
	// std::cout << "Shading pass : " << Platform::ToSeconds(end - start) << "s" << std::endl;
	
	// Start post-processing
	// start = Platform::GetTime();

	for (int y = p_nTileY + m_nDBSize; y < endTileY - m_nDBSize; ++y)
	{
		for (int x = p_nTileX + m_nDBSize; x < endTileX - m_nDBSize; ++x)
		{
			const int indexSrc = x + y * m_nWidth;
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
					const int index = (x + dx) + (y + dy) * m_nWidth;

					// If a valid intersection, compute
					if (!p_pGeometryBuffer[index].Valid) continue;

					if (
						(Vector3::DistanceSquared(depth, p_pGeometryBuffer[index].Surface.PointWS) < m_fDBDist) &&
						(Vector3::Dot(normal, p_pGeometryBuffer[index].Surface.ShadingBasisWS.W) > m_fDBCos)
					)
					{
						Le += p_pGeometryBuffer[index].Indirect;// * Vector3::Dot(normal, p_pGeometryBuffer[index].Surface.ShadingBasisWS.W); 
						contrib++;						
					}
				}
			}

			p_pGeometryBuffer[indexSrc].Final = p_pGeometryBuffer[indexSrc].Direct + 
				(Le * p_pGeometryBuffer[indexSrc].Reflectance) / contrib;

			m_pDevice->Set(m_nWidth - (x + 1), m_nHeight - (y + 1), p_pGeometryBuffer[indexSrc].Final);
		}
	}

	// end = Platform::GetTime();
	// std::cout << "Discontinuity pass : " << Platform::ToSeconds(end - start) << "s" << std::endl;
}
//----------------------------------------------------------------------------------------------
void MultipassRenderer::RenderToAuxiliary(int p_nTileX, int p_nTileY, int p_nTileWidth, int p_nTileHeight, Spectrum *p_colourBuffer)
{
	BOOST_ASSERT(m_pScene != NULL && m_pIntegrator != NULL && m_pFilter != NULL && 
		m_pScene->GetCamera() != NULL && m_pScene->GetSpace() != NULL && m_pScene->GetSampler() != NULL);

	int x = p_nTileX - m_nDBSize;
	if (x < 0) x = 0;

	int width = p_nTileWidth + m_nDBSize + m_nDBSize;
	if (width + x > m_nWidth)
		width = m_nWidth - x;
	
	int y = p_nTileY - m_nDBSize;	
	if (y < 0) y = 0;

	int height = p_nTileHeight + m_nDBSize + m_nDBSize;	
	if (height + y > m_nHeight)
		height = m_nHeight - y;

	if (m_bUseCombinedPass)
	{
		ComputeCombinedPass(m_pGeometryBuffer, x, y, width, height);
	}
	else
	{
		//ComputeCombinedPass(m_pGeometryBuffer, x, y, width, height);
		ComputeIntersectionPass(m_pGeometryBuffer, x, y, width, height);
		ComputeShadingPass(m_pGeometryBuffer, x, y, width, height);
	}

	// Copy colour data into colour buffer
	for (int y = p_nTileY; y < p_nTileY + p_nTileHeight; y++)
	{
		for (int x = p_nTileX; x < p_nTileX + p_nTileWidth; x++)
		{
			*(p_colourBuffer++) = m_pGeometryBuffer[x + y * m_nWidth].Final;
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

	if (!updateIO) m_pDevice->BeginFrame();
	
	if (m_bUseCombinedPass)
	{
		ComputeCombinedPass(m_pGeometryBuffer, 0, 0, width, height);
	}
	else
	{
		ComputeIntersectionPass(m_pGeometryBuffer, 0, 0, width, height);
		ComputeShadingPass(m_pGeometryBuffer, 0, 0, width, height);
	}

	if(!updateIO) { m_pDevice->EndFrame(); }
}
//----------------------------------------------------------------------------------------------