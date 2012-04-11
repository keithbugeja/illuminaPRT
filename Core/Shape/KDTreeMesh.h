//----------------------------------------------------------------------------------------------
//	Filename:	KDTreeMesh.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//  TODO: 
//		1. Compress the kd-tree node structure
//		2. Provide a better node allocation strategy -> contiguous memory?
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/shared_ptr.hpp>
#include "Shape/TreeMesh.h"

namespace Illumina 
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		// KD-Tree Node
		//----------------------------------------------------------------------------------------------
		// Represents a single node in the kd-tree structure. Note that only leaf nodes in the 
		// structure contain any geometry.
		//----------------------------------------------------------------------------------------------
		struct KDTreeMeshNode
		{
			// Node Type
			ITreeMesh::NodeType Type;

			// Node bounding box
			AxisAlignedBoundingBox BoundingBox;

			// Partition Axis
			int Axis;

			// Partition Point
			float Partition;

			// Only if an internal node
			KDTreeMeshNode *m_pChild[2];

			// Only if a leaf
			List<IndexedTriangle*> TriangleList;

			KDTreeMeshNode() { m_pChild[0] = m_pChild[1] = NULL; }
			~KDTreeMeshNode() { }
		};

		//----------------------------------------------------------------------------------------------
		// KD-Tree Mesh
		//----------------------------------------------------------------------------------------------
		class KDTreeMesh
			: public ITreeMesh
		{
		protected:
			using ITriangleMesh::m_fArea;
			using ITriangleMesh::m_boundingBox;
			using ITriangleMesh::m_random;
			using ITreeMesh::m_statistics;
		
		public:
			using ITriangleMesh::TriangleList;
			using ITriangleMesh::VertexList;

		protected:
			KDTreeMeshNode m_rootNode;
			int m_nMaxLeafObjects;
			int m_nMaxTreeDepth;
			float m_fMinNodeWidth;

		protected:
			KDTreeMeshNode* RequestNode(void);
			int ReleaseNode(KDTreeMeshNode *p_pNode);

		public:
			KDTreeMesh(int p_nMaxObjectsPerLeaf = 15, int p_nMaxTreeDepth = 30);
			KDTreeMesh(const std::string &p_strName, int p_nMaxObjectsPerLeaf = 15, int p_nMaxTreeDepth = 30);

			~KDTreeMesh();

			boost::shared_ptr<ITriangleMesh> CreateInstance(void);

			bool Compile(void);
			bool Intersects(const Ray &p_ray, DifferentialSurface &p_surface);
			bool Intersects(const Ray &p_ray);
			std::string ToString(void) const;

		protected:
			bool Intersect_New(KDTreeMeshNode *p_pNode, Ray &p_ray);
			bool Intersect_New(KDTreeMeshNode *p_pNode, Ray &p_ray, DifferentialSurface &p_surface);

			bool Intersect_Stack(KDTreeMeshNode *p_pNode, Ray &p_ray);
			bool Intersect_Stack(KDTreeMeshNode *p_pNode, Ray &p_ray, DifferentialSurface &p_surface);

			bool Intersect_Recursive(KDTreeMeshNode *p_pNode, Ray &p_ray);
			bool Intersect_Recursive(KDTreeMeshNode *p_pNode, Ray &p_ray, DifferentialSurface &p_surface);

			void BuildHierarchy(KDTreeMeshNode *p_pNode, List<IndexedTriangle*> &p_objectList, int p_nAxis, int p_nDepth = 0);
			void BuildHierarchy_S2(KDTreeMeshNode *p_pNode, List<IndexedTriangle*> &p_objectList, int p_nAxis, int p_nDepth = 0);
		};
	} 
}
