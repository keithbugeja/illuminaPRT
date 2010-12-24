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
	//Vector3 wIn;

	VisibilityQuery visibilityQuery(p_pScene);

	Spectrum Li = p_pLight->Radiance(p_point, p_u, p_v, p_wOut, visibilityQuery);
				
	//if (!Li.IsBlack())
	if (!Li.IsBlack() && !visibilityQuery.IsOccluded())
	{
		return Li * Maths::Max(0, Vector3::Dot(p_wOut, p_normal));
	}

	return 0;
}

/*
public Spectrum EstimateDirectLighting(ILight light, Vector3 p_point, 
	Vector3 p_normal, Vector3 p_wOut, BSDF p_bsdf, int p_bxdfIndex)
{
	Sample1DDistribution samples = new Sample1DDistribution();
	m_scene.Sampler.GetSampleDistribution(samples, 5);

	VisibilityQuery visibilityQuery = new VisibilityQuery(m_scene);
	Vector3 wIn = new Vector3();
			
	Spectrum Li = light.Radiance(p_point, samples[0], samples[1], ref wIn, ref visibilityQuery),
		Ld = Spectrum.Zero;

	if (!Li.IsZero())
	{
		Spectrum f = p_bsdf.F(p_wOut, wIn);

		if (!f.IsZero() && !visibilityQuery.IsOccluded())
			Ld += f * Li * Vector3.AbsDot(wIn, p_normal);
	}

	// TODO: Add importance sampling

	return Ld;
}
*/
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