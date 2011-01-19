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
#include "Shape/IndexedTriangle.h"
#include "Geometry/BoundingBox.h"
#include "Threading/List.h"
#include "Maths/Random.h"

namespace Illumina 
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		// This class is no longer templated.
		// Format is now fixed at using IndexedTriangles 
		// and Vertex type vertices
		//----------------------------------------------------------------------------------------------
		class ITriangleMesh 
			: public IShape
		{
		protected:
			// Triangle mesh area
			float m_fArea;

			// Model axis-aligned bounding box
			AxisAlignedBoundingBox m_boundingBox;

			// Uniform random number generator for sampling
			Random m_random;

		public:
			// Triangle and vertex lists
			List<IndexedTriangle> TriangleList;
			List<Vertex> VertexList;

		public:
			ITriangleMesh(void);
			ITriangleMesh(const std::string &p_strName);

			// Methods for instance creation
			virtual boost::shared_ptr<ITriangleMesh> CreateInstance(void) = 0;

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
			size_t AddVertex(const Vertex &p_vertex);
			void AddVertexList(const Vertex *p_pVertex, int p_nCount);
			void AddVertexList(const List<Vertex> &p_vertexList);

			// Triangle face management
			void AddTriangle(const Vertex &p_v1, const Vertex &p_v2, const Vertex &p_v3, int p_nGroupId = -1);
			void AddTriangleList(const Vertex *p_pVertexList, int p_nTriangleCount, int p_nGroupId = -1);
			void AddTriangleList(const List<Vertex> &p_vertexList, int p_nGroupId = -1);

			// Indexed triangles
			void AddIndexedTriangle(int p_v1, int p_v2, int p_v3, int p_nGroupId = -1);
			void AddIndexedTriangleList(const int *p_pIndexList, int p_nTriangleCount, int p_nGroupId = -1);
			void AddIndexedTriangleList(const List<int> &p_indexList, int p_nGroupId = -1);

			// Compile method used to prepare complex structures for usage (e.g., BVHs)
			bool IsCompilationRequired(void) const { return true; }
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