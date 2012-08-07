//----------------------------------------------------------------------------------------------
//	Filename:	PersistentMesh.h
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
		struct PersistentIndexedTriangle
		{
			int VertexID[3];
			int GroupID;

			Vector3 Edge[2];
		};

		struct PersistentTreeNode
		{
			float Partition;

			unsigned int Axis : 2;
			unsigned int ItemCount : 30;
		};

		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------
		class PersistentMesh 
			: public IShape
		{
		protected:
			// Mesh area
			float m_fArea;

			// Model axis-aligned bounding box
			AxisAlignedBoundingBox m_boundingBox;

			// Uniform random number generator for sampling
			Random m_random;

			// Triangle and vertex lists
			PersistentTreeNode *m_pRootNode;
			PersistentIndexedTriangle *m_pTriangleList;
			Vertex *m_pVertexList;

			int m_nTriangleCount,
				m_nVertexCount;

			// Memory-mapped files
			boost::iostreams::mapped_file_source  m_vertexFile;
			boost::iostreams::mapped_file_source  m_triangleFile;
			boost::iostreams::mapped_file_source  m_treeFile;

		protected:
			void MapFiles(const std::string &p_strTrunkName);
			void UnmapFiles(void);

			bool IntersectP(Ray &p_ray);
			bool IntersectP(Ray &p_ray, DifferentialSurface &p_surface);

			bool IntersectFace(Ray p_ray, PersistentIndexedTriangle* p_pTriangle);
			bool IntersectFace(Ray p_ray, PersistentIndexedTriangle* p_pTriangle, DifferentialSurface &p_surface);

		public:
			PersistentMesh(const std::string &p_strName, const std::string &p_strTrunkName);
			PersistentMesh(const std::string &p_strTrunkName);
			~PersistentMesh();

			// Bounding volume
			bool IsBounded(void) const;
			void ComputeBoundingVolume(void);
			IBoundingVolume* GetBoundingVolume(void) const;

			// Area methods
			void ComputeArea(void);
			float GetArea(void) const;

			// Normals
			bool UpdateNormals(void);

			// Compile method used to prepare complex structures for usage (e.g., BVHs)
			bool IsCompilationRequired(void) const { return true; }
			bool Compile(void) { ComputeBoundingVolume(); ComputeArea(); return true; }

			// Update and rebuild methods for dynamic meshes
			bool Update(void) { return true; }
			bool Rebuild(void) { return true; }

			// Sampling methods
			float GetPdf(const Vector3 &p_point) const;
			Vector3 SamplePoint(float p_u, float p_v, Vector3 &p_normal);
			Vector3 SamplePoint(const Vector3 &p_viewPoint, float p_u, float p_v, Vector3 &p_normal);

			// Intersect methods
			bool Intersects(const Ray &p_ray, DifferentialSurface &p_surface);
			bool Intersects(const Ray &p_ray);

			std::string PersistentMesh::ToString(void) const;
		};		
	} 
}