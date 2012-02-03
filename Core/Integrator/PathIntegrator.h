//----------------------------------------------------------------------------------------------
//	Filename:	PathIntegrator.h
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
		class PathIntegrator : 
			public IIntegrator
		{
		protected:
			int m_nMaxRayDepth,
				m_nShadowSampleCount;

			float m_fReflectEpsilon;

			Random m_random;

		public:
			PathIntegrator(const std::string &p_strName, int p_nMaxRayDepth = 4, int p_nShadowSampleCount = 16, float p_fReflectEpsilon = 1E-1f);
			PathIntegrator(int p_nMaxRayDepth = 4, int p_nShadowSampleCount = 1, float p_fReflectEpsilon = 1E-1f);

			bool Initialise(Scene *p_pScene, ICamera *p_pCamera);
			bool Shutdown(void);

			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection);
			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, Intersection &p_intersection);
		};
	}
}