//----------------------------------------------------------------------------------------------
//	Filename:	TriangleMesh.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/shared_ptr.hpp>

#include "Shape/Shape.h"
#include "Shape/VertexFormats.h"
#include "Geometry/BoundingBox.h"
#include "Maths/Montecarlo.h"
#include "Threading/List.h"

namespace Illumina 
{
	namespace Core
	{
		// Note that writes to this class are not thread-safe!
		// T = IndexedTriangle derivative
		// U = VertexBase derivative
		template<class T, class U> class ITriangleMesh
			: public IShape
		{
		protected:
			float m_fArea;
			Random m_random;
			AxisAlignedBoundingBox m_boundingBox;

		public:
			// Triangle array
			List<T> TriangleList;
			List<U> VertexList;
			
			// Methods for instance creation
			virtual boost::shared_ptr<ITriangleMesh<T, U>> CreateInstance(void) = 0;

			// Bounding volume
			bool IsBounded(void) const;
			void ComputeBoundingVolume(void);
			IBoundingVolume* GetBoundingVolume(void) const;

			// Area methods
			void ComputeArea(void);
			float GetArea(void) const;

			// Normals
			bool UpdateNormals(void);

			// Vertex management			
			size_t AddVertex(const U &p_vertex);
			void AddVertexList(const U *p_pVertex, int p_nCount);
			void AddVertexList(const List<U> &p_vertexList);

			// Triangle face management
			void AddTriangle(const U &p_v1, const U &p_v2, const U &p_v3, int p_nGroupId = -1);
			void AddTriangleList(const U *p_pVertexList, int p_nTriangleCount, int p_nGroupId = -1);
			void AddTriangleList(const List<U> &p_uvList, int p_nGroupId = -1);

			// Indexed triangles
			void AddIndexedTriangle(int p_v1, int p_v2, int p_v3, int p_nGroupId = -1);
			void AddIndexedTriangleList(const int *p_pIndexList, int p_nTriangleCount, int p_nGroupId = -1);
			void AddIndexedTriangleList(const List<int> &p_indexList, int p_nGroupId = -1);

			// Compile method used to prepare complex structures for usage (e.g., BVHs)
			bool Compile(void) { return true; }

			// Update and rebuild methods for dynamic meshes
			bool Update(void) { return true; }
			bool Rebuild(void) { return true; }

			// Sampling methods
			float GetPdf(const Vector3 &p_point) const;
			Vector3 SamplePoint(float p_u, float p_v, Vector3 &p_normal);
			Vector3 SamplePoint(const Vector3 &p_viewPoint, float p_u, float p_v, Vector3 &p_normal);

			// Intersect methods
			virtual bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface) = 0;
			virtual bool Intersects(const Ray &p_ray, float p_fTime) = 0;
		};		
	} 
}

#include "TriangleMesh.inl"