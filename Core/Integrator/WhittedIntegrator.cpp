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
#include "Scene/Visibility.h"
#include "Scene/Scene.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
WhittedIntegrator::WhittedIntegrator(const std::string &p_strName, int p_nMaxRayDepth, int p_nShadowSampleCount, bool p_bEnableShadowRays)
	: IIntegrator(p_strName) 
	, m_nMaxRayDepth(p_nMaxRayDepth)
	, m_nShadowSampleCount(p_nShadowSampleCount)
	, m_bEnableShadowRays(p_bEnableShadowRays)
{ }
//----------------------------------------------------------------------------------------------
WhittedIntegrator::WhittedIntegrator(int p_nMaxRayDepth, int p_nShadowSampleCount, bool p_bEnableShadowRays)
	: m_nMaxRayDepth(p_nMaxRayDepth)
	, m_nShadowSampleCount(p_nShadowSampleCount)
	, m_bEnableShadowRays(p_bEnableShadowRays)
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
Spectrum WhittedIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection)
{
	return Radiance(p_pContext, p_pScene, p_ray, p_intersection, 0);
}
//----------------------------------------------------------------------------------------------
Spectrum WhittedIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, int p_nRayDepth)
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

		//if (p_intersection.IsEmissive())
			//return Spectrum(10,10,0);

		//if (p_intersection.IsEmissive() && (p_nRayDepth == 0 || pMaterial->HasBxDFType(BxDF::Specular))) 
		//	return Spectrum(1,0,1);
		//return p_intersection.GetLight()->Radiance(p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, wOut);

		//Spectrum r;
		//r[0] = 0.5f * (1.0f + p_intersection.Surface.ShadingNormal[0]); 
		//r[1] = 0.5f * (1.0f + p_intersection.Surface.ShadingNormal[1]); 
		//r[2] = 0.5f * (1.0f + p_intersection.Surface.ShadingNormal[2]); 

		//return r;
		////return r * 10;
		//float cosT = Vector3::Dot(p_intersection.Surface.GeometryNormal, Vector3::UnitYNeg);	
		//cosT = p_intersection.Surface.Distance;
		//return cosT * 10.0f;

		// Diffuse next
		if (!pMaterial->HasBxDFType(BxDF::Specular))
		{
			if (m_bEnableShadowRays)
			{
				Ld = SampleAllLights(p_pScene, p_intersection, p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut, p_pScene->GetSampler(), 1);
			}
			else
			{
				for (size_t lightIdx = 0; lightIdx < p_pScene->LightList.Size(); ++lightIdx)
				{
					sample = p_pScene->GetSampler()->Get2DSample();
					Spectrum Ls = p_pScene->LightList[0]->SampleRadiance(p_intersection.Surface.PointWS, sample.U, sample.V, wIn, pdf, visibilityQuery);
				
					wIn = -wIn;

					Vector3 bsdfIn, bsdfOut;

					BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, wOut, bsdfOut);
					BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, wIn, bsdfIn);

					//Ld += Ls * Maths::Max(0, Vector3::Dot(wIn, p_intersection.Surface.GeometryBasisWS.W)) * pMaterial->F(p_intersection.Surface, bsdfOut, bsdfIn);
					Ld += Ls * Maths::Max(0, Vector3::Dot(wIn, p_intersection.Surface.ShadingBasisWS.W)) * pMaterial->F(p_intersection.Surface, bsdfOut, bsdfIn);
				}
			}
		}

		bool IsSpecularReflective = false,
			IsSpecularTransmissive = false;

		// Reflection
		/**/ /**/
		if (pMaterial->HasBxDFType(BxDF::Type(BxDF::Specular | BxDF::Reflection)))
		{
			IsSpecularReflective = true;

			sample = p_pScene->GetSampler()->Get2DSample();
			
			Vector3 i,o;

			Vector3::Reflect(-wOut, p_intersection.Surface.ShadingNormal, wIn);
			Spectrum f(1); pdf = 1;

			//BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, wOut, o, false);
			//Spectrum f = p_intersection.GetMaterial()->SampleF(p_intersection.Surface, o, i, sample.U, sample.V, &pdf, BxDF::Type(BxDF::Reflection), &bxdfType);
			//BSDF::SurfaceToWorld(p_intersection.WorldTransform, p_intersection.Surface, i, wIn, false);

			//f = 0;

			//ray.Origin = p_intersection.Surface.PointWS + p_intersection.Surface.ShadingNormal * 1E-4f;
			ray.Origin = p_intersection.Surface.PointWS + wIn * 1E-4f;
			ray.Direction = wIn;
			ray.Min = 1E-4f;
			ray.Max = Maths::Maximum;

			if (!f.IsBlack() && pdf != 0.f)
			{
				Intersection intersection;
				Ls = f * Radiance(p_pContext, p_pScene, ray, intersection, p_nRayDepth + 1);
			}
		} 
		/**/
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
			BSDF::SurfaceToWorld(p_intersection.WorldTransform, p_intersection.Surface, i, wIn);

			//ray.Origin = p_intersection.Surface.PointWS + p_intersection.Surface.GeometryBasisWS.W * 1E-5f;
			//ray.Direction = wIn;
			//ray.Min = 0.f;
			//ray.Max = Maths::Maximum;

			ray.Direction = wIn;
			ray.Origin = p_intersection.Surface.PointWS  + wIn * 1E-4f;
			ray.Min = 1e-4f;
			ray.Max = Maths::Maximum;

			if (!f.IsBlack() && pdf != 0.f)
			{
				Intersection intersection;
				Lt = f * Radiance(p_pContext, p_pScene, ray, intersection, p_nRayDepth + 1);
			}
		}
	/**/
		
		return Ld + Ls + Lt;
	}

	return 0.0f;
}
//----------------------------------------------------------------------------------------------