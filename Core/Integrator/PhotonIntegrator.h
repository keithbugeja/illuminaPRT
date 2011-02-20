//----------------------------------------------------------------------------------------------
//	Filename:	PhotonIntegrator.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Integrator/Integrator.h"
#include "Maths/Random.h"

#include "Staging/Acceleration.h"

//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		struct Photon
		{
			Vector3	Position,
				Direction;

			Spectrum Power;

			AxisAlignedBoundingBox aabb;
	
			IBoundingVolume* GetBoundingVolume(void) {
				aabb.SetExtents(Position, Position);
				return &aabb;
			}
		};
		//----------------------------------------------------------------------------------------------

		class PhotonIntegrator : 
			public IIntegrator
		{
		protected:
			int m_nMaxPhotonCount;

			int m_nMaxRayDepth,
				m_nShadowSampleCount;

			float m_fReflectEpsilon;

			Random m_random;

			List<Photon> m_photonList;
			KDTree<Photon> m_photonMap;

		public:
			PhotonIntegrator(const std::string &p_strName, int p_nMaxPhotonCount = 100000, int p_nMaxRayDepth = 4, int p_nShadowSampleCount = 16, float p_fReflectEpsilon = 1E-1f);
			PhotonIntegrator(int p_nMaxPhotonCount = 100000, int p_nMaxRayDepth = 4, int p_nShadowSampleCount = 1, float p_fReflectEpsilon = 1E-1f);

			bool Initialise(Scene *p_pScene, ICamera *p_pCamera);
			bool Shutdown(void);

			Spectrum Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection);
		
		protected:
			Spectrum Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, int p_nDepth);
		};
	}
}