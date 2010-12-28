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
#include "Spectrum/Spectrum.h"
#include "Sampler/Sampler.h"
#include "Staging/Visibility.h"
#include "Staging/Scene.h"
#include "Material/BSDF.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
Spectrum IIntegrator::EstimateDirectLighting(Scene *p_pScene, ILight *p_pLight, BSDF *p_pBSDF,
	const Intersection &p_intersection, const Vector3 &p_point, const Vector3 &p_normal, 
	const Vector3 &p_wOut, Vector3 &p_wIn, float p_u, float p_v)
{ 
	VisibilityQuery visibilityQuery(p_pScene);

	Spectrum Ls = p_pLight->SampleRadiance(p_point, p_u, p_v, p_wIn, visibilityQuery);
				
	if (Ls.IsBlack() || visibilityQuery.IsOccluded())
		return 0.0f;
	
	if (p_pBSDF == NULL)
		return Ls * Maths::Max(0, Vector3::AbsDot(p_wIn, p_normal));

	Vector3 bsdfIn, bsdfOut;

	BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, p_wOut, bsdfOut);
	BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, p_wIn, bsdfIn);

	return Ls * Vector3::AbsDot(p_wIn, p_normal) * p_pBSDF->F(bsdfOut, bsdfIn);
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

	BSDF *pBSDF = p_intersection.HasMaterial() 
		? (BSDF*)p_intersection.GetMaterial()
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