//----------------------------------------------------------------------------------------------
//	Filename:	IndexedTriangle.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/shared_ptr.hpp>

#include "System/IlluminaPRT.h"

#include "Shape/Shape.h"
#include "Shape/VertexFormats.h"
#include "Geometry/BoundingBox.h"

namespace Illumina 
{
	namespace Core
	{
		class IndexedTriangle 
			: public IShape
		{
		protected:
			ITriangleMesh *m_pMesh;
			int m_nVertexID[3];
			int m_nGroupID;

			Vector3 m_edge[2];

			AxisAlignedBoundingBox m_boundingBox;

		public:
			inline int GetVertexIndex(int p_nVertex) { return m_nVertexID[p_nVertex]; }
			inline const Vector3& GetEdge(int p_nEdge) const { return m_edge[p_nEdge]; }
			inline const float* GetEdgeP(int p_nEdge) const { return m_edge[p_nEdge].Element; }

		public:
			IndexedTriangle(ITriangleMesh *p_pMesh, int p_nV1, int p_nV2, int p_nV3, int p_nGroupId = -1);
			IndexedTriangle(const IndexedTriangle &p_triangle);
			
			bool IsBounded(void) const;
			void ComputeBoundingVolume(void);
			IBoundingVolume* GetBoundingVolume(void) const;
			
			bool HasGroup(void) const;
			int GetGroupId(void) const;

			bool Intersects(const Ray &p_ray, DifferentialSurface &p_surface);
			bool Intersects(const Ray &p_ray);
			
			float GetArea(void) const;
			float GetPdf(const Vector3 &p_point) const;

			Vector3 SamplePoint(float p_u, float p_v, Vector3 &p_normal);

			IndexedTriangle& operator=(const IndexedTriangle& p_indexedTriangle);
		};
	} 
}