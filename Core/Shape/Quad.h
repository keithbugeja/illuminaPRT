//----------------------------------------------------------------------------------------------
//	Filename:	Quad.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Description goes here...
//----------------------------------------------------------------------------------------------
#pragma once

#include "Shape/Shape.h"
#include "Shape/Triangle.h"
#include "Geometry/BoundingBox.h"

namespace Illumina 
{
	namespace Core
	{
		class Quad 
			: public IShape
		{
		protected:
			AxisAlignedBoundingBox	m_boundingBox;
			Triangle m_tesselatedQuad[2];

		public:
			Quad(const std::string &p_strName, 
				const Vector3 &p_vertex1, const Vector3 &p_vertex2, 
				const Vector3 &p_vertex3, const Vector3 &p_vertex4);
			Quad(const std::string &p_strName, 
				const Vector3 &p_vertex1, const Vector3 &p_vertex2, 
				const Vector3 &p_vertex3, const Vector3 &p_vertex4, 
				const Vector2 &p_uv1, const Vector2 &p_uv2, 
				const Vector2 &p_uv3, const Vector2 &p_uv4);
			Quad(const Vector3 &p_vertex1, const Vector3 &p_vertex2, 
				const Vector3 &p_vertex3, const Vector3 &p_vertex4);
			Quad(const Vector3 &p_vertex1, const Vector3 &p_vertex2, 
				const Vector3 &p_vertex3, const Vector3 &p_vertex4, 
				const Vector2 &p_uv1, const Vector2 &p_uv2, 
				const Vector2 &p_uv3, const Vector2 &p_uv4);
			Quad(const Quad &p_quad);
			
			bool IsBounded(void) const;
			void ComputeBoundingVolume(void);
			IBoundingVolume* GetBoundingVolume(void) const;

			bool Intersects(const Ray &p_ray, DifferentialSurface &p_surface);
			bool Intersects(const Ray &p_ray);

			float GetArea(void) const;
			float GetPdf(const Vector3 &p_point) const;

			Vector3 SamplePoint(float p_u, float p_v, Vector3 &p_normal);
		};
	} 
}