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
	return true;
}
//----------------------------------------------------------------------------------------------
Spectrum WhittedIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, Intersection &p_intersection, RadianceContext *p_pRadianceContext)
{
	VisibilityQuery visibilityQuery(p_pScene);

	Spectrum pathThroughput(1.f), 
		L(0.f);
	
	IMaterial *pMaterial = NULL;
	//bool specularBounce = false;

	//BxDF::Type bxdfType;
	
	Vector3 wIn, wOut, 
		wInLocal, wOutLocal; 

	Vector2 sample;

	//float pdf;

	int rayDepth = 0;

	//----------------------------------------------------------------------------------------------
	// Avoid having to perform multiple checks for a NULL radiance context
	//----------------------------------------------------------------------------------------------
	RadianceContext radianceContext;

	if (p_pRadianceContext == NULL)
		p_pRadianceContext = &radianceContext;
	
	// Initialise context
	p_pRadianceContext->Indirect = 
		p_pRadianceContext->Direct = 
		p_pRadianceContext->Albedo = 0.f;

	// Construct ray from intersection details
	Ray ray(p_intersection.Surface.RayOriginWS, 
		p_intersection.Surface.RayDirectionWS); 

	//----------------------------------------------------------------------------------------------
	// If intersection is invalid, return
	//----------------------------------------------------------------------------------------------
	if (!p_intersection.IsValid())
	{
		for (size_t lightIndex = 0; lightIndex < p_pScene->LightList.Size(); ++lightIndex)
			p_pRadianceContext->Direct += p_pScene->LightList[lightIndex]->Radiance(-ray);

		return p_pRadianceContext->Direct;
	} 

	//----------------------------------------------------------------------------------------------
	// Set spatial information to radiance context
	//----------------------------------------------------------------------------------------------
	p_pRadianceContext->SetSpatialContext(&p_intersection);

	//----------------------------------------------------------------------------------------------
	// Primitive has no material assigned - terminate
	//----------------------------------------------------------------------------------------------
	if (p_intersection.HasMaterial())
	{
		pMaterial = p_intersection.GetMaterial();
	
		//----------------------------------------------------------------------------------------------
		// Sample lights for specular / first bounce
		//----------------------------------------------------------------------------------------------
		wOut = -Vector3::Normalize(ray.Direction);
	
		//----------------------------------------------------------------------------------------------
		// Additional logic for first hit
		//----------------------------------------------------------------------------------------------
		if (rayDepth == 0) 
		{
			// Set albedo for first hit
			p_pRadianceContext->Albedo = pMaterial->Rho(wOut, p_intersection.Surface);

			// If an emissive material has been encountered, add contribution and terminate path
			if (p_intersection.IsEmissive())
			{
				p_pRadianceContext->Direct = 
					p_pRadianceContext->Final = 
					p_intersection.GetLight()->Radiance(p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, wOut); 
				
				return p_pRadianceContext->Final;
			}
		}

		//----------------------------------------------------------------------------------------------
		// Sample lights for direct lighting
		//----------------------------------------------------------------------------------------------
		if (pMaterial->HasBxDFType(BxDF::Reflection, false))
		{
			p_pRadianceContext->Direct = 
				SampleAllLights(p_pScene, p_intersection, 
				p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut, 
				p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);
		}
	}

	return p_pRadianceContext->Direct;
}
//----------------------------------------------------------------------------------------------
Spectrum WhittedIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, RadianceContext *p_pRadianceContext)
{
	// Compute intersection step
	if (!p_pScene->Intersects(Ray(p_ray), p_intersection))
	{
		p_intersection.Surface.RayOriginWS = p_ray.Origin;
		p_intersection.Surface.RayDirectionWS = p_ray.Direction;
	}

	return Radiance(p_pContext, p_pScene, p_intersection, p_pRadianceContext);
}
//----------------------------------------------------------------------------------------------
/*
Spectrum WhittedIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, RadianceContext *p_pRadianceContext)
{
	// Compute intersection step
	if (!p_pScene->Intersects(Ray(p_ray), p_intersection))
	{
		p_intersection.Surface.RayOriginWS = p_ray.Origin;
		p_intersection.Surface.RayDirectionWS = p_ray.Direction;
	}

	return Radiance(p_pContext, p_pScene, p_intersection, p_pRadianceContext);
}
//----------------------------------------------------------------------------------------------
Spectrum WhittedIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, Intersection &p_intersection, RadianceContext *p_pRadianceContext)
{
	p_pScene->GetSampler()->Reset();

	//----------------------------------------------------------------------------------------------
	// Avoid having to perform multiple checks for a NULL radiance context
	//----------------------------------------------------------------------------------------------
	RadianceContext radianceContext;

	if (p_pRadianceContext == NULL)
		p_pRadianceContext = &radianceContext;
	
	// Initialise context
	p_pRadianceContext->Indirect = 
		p_pRadianceContext->Direct = 
		p_pRadianceContext->Albedo = 0.f;

	// Construct ray from intersection details
	Ray ray(p_intersection.Surface.RayOriginWS, 
		p_intersection.Surface.RayDirectionWS); 

	// No intersection
	if (!p_intersection.IsValid())
	{
		for (size_t lightIndex = 0; lightIndex < p_pScene->LightList.Size(); ++lightIndex)
			p_pRadianceContext->Direct += p_pScene->LightList[lightIndex]->Radiance(-ray);

		return p_pRadianceContext->Final = p_pRadianceContext->Direct;
	}

	//----------------------------------------------------------------------------------------------
	// Set spatial information to radiance context
	//----------------------------------------------------------------------------------------------
	p_pRadianceContext->SetSpatialContext(&p_intersection);

	//----------------------------------------------------------------------------------------------
	// Primitive has no material assigned - terminate
	//----------------------------------------------------------------------------------------------
	if (!p_intersection.HasMaterial()) 
		p_pRadianceContext->Direct;
		
	IMaterial *pMaterial = p_intersection.GetMaterial();

	//----------------------------------------------------------------------------------------------
	// Sample lights for specular / first bounce
	//----------------------------------------------------------------------------------------------
	Vector3 wOut = -Vector3::Normalize(ray.Direction);

	// Set albedo for first hit
	p_pRadianceContext->Albedo = pMaterial->Rho(wOut, p_intersection.Surface);

	// If an emissive material has been encountered, add contribution and terminate path
	if (p_intersection.IsEmissive())
	{
		p_pRadianceContext->Final = p_pRadianceContext->Direct = 
			p_intersection.GetLight()->Radiance(p_intersection.Surface.PointWS, 
								p_intersection.Surface.GeometryBasisWS.W, wOut); 
				
		return p_pRadianceContext->Final;
	}

	p_pRadianceContext->Direct += SampleAllLights(p_pScene, p_intersection, 
				p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut, 
				p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);

	return p_pRadianceContext->Final = p_pRadianceContext->Direct;
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

					Ld += Ls * Maths::Max(0, Vector3::Dot(wIn, p_intersection.Surface.ShadingBasisWS.W)) * pMaterial->F(p_intersection.Surface, bsdfOut, bsdfIn);
				}
			}
		}

		bool IsSpecularReflective = false,
			IsSpecularTransmissive = false;

		// Reflection
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

			ray.Set(p_intersection.Surface.PointWS + wIn * 1E-4f, wIn, 1E-4f, Maths::Maximum);
			
			//ray.Origin = p_intersection.Surface.PointWS + wIn * 1E-4f;
			//ray.Direction = wIn;
			//ray.Min = 1E-4f;
			//ray.Max = Maths::Maximum;
			//Vector3::Inverse(ray.Direction, ray.DirectionInverseCache);

			if (!f.IsBlack() && pdf != 0.f)
			{
				Intersection intersection;
				Ls = f * Radiance(p_pContext, p_pScene, ray, intersection, p_nRayDepth + 1);
			}
		} 

		// Refraction
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

			ray.Set(p_intersection.Surface.PointWS + wIn * 1E-4f, wIn, 1E-4f, Maths::Maximum);

			//ray.Direction = wIn;
			//ray.Origin = p_intersection.Surface.PointWS  + wIn * 1E-4f;
			//ray.Min = 1e-4f;
			//ray.Max = Maths::Maximum;
			//Vector3::Inverse(ray.Direction, ray.DirectionInverseCache);

			if (!f.IsBlack() && pdf != 0.f)
			{
				Intersection intersection;
				Lt = f * Radiance(p_pContext, p_pScene, ray, intersection, p_nRayDepth + 1);
			}
		}
		
		return Ld + Ls + Lt;
	}

	return 0.0f;
}
*/
//----------------------------------------------------------------------------------------------