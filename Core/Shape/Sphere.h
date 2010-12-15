//----------------------------------------------------------------------------------------------
//	Filename:	SphereUV.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Shape/Shape.h"
#include "Geometry/BoundingBox.h"

namespace Illumina 
{
	namespace Core
	{
		class Sphere : 
			public IShape
		{
		public:
			AxisAlignedBoundingBox m_boundingBox;

			Vector3 Centre;
			float Radius;

		public:
			Sphere(const Vector3 &p_centre, float p_fRadius);
			Sphere(const Sphere &p_sphere); 

			bool IsBounded(void) const;
			void ComputeBoundingVolume(void);
			IBoundingVolume* GetBoundingVolume(void) const;

			bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface);
			bool Intersects(const Ray &p_ray, float p_fTime);
		};
	} 
}