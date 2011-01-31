//----------------------------------------------------------------------------------------------
//	Filename:	Integrator.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <string>

#include "System/IlluminaPRT.h"
#include "System/FactoryManager.h"

#include "Object/Object.h"

#include "Geometry/Vector2.h"
#include "Geometry/Vector3.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		// IIntegrator : Abstract base class for transport integrator methods. 
		//----------------------------------------------------------------------------------------------
		class IIntegrator 
			: public Object
		{
		protected:
			IIntegrator(const std::string &p_strName) : Object(p_strName) { }
			IIntegrator(void) { }

		public:
			virtual bool Initialise(Scene *p_pScene, ICamera *p_pCamera) = 0;
			virtual bool Shutdown(void) = 0;

			virtual Spectrum Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection) = 0;

			static Spectrum EstimateDirectLighting(Scene *p_pScene, ILight *p_pLight, IMaterial *p_pMaterial, 
				const Intersection &p_intersection, const Vector3 &p_point, const Vector3 &p_normal, const Vector3 &p_wOut, 
				Vector3 &p_wIn, float p_u, float p_v);

			static Spectrum SampleAllLights(Scene *p_pScene, const Intersection &p_intersection,
				const Vector3 &p_point, const Vector3 &p_pNormal, const Vector3 &p_wOut,
				ISampler *p_pSampler, int p_nShadowSamples = 1);

			static Spectrum SampleAllLights(Scene *p_pScene, const Intersection &p_intersection,
				const Vector3 &p_point, const Vector3 &p_pNormal, const Vector3 &p_wOut,
				ISampler *p_pSampler, ILight *p_pExclude = NULL, int p_nShadowSamples = 1);
		};

		//----------------------------------------------------------------------------------------------
		// IntegratorManager : All Integrator factories must register with object.
		//----------------------------------------------------------------------------------------------
		typedef FactoryManager<IIntegrator> IntegratorManager;
	}
}