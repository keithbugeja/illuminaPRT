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
IIntegrator::IIntegrator(void) 
{ }
//----------------------------------------------------------------------------------------------
IIntegrator::IIntegrator(const std::string &p_strId)
	: Object(p_strId)
{ }
//----------------------------------------------------------------------------------------------
Spectrum IIntegrator::EstimateDirectLighting(Scene *p_pScene, ILight *p_pLight, IMaterial *p_pMaterial,
	const Intersection &p_intersection, const Vector3 &p_point, const Vector3 &p_normal, 
	const Vector3 &p_wOut, Vector3 &p_wIn, float p_u, float p_v)
{ 
	VisibilityQuery visibilityQuery(p_pScene);

	Spectrum Ls = p_pLight->SampleRadiance(p_point, p_u, p_v, p_wIn, visibilityQuery);
				
	if (Ls.IsBlack() || visibilityQuery.IsOccluded())
		return 0.0f;

	p_wIn = -p_wIn;

	if (p_pMaterial == NULL)
		return Ls * Maths::Max(0, Vector3::Dot(p_wIn, p_normal));
		//return Ls * Maths::Max(0, Vector3::AbsDot(p_wIn, p_normal));

	Vector3 bsdfIn, bsdfOut;

	BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, p_wOut, bsdfOut);
	BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, p_wIn, bsdfIn);

	return Ls * Maths::Max(0, Vector3::Dot(p_wIn, p_normal)) * p_pMaterial->F(p_intersection.Surface, bsdfOut, bsdfIn);
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

			Ls += Le / p_nSampleCount;
		}
	}

	delete[] lightSample;

	return Ls;
}