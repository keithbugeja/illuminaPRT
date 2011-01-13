//----------------------------------------------------------------------------------------------
//	Filename:	PathIntegrator.cpp
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
PathIntegrator::PathIntegrator(int p_nMaxRayDepth, int p_nShadowSampleCount, bool p_bShowNormals)
	: m_nShadowSampleCount(p_nShadowSampleCount)
	, m_nMaxRayDepth(p_nMaxRayDepth)
	, m_bShowNormals(p_bShowNormals)
{ }
//----------------------------------------------------------------------------------------------
bool PathIntegrator::Initialise(Scene *p_pScene, ICamera *p_pCamera)
{
	//std::cout << "Path Tracing Integrator :: Initialise()" << std::endl;
	return true;
}
//----------------------------------------------------------------------------------------------
bool PathIntegrator::Shutdown(void)
{
	//std::cout << "Path Tracing Integrator :: Shutdown()" << std::endl;
	return true;
}
//----------------------------------------------------------------------------------------------
Spectrum PathIntegrator::Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection)
{
	VisibilityQuery visibilityQuery(p_pScene);

	Spectrum pathThroughput(1.0f), 
		L(0.0f);
	
	bool specularBounce = false;

	BxDF::Type bxdfType;

	Vector3 wIn, wOut; 
	Vector2 sample;

	Ray ray(p_ray); 

	float pdf;
	
	for (int rayDepth = 0; rayDepth < m_nMaxRayDepth; rayDepth++)
	{
		//----------------------------------------------------------------------------------------------
		// No intersection - light from sky/distant luminaires
		//----------------------------------------------------------------------------------------------
		if(!p_pScene->Intersects(ray, p_intersection))
		{
			if (rayDepth == 0) 
			{
				for (size_t lightIndex = 0; lightIndex < p_pScene->LightList.Size(); ++lightIndex)
				{
					L += p_pScene->LightList[lightIndex]->Radiance(p_ray);
				}
			}
			else if (rayDepth > 0 && specularBounce)
			{
				for (size_t lightIndex = 0; lightIndex < p_pScene->LightList.Size(); ++lightIndex)
				{
					L += pathThroughput * p_pScene->LightList[lightIndex]->Radiance(p_ray);
				}
			}

			break;
		}

		//----------------------------------------------------------------------------------------------
		// Sample lights for specular / first bounce
		//----------------------------------------------------------------------------------------------
		wOut = -Vector3::Normalize(ray.Direction);

		// Add emitted light : only on first bounce or specular to avoid double counting
		if (rayDepth == 0 || specularBounce)
		{
			// wOut is the direction of surface radiance at the point of intersection on the emissive primitive
			if (p_intersection.IsEmissive()) 
				L += pathThroughput  * p_intersection.GetLight()->Radiance(p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, wOut);
		}

		//----------------------------------------------------------------------------------------------
		// Sample lights (direct lighting)
		//----------------------------------------------------------------------------------------------
		//L += pathThroughput * SampleAllLights(p_pScene, p_intersection, p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut, p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);
		L += pathThroughput * SampleAllLights(p_pScene, p_intersection, p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, wOut, p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);

		//----------------------------------------------------------------------------------------------
		// Sample bsdf for next direction
		//----------------------------------------------------------------------------------------------
		if (!p_intersection.HasMaterial())
			break;
			
		// Generate random samples
		sample = p_pScene->GetSampler()->Get2DSample();

		// Convert to surface cs
		BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, wOut, wOut);

		// Sample new direction
		Spectrum f = p_intersection.GetMaterial()->SampleF(p_intersection.Surface, wOut, wIn, sample.U, sample.V, &pdf, BxDF::All_Combined, &bxdfType);

		if (f.IsBlack() || pdf == 0.0f)
			break;

		// Convert to world cs
		BSDF::SurfaceToWorld(p_intersection.WorldTransform, p_intersection.Surface, wIn, wIn);

		//----------------------------------------------------------------------------------------------
		// Set up new bounce
		//----------------------------------------------------------------------------------------------
		ray.Min = 1E-4f;
		ray.Max = Maths::Maximum;
		ray.Origin = p_intersection.Surface.PointWS + wIn * 1E-4f;
		ray.Direction = wIn;

		specularBounce = (bxdfType & BxDF::Specular) != 0;
		pathThroughput *= f * Vector3::Dot(wIn, p_intersection.Surface.GeometryBasisWS.W) / pdf;
		//pathThroughput *= f * Vector3::AbsDot(wIn, p_intersection.Surface.GeometryBasisWS.W) / pdf;

		//----------------------------------------------------------------------------------------------
		// Possibly terminate the path
		//----------------------------------------------------------------------------------------------
		if (rayDepth > 3)
		{
			float continueProbability = Maths::Min(0.5f, pathThroughput[1]);
			if (p_pScene->GetSampler()->Get1DSample() > continueProbability)
				break;
			pathThroughput /= continueProbability;
		}
	}

	return L;
}
//----------------------------------------------------------------------------------------------