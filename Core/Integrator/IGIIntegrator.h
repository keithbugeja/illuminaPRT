//----------------------------------------------------------------------------------------------
//	Filename:	IGIIntegrator.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Integrator/Integrator.h"
#include "Geometry/Intersection.h"
#include "Maths/Random.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		struct VirtualPointLight
		{
			Intersection SurfaceIntersection;

			Vector3 Position;
			Vector3 Direction;
			Vector3 Normal;

			Spectrum Power;
		};

		class IGIIntegrator : 
			public IIntegrator
		{
		protected:
			int m_nMaxVPL,
				m_nMaxRayDepth,
				m_nShadowSampleCount;

			float m_fReflectEpsilon;

			Random m_random;

		public:
			std::vector<VirtualPointLight> VirtualPointLightList;

		public:
			IGIIntegrator(const std::string &p_strName, int p_nMaxVPL = 256, int p_nMaxRayDepth = 4, int p_nShadowSampleCount = 1, float p_fReflectEpsilon = 1E-1f);
			IGIIntegrator(int p_nMaxVPL = 256, int p_nMaxRayDepth = 4, int p_nShadowSampleCount = 1, float p_fReflectEpsilon = 1E-1f);

			bool Initialise(Scene *p_pScene, ICamera *p_pCamera);
			bool Shutdown(void);

			bool Prepare(Scene *p_pScene);

			Spectrum Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection);
		
		protected:
			Spectrum Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, int p_nDepth);

			void TraceVPLs(Scene *p_pScene, int p_nLightIdx, int p_nVPLCount, std::vector<VirtualPointLight> &p_vplList);
		};
	}
}