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
		class WhittedIntegrator 
			: public IIntegrator
		{
		protected:
			int m_nMaxRayDepth,
				m_nShadowSampleCount;

			bool m_bEnableShadowRays;

		public:
			WhittedIntegrator(const std::string &p_strName, int p_nMaxRayDepth = 4, int p_nShadowSampleCount = 16, bool p_bEnableShadowRays = true);
			WhittedIntegrator(int p_nMaxRayDepth = 4, int p_nShadowSampleCount = 16, bool p_bEnableShadowRays = true);

			bool Initialise(Scene *p_pScene, ICamera *p_pCamera);
			bool Shutdown(void);

			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, RadianceContext *p_pRadianceContext = NULL);
			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, Intersection &p_intersection, RadianceContext *p_pRadianceContext = NULL);		
		
		protected:
			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, int p_nRayDepth);
		};
	}
}