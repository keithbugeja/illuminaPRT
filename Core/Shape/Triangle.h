//----------------------------------------------------------------------------------------------
//	Filename:	Triangle.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Description goes here...
//----------------------------------------------------------------------------------------------
#pragma once

#include "Shape/Shape.h"
#include "Geometry/BoundingBox.h"

namespace Illumina 
{
	namespace Core
	{
		class Triangle : public IShape
		{
		protected:
			AxisAlignedBoundingBox	m_boundingBox;

		public:
			Vector3 Vertex[3];
			Vector2 UV[3];

		public:
			Triangle(const Vector3 &p_vertex1, 
				const Vector3 &p_vertex2, const Vector3 &p_vertex3);
			Triangle(const Vector3 &p_vertex1, const Vector3 &p_vertex2, const Vector3 &p_vertex3,
				const Vector2 &p_uv1, const Vector2 &p_uv2, const Vector2 &p_uv3);
			Triangle(const Triangle &p_triangle);
			
			bool IsBounded(void) const;
			void ComputeBoundingVolume(void);
			IBoundingVolume* GetBoundingVolume(void) const;

			bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface);
			bool Intersects(const Ray &p_ray, float p_fTime);

			float GetArea(void) const;
			float GetPdf(const Vector3 &p_point) const;
			Vector3 SamplePoint(float p_u, float p_v, Vector3 &p_normal) const;
		};
	} 
}