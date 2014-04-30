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
	, m_nMaxRayDepth(p_nMaxRayDepth)
	, m_nShadowSampleCount(p_nShadowSampleCount)
	, m_fReflectEpsilon(p_fReflectEpsilon)
{ }
//----------------------------------------------------------------------------------------------
PathIntegrator::PathIntegrator(int p_nMaxRayDepth, int p_nShadowSampleCount, float p_fReflectEpsilon)
	: m_nMaxRayDepth(p_nMaxRayDepth)
	, m_nShadowSampleCount(p_nShadowSampleCount)
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
Spectrum PathIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, Intersection &p_intersection, RadianceContext *p_pRadianceContext)
{
	VisibilityQuery visibilityQuery(p_pScene);

	Spectrum pathThroughput(1.f);
		// L(0.f);
	
	IMaterial *pMaterial = NULL;
	bool specularBounce = false;

	BxDF::Type bxdfType;
	
	Vector3 wIn, wOut, 
		wInLocal, wOutLocal; 

	Vector2 directionSample;

	float bsdfSample, 
		pdf;

	//----------------------------------------------------------------------------------------------
	// Avoid having to perform multiple checks for a NULL radiance context
	//----------------------------------------------------------------------------------------------
	RadianceContext radianceContext;

	if (p_pRadianceContext == NULL)
		p_pRadianceContext = &radianceContext;
	
	// Initialise context
	p_pRadianceContext->Flags = 0;

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

		p_pRadianceContext->Flags |= 
			RadianceContext::DF_Computed | RadianceContext::DF_Direct;

		return p_pRadianceContext->Direct;
	} 

	//----------------------------------------------------------------------------------------------
	// Set spatial information to radiance context
	//----------------------------------------------------------------------------------------------
	p_pRadianceContext->SetSpatialContext(&p_intersection);

	Spectrum *L = &(p_pRadianceContext->Direct);
	//----------------------------------------------------------------------------------------------
	// Start tracing path
	//----------------------------------------------------------------------------------------------
	for (int rayDepth = 0; rayDepth < m_nMaxRayDepth; rayDepth++)
	{
		//----------------------------------------------------------------------------------------------
		// Primitive has no material assigned - terminate
		//----------------------------------------------------------------------------------------------
		if (!p_intersection.HasMaterial()) 
			break;
		
		pMaterial = p_intersection.GetMaterial();

		//----------------------------------------------------------------------------------------------
		// Sample lights for specular / first bounce
		//----------------------------------------------------------------------------------------------
		wOut = -Vector3::Normalize(ray.Direction);

		if (rayDepth == 0) p_pRadianceContext->Albedo = pMaterial->Rho(wOut, p_intersection.Surface);

		//----------------------------------------------------------------------------------------------
		// Compute emissive and direct lighting
		//----------------------------------------------------------------------------------------------
		if ((rayDepth == 0 || specularBounce) && p_intersection.IsEmissive()) 
			*L += pathThroughput * 
				p_intersection.GetLight()->Radiance(p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut); 

		*L += pathThroughput * SampleAllLights(p_pScene, p_intersection, 
			p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut, 
			p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);

		//----------------------------------------------------------------------------------------------
		// Sample bsdf for surface bxdf and direction
		//----------------------------------------------------------------------------------------------
		// Generate random samples
		directionSample = p_pScene->GetSampler()->Get2DSample();
		bsdfSample = p_pScene->GetSampler()->Get1DSample();

		// Convert to surface coordinate system where (0,0,1) represents surface normal
		// Note: 
		// -- All Material/BSDF/BxDF operations are carried out in surface coordinates
		// -- All inputs must be in surface coordinates
		// -- All outputs are in surface coordinates
		BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, wOut, wOutLocal, true);

		// Sample new direction in wIn (remember we're tracing backwards)
		// -- wIn returns the sampled direction
		// -- pdf returns the reflectivity function's pdf at the sampled point
		// -- bxdfType returns the type of BxDF sampled
		Spectrum f = pMaterial->SampleF(p_intersection.Surface, wOutLocal, wInLocal, directionSample.U, directionSample.V, bsdfSample, &pdf, BxDF::All_Combined, &bxdfType);

		// If the reflectivity or pdf are zero, terminate path
		if (f.IsBlack() || pdf == 0.0f) break;

		// Record if bounce is a specular bounce
		specularBounce = ((int)(bxdfType & BxDF::Specular)) != 0;

		// Convert back to world coordinates
		BSDF::SurfaceToWorld(p_intersection.WorldTransform, p_intersection.Surface, wInLocal, wIn, true);

		// Update path contribution at current stage
		pathThroughput *= f * Vector3::AbsDot(wIn, p_intersection.Surface.ShadingBasisWS.W) / pdf;

		//----------------------------------------------------------------------------------------------
		// Use Russian roulette to possibly terminate path
		//----------------------------------------------------------------------------------------------
		if (rayDepth > 3)
		{
			float continueProbability = Maths::Min(0.5f, pathThroughput.Luminance());

			if (p_pScene->GetSampler()->Get1DSample() > continueProbability)
				break;

			pathThroughput /= continueProbability;
		}

		//----------------------------------------------------------------------------------------------
		// Adjust path for new bounce
		// -- ray is moved by a small epsilon in normal direction
		// -- ray origin is set to point of intersection
		//----------------------------------------------------------------------------------------------
		if (specularBounce && (int)(bxdfType & BxDF::Transmission)) 
			ray.Set(p_intersection.Surface.PointWS - p_intersection.Surface.GeometryBasisWS.W * m_fReflectEpsilon, wIn, m_fReflectEpsilon, Maths::Maximum);
		else
			ray.Set(p_intersection.Surface.PointWS + p_intersection.Surface.GeometryBasisWS.W * m_fReflectEpsilon, wIn, m_fReflectEpsilon, Maths::Maximum);

		//----------------------------------------------------------------------------------------------
		// Find next intersection
		//----------------------------------------------------------------------------------------------
		if(!p_pScene->Intersects(ray, p_intersection))
		{
			if (specularBounce) 
				for (size_t lightIndex = 0; lightIndex < p_pScene->LightList.Size(); ++lightIndex)
					p_pRadianceContext->Indirect += pathThroughput * p_pScene->LightList[lightIndex]->Radiance(-ray);

			break;
		}

		L = &(p_pRadianceContext->Indirect);

		/*
		//----------------------------------------------------------------------------------------------
		// Primitive has no material assigned - terminate
		//----------------------------------------------------------------------------------------------
		if (!p_intersection.HasMaterial()) 
			break;
		
		pMaterial = p_intersection.GetMaterial();

		// Kludge for sampling of direct lighting
		p_intersection.Surface.PointWS += p_intersection.Surface.GeometryBasisWS.W * m_fReflectEpsilon;

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
				p_pRadianceContext->Direct += pathThroughput * 
					p_intersection.GetLight()->Radiance(p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, wOut); 
				
				break;
			}
		}

		//----------------------------------------------------------------------------------------------
		// Sample lights for direct lighting
		// -- If the currently intersected primitive is a luminaire, do not sample it 
		//----------------------------------------------------------------------------------------------
		if (!specularBounce)
		{
			if (rayDepth == 0)
			{
				p_pRadianceContext->Direct += pathThroughput * SampleAllLights(p_pScene, p_intersection, 
					p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut, 
					p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);
			} 
			else 
			{
				p_pRadianceContext->Indirect += pathThroughput * SampleAllLights(p_pScene, p_intersection, 
					p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut, 
					p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);
			}
		}
		else
		{
			//----------------------------------------------------------------------------------------------
			// Add emitted light : only on first bounce (above) or specular to avoid double counting
			//----------------------------------------------------------------------------------------------
			// Add contribution from luminaire
			// -- Captures highlight on specular materials
			// -- Transmits light through dielectrics
			// -- Renders light primitive for first bounce intersections
			if (p_intersection.IsEmissive())
			{
				p_pRadianceContext->Indirect += pathThroughput * 
				p_intersection.GetLight()->Radiance(p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, wOut);
			}
		}
		
		//----------------------------------------------------------------------------------------------
		// Sample bsdf for next direction
		//----------------------------------------------------------------------------------------------
		// Generate random samples
		directionSample = p_pScene->GetSampler()->Get2DSample();
		bsdfSample = p_pScene->GetSampler()->Get1DSample();

		// Convert to surface coordinate system where (0,0,1) represents surface normal
		// Note: 
		// -- All Material/BSDF/BxDF operations are carried out in surface coordinates
		// -- All inputs must be in surface coordinates
		// -- All outputs are in surface coordinates

		BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, wOut, wOutLocal, true);

		// Sample new direction in wIn (remember we're tracing backwards)
		// -- wIn returns the sampled direction
		// -- pdf returns the reflectivity function's pdf at the sampled point
		// -- bxdfType returns the type of BxDF sampled
		Spectrum f = pMaterial->SampleF(p_intersection.Surface, wOutLocal, wInLocal, directionSample.U, directionSample.V, bsdfSample, &pdf, BxDF::All_Combined, &bxdfType);

		// If the reflectivity or pdf are zero, terminate path
		if (f.IsBlack() || pdf == 0.0f) break;

		// Record if bounce is a specular bounce
		specularBounce = ((int)(bxdfType & BxDF::Specular)) != 0;

		// Convert back to world coordinates
		BSDF::SurfaceToWorld(p_intersection.WorldTransform, p_intersection.Surface, wInLocal, wIn, true);
		
		//----------------------------------------------------------------------------------------------
		// Adjust path for new bounce
		// -- ray is moved by a small epsilon in sampled direction
		// -- ray origin is set to point of intersection
		//----------------------------------------------------------------------------------------------
		if (specularBounce && (int)(bxdfType & BxDF::Transmission)) 
			ray.Set(p_intersection.Surface.PointWS - p_intersection.Surface.GeometryBasisWS.W * m_fReflectEpsilon, wIn, m_fReflectEpsilon, Maths::Maximum);
		else
			ray.Set(p_intersection.Surface.PointWS + p_intersection.Surface.GeometryBasisWS.W * m_fReflectEpsilon, wIn, m_fReflectEpsilon, Maths::Maximum);
		
		// ray.Set(p_intersection.Surface.PointWS + wIn * m_fReflectEpsilon, wIn, m_fReflectEpsilon, Maths::Maximum);
		// ray.Set(p_intersection.Surface.PointWS, wIn, m_fReflectEpsilon, Maths::Maximum);
		
		// Update path contribution at current stage
		pathThroughput *= f * Vector3::AbsDot(wIn, p_intersection.Surface.ShadingBasisWS.W) / pdf;

		//----------------------------------------------------------------------------------------------
		// Use Russian roulette to possibly terminate path
		//----------------------------------------------------------------------------------------------
		if (rayDepth > 3)
		{
			float continueProbability = Maths::Min(0.5f, pathThroughput.Luminance());
			// float continueProbability = Maths::Min(0.5f, 0.33f * (pathThroughput[0] + pathThroughput[1] + pathThroughput[2]));

			if (p_pScene->GetSampler()->Get1DSample() > continueProbability)
				break;

			pathThroughput /= continueProbability;
		}

		//----------------------------------------------------------------------------------------------
		// Check intersection for next round
		//----------------------------------------------------------------------------------------------
		if(!p_pScene->Intersects(ray, p_intersection))
		{
			if (specularBounce) 
			{
				for (size_t lightIndex = 0; lightIndex < p_pScene->LightList.Size(); ++lightIndex)
					p_pRadianceContext->Indirect += pathThroughput * p_pScene->LightList[lightIndex]->Radiance(-ray);
			}

			break;
		}
		*/
	}

	// Populate radiance context
	p_pRadianceContext->Flags |= RadianceContext::DF_Albedo | 
		RadianceContext::DF_Direct | RadianceContext::DF_Indirect;
	
	return //p_pRadianceContext->Indirect;
		p_pRadianceContext->Direct + p_pRadianceContext->Indirect;
}
//----------------------------------------------------------------------------------------------
Spectrum PathIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, RadianceContext *p_pRadianceContext)
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