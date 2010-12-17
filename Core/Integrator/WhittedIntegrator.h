//----------------------------------------------------------------------------------------------
//	Filename:	WhittedIntegrator.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Integrator/Integrator.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class WhittedIntegrator : public IIntegrator
		{
		protected:
			int m_nMaxRayDepth,
				m_nShadowSampleCount;

		public:
			WhittedIntegrator(int p_nMaxRayDepth = 3, int p_nShadowSampleCount = 1);

			bool Initialise(Scene *p_pScene, ICamera *p_pCamera);
			bool Shutdown(void);

			Spectrum Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection);
		
		protected:
			Spectrum Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, int p_nDepth);
		};
	}
}