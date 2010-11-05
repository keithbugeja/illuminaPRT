//----------------------------------------------------------------------------------------------
//	Filename:	Camera.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Implementation for Rays in R3
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>

#include "Geometry/Vector2.h"
#include "Geometry/Vector3.h"
#include "Geometry/Basis.h"
#include "Geometry/Ray.h"

namespace Illumina 
{
	namespace Core
	{
		class ICamera
		{
		protected:
			OrthonormalBasis m_uvw;
			Vector3 m_centre, m_corner, m_across, m_up;

			float m_d;
			float m_u0, m_u1, m_v0, m_v1;

		public:
			void SetProjection(float p_fLeft, float p_fRight, float p_fTop, float p_fBottom, float p_fDistance);

			void Move(const Vector3 &p_displacement);
			void MoveTo(const Vector3 &p_position);

			void Look(const Vector3 &p_direction, const Vector3 &p_up);
			void LookAt(const Vector3 &p_target);

		public:
			virtual Ray GetRay(float p_fPixelX, float p_fPixelY, float p_fXi1, float p_fXi2) const = 0;
			virtual void GetRay(float p_fPixelX, float p_fPixelY, float p_fXi1, float p_fXi2, Ray &p_ray) const = 0;
			virtual std::string ToString(void) const { return "Camera"; }
		};
	} 
}