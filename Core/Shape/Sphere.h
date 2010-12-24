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

			float GetArea(void) const;
			float GetPdf(const Vector3 &p_point) const;

			Vector3 SamplePoint(float p_u, float p_v, Vector3 &p_normal);
			Vector3 SamplePoint(const Vector3 &p_viewPoint, float p_u, float p_v, Vector3 &p_normal);
		};
	} 
}