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
#include "Staging/Visibility.h"
#include "Staging/Scene.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
Spectrum IIntegrator::EstimateDirectLighting(Scene *p_pScene, ILight *p_pLight, const BSDF &p_bsdf, 
				const Vector3 &p_point, const Vector3 &p_normal, Vector3 &p_wOut)
{ 
	Vector3 wIn;

	VisibilityQuery visibilityQuery(p_pScene);

	Spectrum Li = p_pLight->Radiance(p_point, wIn, visibilityQuery);
				
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
/*
		public Spectrum SampleAllLights(Vector3 p_point, Vector3 p_normal, Vector3 p_wOut, BSDF p_bsdf, 
			Sample1DDistribution p_lightSamples, Sample1DDistribution p_bxdfSamples)
		{
			Spectrum L = Spectrum.Zero;

			for (int lightIdx = 0, lightCount = m_scene.Lights.Count; lightIdx < lightCount; lightIdx++)
			{
				ILight light = m_scene.Lights[lightIdx];
				int sampleCount = (int)p_lightSamples[lightIdx++];

				Spectrum Ld = Spectrum.Zero;

				for (int sampleIdx = 0; sampleIdx < sampleCount; sampleIdx++)
				{
					Ld += EstimateDirectLighting(light, p_point, p_normal, p_wOut, p_bsdf, (int)p_bxdfSamples[lightIdx]);
				}

				L += Ld / sampleCount;
			}

			return L;
		}
		*/