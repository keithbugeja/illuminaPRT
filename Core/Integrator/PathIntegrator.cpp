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
#include "Scene/Visibility.h"
#include "Scene/Primitive.h"
#include "Scene/Scene.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
PathIntegrator::PathIntegrator(const std::string &p_strName, int p_nMaxRayDepth, int p_nShadowSampleCount, float p_fReflectEpsilon)
	: IIntegrator(p_strName) 
	, m_nShadowSampleCount(p_nShadowSampleCount)
	, m_nMaxRayDepth(p_nMaxRayDepth)
	, m_fReflectEpsilon(p_fReflectEpsilon)
{ }
//----------------------------------------------------------------------------------------------
PathIntegrator::PathIntegrator(int p_nMaxRayDepth, int p_nShadowSampleCount, float p_fReflectEpsilon)
	: m_nShadowSampleCount(p_nShadowSampleCount)
	, m_nMaxRayDepth(p_nMaxRayDepth)
	, m_fReflectEpsilon(p_fReflectEpsilon)
{ }
//----------------------------------------------------------------------------------------------
bool PathIntegrator::Initialise(Scene *p_pScene, ICamera *p_pCamera)
{
	return true;
}
//----------------------------------------------------------------------------------------------
bool PathIntegrator::Shutdown(void)
{
	return true;
}
//----------------------------------------------------------------------------------------------
Spectrum PathIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection)
{
	VisibilityQuery visibilityQuery(p_pScene);

	Spectrum pathThroughput(1.0f), 
		L(0.0f);

	IMaterial *pMaterial = NULL;

	bool specularBounce = false;

	BxDF::Type bxdfType;

	Vector3 wIn, wOut,
		wInLocal, wOutLocal; 
	Vector2 sample;

	Ray ray(p_ray); 

	float pdf;
	
	for (int rayDepth = 0; rayDepth < m_nMaxRayDepth; rayDepth++)
	{
		//----------------------------------------------------------------------------------------------
		// No intersection
		//----------------------------------------------------------------------------------------------
		if(!p_pScene->Intersects(ray, p_intersection))
		{
			if (rayDepth == 0 || specularBounce) 
			{
				for (size_t lightIndex = 0; lightIndex < p_pScene->LightList.Size(); ++lightIndex)
					L += pathThroughput * p_pScene->LightList[lightIndex]->Radiance(-ray);
			}

			break;
		}
		
		//----------------------------------------------------------------------------------------------
		// Primitive has no material assigned - terminate
		//----------------------------------------------------------------------------------------------
		if (!p_intersection.HasMaterial()) 
			break;
		
		// Get material for intersection primitive
		pMaterial = p_intersection.GetMaterial();

		//----------------------------------------------------------------------------------------------
		// Sample lights for specular / first bounce
		//----------------------------------------------------------------------------------------------
		wOut = -Vector3::Normalize(ray.Direction);

		// Add emitted light : only on first bounce or specular to avoid double counting
		if (p_intersection.IsEmissive())
		{
			if (rayDepth == 0 || specularBounce)
			{
				// Add contribution from luminaire
				// -- Captures highlight on specular materials
				// -- Transmits light through dielectrics
				// -- Renders light primitive for first bounce intersections
				L += pathThroughput * p_intersection.GetLight()->Radiance(p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, wOut);

				if (rayDepth == 0) break;
			}
		}

		//----------------------------------------------------------------------------------------------
		// Sample lights for direct lighting
		// -- If the currently intersected primitive is a luminaire, do not sample it 
		//----------------------------------------------------------------------------------------------
		if (!specularBounce)
			L += pathThroughput * SampleAllLights(p_pScene, p_intersection, p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut, p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);
			//L += pathThroughput * SampleAllLights(p_pScene, p_intersection, p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, wOut, p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);
			
		//----------------------------------------------------------------------------------------------
		// Sample bsdf for next direction
		//----------------------------------------------------------------------------------------------
		// Generate random samples
		sample = p_pScene->GetSampler()->Get2DSample();

		// Convert to surface coordinate system where (0,0,1) represents surface normal
		// Note: 
		// -- All Material/BSDF/BxDF operations are carried out in surface coordinates
		// -- All inputs must be in surface coordinates
		// -- All outputs are in surface coordinates

		BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, wOut, wOutLocal);

		// Sample new direction in wIn (remember we're tracing backwards)
		// -- wIn returns the sampled direction
		// -- pdf returns the reflectivity function's pdf at the sampled point
		// -- bxdfType returns the type of BxDF sampled
		Spectrum f = pMaterial->SampleF(p_intersection.Surface, wOutLocal, wInLocal, sample.U, sample.V, &pdf, BxDF::All_Combined, &bxdfType);

		// If the reflectivity or pdf are zero, terminate path
		if (f.IsBlack() || pdf == 0.0f) break;

		// Record if bounce is a specular bounce
		specularBounce = ((int)(bxdfType & BxDF::Specular)) != 0;

		// Convert back to world coordinates
		BSDF::SurfaceToWorld(p_intersection.WorldTransform, p_intersection.Surface, wInLocal, wIn);

		//----------------------------------------------------------------------------------------------
		// Adjust path for new bounce
		// -- ray is moved by a small epsilon in sampled direction
		// -- ray origin is set to point of intersection
		//----------------------------------------------------------------------------------------------
		ray.Set(p_intersection.Surface.PointWS + wIn * m_fReflectEpsilon, wIn, 0.f, Maths::Maximum);

		//ray.Min = 0.f;
		//ray.Max = Maths::Maximum;
		//ray.Origin = p_intersection.Surface.PointWS + wIn * m_fReflectEpsilon;
		//ray.Direction = wIn;
		//Vector3::Inverse(ray.Direction, ray.DirectionInverseCache);
		
		// Update path contribution at current stage
		pathThroughput *= f * Vector3::AbsDot(wIn, p_intersection.Surface.GeometryBasisWS.W) / pdf;

		//----------------------------------------------------------------------------------------------
		// Use Russian roulette to possibly terminate path
		//----------------------------------------------------------------------------------------------
		if (rayDepth > 3)
		{
			float continueProbability = Maths::Min(0.5f, 0.33f * (pathThroughput[0] + pathThroughput[1] + pathThroughput[2]));

			if (p_pScene->GetSampler()->Get1DSample() > continueProbability)
				break;
			pathThroughput /= continueProbability;
		}
	}

	return L;
}
//----------------------------------------------------------------------------------------------