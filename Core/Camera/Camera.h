//----------------------------------------------------------------------------------------------
//	Filename:	Camera.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "System/FactoryManager.h"

#include "Object/Object.h"

#include "Geometry/Vector3.h"
#include "Geometry/Basis.h"
#include "Geometry/Ray.h"

//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		// ICamera : Abstract base class for camera models. 
		//----------------------------------------------------------------------------------------------
		class ICamera 
			: public Object
		{
		protected:
			OrthonormalBasis m_uvw;
			Vector3 m_centre, m_corner, m_across, m_up;

			float m_d;
			float m_u0, m_u1, m_v0, m_v1;

			float m_fov,
				m_aspect;			

		protected:
			ICamera(const std::string& p_strName);
			ICamera(void);

		public:
			void SetProjection(float p_fLeft, float p_fRight, float p_fTop, float p_fBottom, float p_fDistance);
			void SetFieldOfView(float p_fDegrees, float p_fAspectRatio);

			void Move(const Vector3 &p_displacement);
			void MoveTo(const Vector3 &p_position);

			void Look(const Vector3 &p_direction, const Vector3 &p_up);
			void LookAt(const Vector3 &p_target);

		public:
			virtual void GetRay(float p_fPixelX, float p_fPixelY, float p_fXi1, float p_fXi2, Ray &p_ray) const = 0;
			virtual Ray GetRay(float p_fPixelX, float p_fPixelY, float p_fXi1, float p_fXi2) const = 0;
			virtual std::string ToString(void) const { return "ICamera"; }
		};

		//----------------------------------------------------------------------------------------------
		// CameraManager : All Camera model factories must register with the CameraManager object
		//----------------------------------------------------------------------------------------------
		typedef FactoryManager<ICamera> CameraManager;
	} 
}