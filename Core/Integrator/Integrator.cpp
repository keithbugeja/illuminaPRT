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

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
Spectrum IIntegrator::EstimateDirectLighting(Scene *p_pScene, ILight *p_pLight,  
	const Vector3 &p_point, const Vector3 &p_normal, float p_u, float p_v, Vector3 &p_wOut, int p_nShadowSamples)
{ 
	VisibilityQuery visibilityQuery(p_pScene);

	Spectrum Li = p_pLight->Radiance(p_point, p_u, p_v, p_wOut, visibilityQuery);
				
	if (!Li.IsBlack())
	{
		if (!visibilityQuery.IsOccluded())
		{
			return Li * Maths::Max(0, Vector3::Dot(p_wOut, p_normal));
		}
	}

	return 0;
}

//----------------------------------------------------------------------------------------------
//Vector3 IIntegrator::SampleHemisphere(const Transformation p_transform, float p_fU, float p_fV)
//{
//	Vector2 spherical(p_fU * Maths::PiTwo, p_fV * Maths::PiHalf);
//	return p_transform.Apply(OrthonormalBasis::FromSpherical(spherical));
//}
//----------------------------------------------------------------------------------------------
Spectrum IIntegrator::SampleAllLights(Scene *p_pScene, const Vector3 &p_point, 
	const Vector3 &p_normal, ISampler* p_pSampler, int p_nSampleCount)
{
	return SampleAllLights(p_pScene, p_point, p_normal, p_pSampler, NULL, p_nSampleCount);
}
//----------------------------------------------------------------------------------------------
Spectrum IIntegrator::SampleAllLights(Scene *p_pScene, const Vector3 &p_point, 
	const Vector3 &p_normal, ISampler *p_pSampler, ILight *p_pExclude, int p_nSampleCount)
{
	Vector2 sample;
	Vector3 wOut;
	Spectrum L(0);

	for (int lightIdx = 0, lightCount = p_pScene->LightList.Size(); lightIdx < lightCount; lightIdx++)
	{
		if (p_pScene->LightList[lightIdx] == p_pExclude) continue;

		Spectrum Ld(0);

		for (int sampleIdx = 0; sampleIdx < p_nSampleCount; sampleIdx++)
		{
			p_pSampler->Get2DSamples(&sample, 1);
			Ld += EstimateDirectLighting(p_pScene, p_pScene->LightList[lightIdx], p_point, p_normal, sample.U, sample.V, wOut, p_nSampleCount);
		}

		L += Ld / p_nSampleCount;
	}

	return L;
}