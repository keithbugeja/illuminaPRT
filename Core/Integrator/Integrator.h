//----------------------------------------------------------------------------------------------
//	Filename:	Integrator.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class IIntegrator
		{
		public:
			virtual bool Initialise(Scene *p_pScene, ICamera *p_pCamera) = 0;
			virtual bool Shutdown(void) = 0;

			virtual Spectrum Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection) = 0;

			static Spectrum EstimateDirectLighting(Scene *p_pScene, ILight *p_pLight, BSDF *p_pBsdf, 
				const Intersection &p_intersection, const Vector3 &p_point, const Vector3 &p_normal, const Vector3 &p_wOut, 
				Vector3 &p_wIn, float p_u, float p_v);

			static Spectrum SampleAllLights(Scene *p_pScene, const Intersection &p_intersection,
				const Vector3 &p_point, const Vector3 &p_pNormal, const Vector3 &p_wOut,
				ISampler *p_pSampler, int p_nShadowSamples = 1);

			static Spectrum SampleAllLights(Scene *p_pScene, const Intersection &p_intersection,
				const Vector3 &p_point, const Vector3 &p_pNormal, const Vector3 &p_wOut,
				ISampler *p_pSampler, ILight *p_pExclude = NULL, int p_nShadowSamples = 1);

			//static Vector3 SampleHemisphere(const Transformation &p_transform, float p_fU, float p_fV);
		};
	}
}