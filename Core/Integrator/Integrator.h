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

			static Spectrum EstimateDirectLighting(Scene *p_pScene, ILight *p_pLight, 
				const Vector3 &p_point, const Vector3 &p_normal, float p_u, float p_v, Vector3 &p_wOut, int p_nShadowSamples = 1);

			static Spectrum SampleAllLights(Scene *p_pScene, const Vector3 &p_point, const Vector3 &p_pNormal, 
				ISampler* p_pSampler, int p_nShadowSamples = 1);

			//static Vector3 SampleHemisphere(const Transformation &p_transform, float p_fU, float p_fV);
		};
	}
}