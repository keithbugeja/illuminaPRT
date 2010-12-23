//----------------------------------------------------------------------------------------------
//	Filename:	IndexedTriangle.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/shared_ptr.hpp>

#include "Shape/VertexFormats.h"
#include "Shape/TriangleMesh.h"

namespace Illumina 
{
	namespace Core
	{
		template <class TVertex> class IndexedTriangle 
			: public IShape
		{
		protected:
			ITriangleMesh<IndexedTriangle, TVertex> *m_pMesh;
			int m_nVertexID[3];

			AxisAlignedBoundingBox m_boundingBox;
		public:
			int GetVertexIndex(int p_nVertex) { return m_nVertexID[p_nVertex]; }

		public:
			IndexedTriangle(const ITriangleMesh<IndexedTriangle, TVertex> *p_pMesh, int p_nV1, int p_nV2, int p_nV3);
			IndexedTriangle(const IndexedTriangle &p_triangle);
			
			bool IsBounded(void) const;
			void ComputeBoundingVolume(void);
			IBoundingVolume* GetBoundingVolume(void) const;

			bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface);
			bool Intersects(const Ray &p_ray, float p_fTime);
			
			float GetArea(void) const;
			float GetPdf(const Vector3 &p_point) const;
			Vector3 SamplePoint(float p_u, float p_v, Vector3 &p_normal) const;

			IndexedTriangle<TVertex>& operator=(const IndexedTriangle<TVertex>& p_indexedTriangle);
		};
	} 
}

#include "IndexedTriangle.inl"