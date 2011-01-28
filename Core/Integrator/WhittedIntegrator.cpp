//----------------------------------------------------------------------------------------------
//	Filename:	WhittedIntegrator.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iostream>
#include "Integrator/WhittedIntegrator.h"

#include "Geometry/Ray.h"
#include "Geometry/Intersection.h"
#include "Sampler/JitterSampler.h"
#include "Material/Material.h"
#include "Spectrum/Spectrum.h"
#include "Staging/Visibility.h"
#include "Staging/Scene.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
WhittedIntegrator::WhittedIntegrator(int p_nMaxRayDepth, int p_nShadowSampleCount)
	: m_nShadowSampleCount(p_nShadowSampleCount)
	, m_nMaxRayDepth(p_nMaxRayDepth)
{ }
//----------------------------------------------------------------------------------------------
WhittedIntegrator::WhittedIntegrator(const std::string &p_strId, int p_nMaxRayDepth, int p_nShadowSampleCount)
	: IIntegrator(p_strId) 
	, m_nShadowSampleCount(p_nShadowSampleCount)
	, m_nMaxRayDepth(p_nMaxRayDepth)
{ }
//----------------------------------------------------------------------------------------------
bool WhittedIntegrator::Initialise(Scene *p_pScene, ICamera *p_pCamera)
{
	//std::cout << "Path Tracing Integrator :: Initialise()" << std::endl;
	return true;
}
//----------------------------------------------------------------------------------------------
bool WhittedIntegrator::Shutdown(void)
{
	//std::cout << "Path Tracing Integrator :: Shutdown()" << std::endl;
	return true;
}
//----------------------------------------------------------------------------------------------
Spectrum WhittedIntegrator::Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection)
{
	return Radiance(p_pScene, p_ray, p_intersection, 0);
}
//----------------------------------------------------------------------------------------------
Spectrum WhittedIntegrator::Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, int p_nRayDepth)
{
	if (p_nRayDepth > m_nMaxRayDepth) 
		return 0;

	Ray ray(p_ray); 

	if(p_pScene->Intersects(ray, p_intersection))
	{
		VisibilityQuery visibilityQuery(p_pScene);

		Spectrum Ls(0.0f), Lt(0.0f), Ld(0.0f);
	
		Vector3 wIn, 
			wOut = -ray.Direction; 
		
		Vector2 sample;
		BxDF::Type bxdfType;
		float pdf;

		IMaterial *pMaterial = p_intersection.GetMaterial();

		if (p_intersection.IsEmissive() && (p_nRayDepth == 0 || pMaterial->HasBxDFType(BxDF::Specular))) return p_intersection.GetLight()->Radiance(p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, wOut);

		// Diffuse next
		if (!pMaterial->HasBxDFType(BxDF::Specular))
			Ld = SampleAllLights(p_pScene, p_intersection, p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, wOut, p_pScene->GetSampler(), 1);

		bool IsSpecularReflective = false,
			IsSpecularTransmissive = false;

		// Reflection
		/**/
		if (pMaterial->HasBxDFType(BxDF::Type(BxDF::Specular | BxDF::Reflection)))
		{
			IsSpecularReflective = true;

			sample = p_pScene->GetSampler()->Get2DSample();
			
			Vector3 i,o;

			BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, wOut, o);
			Spectrum f = p_intersection.GetMaterial()->SampleF(p_intersection.Surface, o, i, sample.U, sample.V, &pdf, BxDF::Type(BxDF::Reflection), &bxdfType);
			BSDF::SurfaceToWorld(p_intersection.WorldTransform, p_intersection.Surface, i, wIn);

			ray.Origin = p_intersection.Surface.PointWS;// + p_intersection.Surface.GeometryBasisWS.W * 1E-6f;
			ray.Direction = wIn;
			ray.Min = 1e-4f;
			ray.Max = Maths::Maximum;

			if (!f.IsBlack() && pdf != 0.f)
			{
				Intersection intersection;
				Ls = f * Radiance(p_pScene, ray, intersection, p_nRayDepth + 1);
			}
		}
		//*/
		/**/
		// Refraction
	/**/
		if (pMaterial->HasBxDFType(BxDF::Type(BxDF::Specular | BxDF::Transmission)))
		{
			IsSpecularTransmissive = true;

			sample = p_pScene->GetSampler()->Get2DSample();
			
			Vector3 i,o;

			BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, wOut, o);
			Spectrum f = p_intersection.GetMaterial()->SampleF(p_intersection.Surface, o, i, sample.U, sample.V, &pdf, BxDF::Type(BxDF::Transmission), &bxdfType);
			//i = -o;
			//Spectrum f(1,0,0);
			//pdf = 1.0f;
			BSDF::SurfaceToWorld(p_intersection.WorldTransform, p_intersection.Surface, i, wIn);

			Vector3 j = -wOut;

			//wIn = -wOut;

			ray.Direction = wIn;
			ray.Origin = p_intersection.Surface.PointWS; // + wIn * 1E-6f;
			ray.Min = 1e-4f;
			ray.Max = Maths::Maximum;

			if (!f.IsBlack() && pdf != 0.f)
			{
				Intersection intersection;
				Lt = f * Radiance(p_pScene, ray, intersection, p_nRayDepth + 1);
			}
		}
	/**/
		
		return Ld + Ls + Lt;
	}

	return 0.0f;
}
//----------------------------------------------------------------------------------------------