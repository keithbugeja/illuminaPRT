//----------------------------------------------------------------------------------------------
//	Filename:	WhittedIntegrator.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iostream>
#include "Integrator/PathIntegrator.h"

#include "Geometry/Ray.h"
#include "Geometry/Intersection.h"
#include "Sampler/JitterSampler.h"
#include "Material/Material.h"
#include "Spectrum/Spectrum.h"
#include "Staging/Visibility.h"
#include "Staging/Scene.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
PathIntegrator::PathIntegrator(int p_nMaxRayDepth, int p_nSampleCount, int p_nShadowSampleCount, bool p_bShowNormals)
	: m_nMaxRayDepth(p_nMaxRayDepth)
	, m_nSampleCount(p_nSampleCount)
	, m_nShadowSampleCount(p_nShadowSampleCount)
	, m_bShowNormals(p_bShowNormals)
{ }
//----------------------------------------------------------------------------------------------
bool PathIntegrator::Initialise(Scene *p_pScene, ICamera *p_pCamera)
{
	std::cout << "Path Tracing Integrator :: Initialise()" << std::endl;
	return true;
}
//----------------------------------------------------------------------------------------------
bool PathIntegrator::Shutdown(void)
{
	std::cout << "Path Tracing Integrator :: Shutdown()" << std::endl;
	return true;
}
//----------------------------------------------------------------------------------------------
Spectrum PathIntegrator::Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection)
{
	VisibilityQuery visibilityQuery(p_pScene);

	Vector3 wIn, wOut; 
	Vector2 sample;
	
	Spectrum L(0);
	
	//m_nMaxRayDepth = 1;
	//const int maxSampleCount = 1,
	//	maxShadowSampleCount = 16;//128;

	////m_nMaxRayDepth = 4;
	////const int maxSampleCount = 100,
	////	maxShadowSampleCount = 10;

	for (int sampleIdx = 0; sampleIdx < m_nSampleCount; sampleIdx++)
	{
		Spectrum pathThroughput(1.0f);
		bool specularBounce = false;
		Ray ray(p_ray); 
		float pdf;

		for (int rayDepth = 0; rayDepth < m_nMaxRayDepth; rayDepth++)
		{
			if(p_pScene->Intersects(ray, p_intersection))
			{
				wOut = -Vector3::Normalize(ray.Direction);

				//Spectrum Le(0);
				//Le[0] = 0.5 * (p_intersection.Surface.GeometryBasisWS.W[0] + 1);
				//Le[1] = 0.5 * (p_intersection.Surface.GeometryBasisWS.W[1] + 1);
				//Le[2] = 0.5 * (p_intersection.Surface.GeometryBasisWS.W[2] + 1);

				//Le[0] = p_intersection.Surface.GeometryBasisWS.W[0];
				//Le[1] = p_intersection.Surface.GeometryBasisWS.W[1];
				//Le[2] = p_intersection.Surface.GeometryBasisWS.W[2];

				//Le[0] = 0.5 * (wOut[0] + 1);
				//Le[1] = 0.5 * (wOut[1] + 1);
				//Le[2] = 0.5 * (wOut[2] + 1);

				//L = Le;

				// Add emitted light : only on first bounce or specular to avoid double counting
				if (rayDepth == 0 || specularBounce)
				{
					if (p_intersection.IsEmissive()) 
						L += pathThroughput  * p_intersection.GetLight()->Radiance(p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, wOut);
				}

				// Sample all scene lights
				//L += pathThroughput * SampleAllLights(p_pScene, p_intersection, p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut, p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);
				L += pathThroughput * SampleAllLights(p_pScene, p_intersection, p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, wOut, p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);

				// Sample bsdf for next direction
				if (!p_intersection.HasMaterial())
					break;

				// Convert to surface cs
				BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, ray.Direction, wOut);

				// Sample new direction
				sample = p_pScene->GetSampler()->Get2DSample();
				Spectrum f = p_intersection.GetMaterial()->SampleF(wOut, wIn, sample.U, sample.V, &pdf);

				if (f.IsBlack() || pdf == 0.0f)
					break;

				// Convert to world cs
				BSDF::SurfaceToWorld(p_intersection.WorldTransform, p_intersection.Surface, wIn, ray.Direction);
					
				// Compute new ray
				ray.Min = 1e-4f;
				ray.Max = Maths::Maximum;
				ray.Origin = p_intersection.Surface.PointWS + ray.Direction * ray.Min;

				pathThroughput *= f * Vector3::AbsDot(wIn, p_intersection.Surface.GeometryBasisWS.W) / pdf;

				// Possibly terminate the path
				if (rayDepth > 3)
				{
					float continueProbability = Maths::Min(0.5f, pathThroughput[1]);
					if (p_pScene->GetSampler()->Get1DSample() > continueProbability)
						break;
					pathThroughput /= continueProbability;
				}
			}
			else
				break;
		}
	}

	return L / m_nSampleCount;
}
//----------------------------------------------------------------------------------------------