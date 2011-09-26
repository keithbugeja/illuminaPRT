//----------------------------------------------------------------------------------------------
//	Filename:	WhittedIntegrator.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iostream>
#include "Integrator/TestIntegrator.h"

#include "Geometry/Ray.h"
#include "Geometry/Intersection.h"
#include "Sampler/JitterSampler.h"
#include "Material/Material.h"
#include "Spectrum/Spectrum.h"
#include "Scene/Visibility.h"
#include "Scene/Scene.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
TestIntegrator::TestIntegrator(const std::string &p_strName)
	: IIntegrator(p_strName) 
{ }
//----------------------------------------------------------------------------------------------
TestIntegrator::TestIntegrator()
{ }
//----------------------------------------------------------------------------------------------
bool TestIntegrator::Initialise(Scene *p_pScene, ICamera *p_pCamera)
{
	return true;
}
//----------------------------------------------------------------------------------------------
bool TestIntegrator::Shutdown(void)
{
	return true;
}
//----------------------------------------------------------------------------------------------
Spectrum TestIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection)
{
	return Radiance(p_pContext, p_pScene, p_ray, p_intersection, 0);
}
//----------------------------------------------------------------------------------------------
Spectrum TestIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, int p_nRayDepth)
{
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

		if (p_intersection.IsEmissive())
			return Spectrum(100);

		Vector3 normal = (Vector3(1) + p_intersection.Surface.GeometryBasisWS.W) * 0.5f;
		return Spectrum(normal.X, normal.Y, normal.Z);

		// Diffuse next
		if (!pMaterial->HasBxDFType(BxDF::Specular))
		{
			//if (m_bEnableShadowRays)
			//{
			//	Ld = SampleAllLights(p_pScene, p_intersection, p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut, p_pScene->GetSampler(), 1);
			//}
			//else
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