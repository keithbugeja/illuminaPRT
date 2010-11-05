//----------------------------------------------------------------------------------------------
//	Filename:	Camera.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Implementation for Rays in R3
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>
#include "Camera/Camera.h"

namespace Illumina 
{
	namespace Core
	{
		class ThinLensCamera : 
			public ICamera
		{
		protected:
			float m_fLensRadius;

		public:
			ThinLensCamera(const Vector3 &p_centre, const Vector3 &p_direction, const Vector3 &p_up, 
				float p_fAperture, float p_fLeft, float p_fRight, float p_fBottom, float p_fTop, float p_fDistance);

			float GetAperture(void) const;
			void SetAperture(float p_fAperture);

			Ray GetRay(float p_fPixelX, float p_fPixelY, float p_fXi1, float p_fXi2) const; 
			void GetRay(float p_fPixelX, float p_fPixelY, float p_fXi1, float p_fXi2, Ray &p_ray) const; 
		};
	} 
}