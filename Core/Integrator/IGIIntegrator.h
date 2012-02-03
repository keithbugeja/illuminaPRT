//----------------------------------------------------------------------------------------------
//	Filename:	IGIIntegrator.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Integrator/Integrator.h"
#include "Geometry/Intersection.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		struct VirtualPointLight
		{
			Intersection Context;

			//Vector3 Normal;
			//Vector3 Position;
			Vector3 Direction;

			Spectrum Power;

			VirtualPointLight() { }

			VirtualPointLight(const VirtualPointLight &p_vpl)
				: Context(p_vpl.Context)
				, Direction(p_vpl.Direction)
				, Power(p_vpl.Power)
			{ }

			VirtualPointLight& operator=(const VirtualPointLight &p_vpl)
			{
				Context = p_vpl.Context;
				Direction = p_vpl.Direction;
				Power = p_vpl.Power;

				return *this;
			}
		};

		class IGIIntegrator : 
			public IIntegrator
		{
		protected:
			int m_nMaxVPL,
				m_nMaxPath,
				m_nTileWidth,
				m_nTileArea,
				m_nMaxRayDepth,
				m_nShadowSampleCount,
				m_nIndirectSampleCount;

			float m_fReflectEpsilon,
				m_fGTermMax;

		public:
			std::vector<std::vector<VirtualPointLight>> VirtualPointLightSet;
			std::vector<VirtualPointLight> VirtualPointLightList;

		public:
			IGIIntegrator(const std::string &p_strName, int p_nMaxVPL = 256, int p_nMaxPath = 8, int p_nTileWidth = 3, float p_fGTermMax = 0.01f, int p_nMaxRayDepth = 4, int p_nShadowSampleCount = 1, int p_nIndirectSampleCount = 1, float p_fReflectEpsilon = 1E-1f);
			IGIIntegrator(int p_nMaxVPL = 256, int p_nMaxPath = 8, int p_nTileWidth = 3, float p_fGTermMax = 0.01f, int p_nMaxRayDepth = 4, int p_nShadowSampleCount = 1, int p_nIndirectSampleCount = 1, float p_fReflectEpsilon = 1E-1f);

			bool Initialise(Scene *p_pScene, ICamera *p_pCamera);
			bool Shutdown(void);

			bool Prepare(Scene *p_pScene);

			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection);
			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, Intersection &p_intersection);
		
		protected:
			void TraceVirtualPointLights(Scene *p_pScene, int p_nMaxPaths, int p_nMaxPointLights, int p_nMaxBounces, std::vector<VirtualPointLight> &p_virtualPointLightList);
			void TraceVPLs(Scene *p_pScene, int p_nLightIdx, int p_nVPLPaths, int p_nMaxVPLs, int p_nMaxBounces, std::vector<VirtualPointLight> &p_vplList);
		};
	}
}