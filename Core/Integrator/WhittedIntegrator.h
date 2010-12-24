//----------------------------------------------------------------------------------------------
//	Filename:	WhittedIntegrator.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Integrator/Integrator.h"
#include "Maths/Random.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class WhittedIntegrator : public IIntegrator
		{
		protected:
			bool m_bShowNormals;

			int m_nMaxRayDepth,
				m_nShadowSampleCount;

			Random m_random;

		public:
			WhittedIntegrator(int p_nMaxRayDepth = 2, int p_nShadowSampleCount = 1, bool p_bShowNormals = false);

			bool Initialise(Scene *p_pScene, ICamera *p_pCamera);
			bool Shutdown(void);

			Spectrum Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection);
		
		protected:
			Spectrum Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, int p_nDepth);
		};
	}
}