//----------------------------------------------------------------------------------------------
//	Filename:	Integrator.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iostream>
#include "Integrator/Integrator.h"

#include "Geometry/Ray.h"
#include "Geometry/Intersection.h"
#include "Material/Material.h"
#include "Spectrum/Spectrum.h"
#include "Scene/Visibility.h"
#include "Sampler/Sampler.h"
#include "Scene/Scene.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
Spectrum IIntegrator::Direct(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, 
	Intersection &p_intersection, RadianceContext *p_pRadianceContext)
{
	// Compute intersection step
	if (!p_pScene->Intersects(Ray(p_ray), p_intersection))
	{
		p_intersection.Surface.RayOriginWS = p_ray.Origin;
		p_intersection.Surface.RayDirectionWS = p_ray.Direction;
	}

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

		p_pRadianceContext->Flags |= RadianceContext::DF_Direct;

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
		IMaterial *pMaterial = p_intersection.GetMaterial();
	
		//----------------------------------------------------------------------------------------------
		// Sample lights for specular / first bounce
		//----------------------------------------------------------------------------------------------
		Vector3 wOut = -Vector3::Normalize(ray.Direction);
	
		//----------------------------------------------------------------------------------------------
		// Additional logic for first hit
		//----------------------------------------------------------------------------------------------
		// Set albedo for first hit
		p_pRadianceContext->Albedo = pMaterial->Rho(wOut, p_intersection.Surface);

		// If an emissive material has been encountered, add contribution and terminate path
		if (p_intersection.IsEmissive())
		{
			p_pRadianceContext->Direct = 
				p_intersection.GetLight()->Radiance(p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, wOut); 

			p_pRadianceContext->Flags |= RadianceContext::DF_Albedo | RadianceContext::DF_Direct;

			return p_pRadianceContext->Direct;
		}

		//----------------------------------------------------------------------------------------------
		// Sample lights for direct lighting
		//----------------------------------------------------------------------------------------------
		if (pMaterial->HasBxDFType(BxDF::Reflection, false))
		{
			p_pRadianceContext->Direct = 
				SampleAllLights(p_pScene, p_intersection, 
				p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut, 
				p_pScene->GetSampler(), p_intersection.GetLight(), 1);
		}
	}

	// Return radiance
	p_pRadianceContext->Flags |= RadianceContext::DF_Albedo | RadianceContext::DF_Direct;

	return p_pRadianceContext->Direct;
}
//----------------------------------------------------------------------------------------------
Spectrum IIntegrator::EstimateDirectLighting(Scene *p_pScene, ILight *p_pLight, IMaterial *p_pMaterial,
	const Intersection &p_intersection, const Vector3 &p_point, const Vector3 &p_normal, 
	const Vector3 &p_wOut, Vector3 &p_wIn, float p_u, float p_v)
{ 
	float pdf;

	VisibilityQuery visibilityQuery(p_pScene);

	Spectrum Ls = p_pLight->SampleRadiance(p_point, p_normal, p_u, p_v, p_wIn, pdf, visibilityQuery);
				
	if (Ls.IsBlack() || visibilityQuery.IsOccluded())
		return 0.0f;

	p_wIn = -p_wIn;

	if (p_pMaterial == NULL)
		return Ls * Maths::Max(0.f, Vector3::Dot(p_wIn, p_normal));
		//return Ls * Maths::Max(0, Vector3::AbsDot(p_wIn, p_normal));

	Vector3 bsdfIn, bsdfOut;

	BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, p_wOut, bsdfOut);
	BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, p_wIn, bsdfIn);

	return Ls * Maths::Max(0.f, Vector3::Dot(p_wIn, p_normal)) * p_pMaterial->F(p_intersection.Surface, bsdfOut, bsdfIn);
	//return Ls * Maths::Max(0, Vector3::AbsDot(p_wIn, p_normal)) * p_pMaterial->F(p_intersection.Surface, bsdfOut, bsdfIn);
}
//----------------------------------------------------------------------------------------------
Spectrum IIntegrator::SampleAllLights(Scene *p_pScene, const Intersection &p_intersection,
	const Vector3 &p_point, const Vector3 &p_normal, const Vector3 &p_wOut, 
	ISampler *p_pSampler, int p_nSampleCount)
{
	return SampleAllLights(p_pScene, p_intersection, p_point, p_normal, p_wOut, p_pSampler, NULL, p_nSampleCount);
}
//----------------------------------------------------------------------------------------------
Spectrum IIntegrator::SampleAllLights(Scene *p_pScene, const Intersection &p_intersection, 
	const Vector3 &p_point, const Vector3 &p_normal, const Vector3 &p_wOut, 
	ISampler *p_pSampler, ILight *p_pExclude, int p_nSampleCount)
{
	Spectrum Ls(0);

	Vector3 wIn;
	Vector2 *lightSample = new Vector2[p_nSampleCount];

	IMaterial *pBSDF = p_intersection.HasMaterial() 
		? p_intersection.GetMaterial()
		: NULL;

	// Sample all lights in scene
	for (int lightIdx = 0, lightCount = p_pScene->LightList.Size(); lightIdx < lightCount; lightIdx++)
	{
		// If light is excluded, skip
		if (p_pScene->LightList[lightIdx] != p_pExclude) 
		{
			Spectrum Le(0);
			p_pSampler->Get2DSamples(lightSample, p_nSampleCount);

			// Sample same light a number of times, for a better estimate
			for (int sampleIdx = 0; sampleIdx < p_nSampleCount; sampleIdx++)
			{
				Le += EstimateDirectLighting(p_pScene, p_pScene->LightList[lightIdx], pBSDF, p_intersection,
					p_point, p_normal, p_wOut, wIn, lightSample[sampleIdx].U, lightSample[sampleIdx].V);
			}

			Ls += Le;
		}
	}

	delete[] lightSample;

	return Ls / p_nSampleCount;
}

//----------------------------------------------------------------------------------------------
Spectrum IIntegrator::SampleF(Scene *p_pScene, Intersection &p_intersection, const Vector3 &p_wOut, Vector3 &p_wIn, float &p_pdf, BxDF::Type &p_bxdfType)
{
	if (!p_intersection.HasMaterial())
		return 0.f;

	IMaterial *pMaterial = p_intersection.GetMaterial();

	Vector3 wOutLocal, 
		wInLocal;

	//----------------------------------------------------------------------------------------------
	// Sample bsdf for next direction
	//----------------------------------------------------------------------------------------------
	// Generate random samples
	Vector2 sample = p_pScene->GetSampler()->Get2DSample();

	// Convert to surface coordinate system where (0,0,1) represents surface normal
	// Note: 
	// -- All Material/BSDF/BxDF operations are carried out in surface coordinates
	// -- All inputs must be in surface coordinates
	// -- All outputs are in surface coordinates
	BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, p_wOut, wOutLocal);

	// Sample new direction in wIn (remember we're tracing backwards)
	// -- wIn returns the sampled direction
	// -- pdf returns the reflectivity function's pdf at the sampled point
	// -- bxdfType returns the type of BxDF sampled
	Spectrum f = pMaterial->SampleF(p_intersection.Surface, wOutLocal, wInLocal, sample.U, sample.V, &p_pdf, BxDF::All_Combined, &p_bxdfType);

	// If the reflectivity or pdf are zero, terminate path
	if (f.IsBlack() || p_pdf == 0.0f) return 0.f;

	// Convert back to world coordinates
	BSDF::SurfaceToWorld(p_intersection.WorldTransform, p_intersection.Surface, wInLocal, p_wIn);

	return f;
}
//----------------------------------------------------------------------------------------------
Spectrum IIntegrator::F(Scene *p_pScene, const Intersection &p_intersection, const Vector3 &p_wOut, const Vector3 &p_wIn /*, BxDF::Type &p_bxdfType*/)
{
	IMaterial *pMaterial = p_intersection.GetMaterial();
	
	if (pMaterial == NULL)
		return 0.f;

	Vector3 bsdfIn, bsdfOut;

	BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, p_wOut, bsdfOut);
	BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, p_wIn, bsdfIn);

	return pMaterial->F(p_intersection.Surface, bsdfOut, bsdfIn);
}
