//----------------------------------------------------------------------------------------------
//	Filename:	TestIntegrator.h
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
		class TestIntegrator
		{
		public:
			TestIntegrator(void);
		};

		/*
		 * TODO : Rewrite test integrator!
		class TestIntegrator 
			: public IIntegrator
		{
		protected:
			Random m_random;

		public:
			TestIntegrator(const std::string &p_strName);
			TestIntegrator();

			bool Initialise(Scene *p_pScene, ICamera *p_pCamera);
			bool Shutdown(void);

			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection);
			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, Intersection &p_intersection) {return 0.f;}
		
		protected:
			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, int p_nRayDepth);
		};
		 */
	}
}