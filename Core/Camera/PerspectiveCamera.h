//----------------------------------------------------------------------------------------------
//	Filename:	Camera.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Camera/Camera.h"

namespace Illumina 
{
	namespace Core
	{
		class PerspectiveCamera : 
			public ICamera
		{
		public:
			PerspectiveCamera(const Vector3 &p_centre, const Vector3 &p_direction, const Vector3 &p_up, 
				float p_fLeft, float p_fRight, float p_fBottom, float p_fTop, float p_fDistance);
			
			PerspectiveCamera(const std::string &p_strName, const Vector3 &p_centre, const Vector3 &p_direction, const Vector3 &p_up, 
				float p_fLeft, float p_fRight, float p_fBottom, float p_fTop, float p_fDistance);

			Ray GetRay(float p_fPixelX, float p_fPixelY, float p_fXi1, float p_fXi2) const; 
			void GetRay(float p_fPixelX, float p_fPixelY, float p_fXi1, float p_fXi2, Ray &p_ray) const;
		};
	} 
}