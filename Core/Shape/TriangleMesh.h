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
#include "Threading/List.h"

namespace Illumina 
{
	namespace Core
	{
		// Note that writes to this class are not thread-safe!
		// T = IndexedTriangle derivative
		// U = VertexBase derivative
		template<class T, class U> class ITriangleMesh
			: public Shape
		{
		protected:
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

			// Vertex management			
			size_t AddVertex(const U &p_vertex);
			void AddVertexList(const U *p_pVertex, int p_nCount);
			void AddVertexList(const List<U> &p_vertexList);

			// Triangle face management
			void AddTriangle(const U &p_v1, const U &p_v2, const U &p_v3);
			void AddTriangleList(const U *p_pVertexList, int p_nTriangleCount);
			void AddTriangleList(const List<U> &p_uvList);

			// Indexed triangles
			void AddIndexedTriangle(int p_v1, int p_v2, int p_v3);
			void AddIndexedTriangleList(const int *p_pIndexList, int p_nTriangleCount);
			void AddIndexedTriangleList(const List<int> &p_indexList);

			// Compile method used to prepare complex structures for usage (e.g., BVHs)
			bool Compile(void) { return true; }

			// Update and rebuild methods for dynamic meshes
			bool Update(void) { return true; }
			bool Rebuild(void) { return true; }

			// Intersect methods
			virtual bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface) = 0;
			virtual bool Intersects(const Ray &p_ray, float p_fTime) = 0;
		};		
	} 
}

#include "TriangleMesh.inl"