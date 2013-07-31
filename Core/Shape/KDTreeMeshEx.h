//----------------------------------------------------------------------------------------------
//	Filename:	KDTreeMeshEx.h
//	Author:		Kevin Napoli
//	Date:		01/07/2013
//----------------------------------------------------------------------------------------------
//  TODO:	Clean up code (use IlluminaPRT coding standards)
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/shared_ptr.hpp>
#include "Shape/TreeMesh.h"
#include "System/MemoryManager.h"

namespace Illumina 
{
	namespace Core
	{
		//#pragma pack(push, 1)
		struct KDTreeMeshExNode 
		{
			KDTreeMeshExNode();
			//~KDTreeMeshExNode();
			
			//union{
				//float Partition;
				List<IndexedTriangle*> TriangleList;
			//};
			
			KDTreeMeshExNode * m_pChild;//pointers for left and right since right = left + 1
			float Partition;
			unsigned char Axis;
			//axis is implied by the depth, leaf is implied by child == NULL
			//unsigned int getCount();
			/*void appendPrimitive(IndexedTriangle * prim);
			void appendPrimitives(vector<IndexedTriangle *> prim);
			bool deletePrimitive(const IndexedTriangle * prim);
			bool deletePrimitives(vector<IndexedTriangle *> prim);*/
		};

		struct StackElem {
			KDTreeMeshExNode * node;
			float a, b;
			//StackElem(){}
			//StackElem(Node * n, float tA, float tB) : node ( n ), a ( tA ), b ( tB ) {}
		};

		class KDTreeMeshEx :
			public ITreeMesh
		{
		public:

			KDTreeMeshEx(int p_nMaxObjectsPerLeaf = 10 /*10*/ /*16*/, int p_nMaxTreeDepth = 35 /*30*/);
			KDTreeMeshEx(const std::string &p_strName, int p_nMaxObjectsPerLeaf = 10 /*10*/ /*16*/, int p_nMaxTreeDepth = 35 /*30*/);
			~KDTreeMeshEx(void);
			//nothing to implement from ITreeMesh (TreeMesh.h)
			// Methods for instance creation
			boost::shared_ptr<ITriangleMesh> CreateInstance(void);
			// Intersect methods
			bool Compile(void); //calls build
			virtual bool Intersects(const Ray &p_ray, DifferentialSurface &p_surface);
			virtual bool Intersects(const Ray &p_ray);
			bool Intersects_Stack(Ray &p_ray, DifferentialSurface &p_surface);
			bool Intersects_Stack(Ray &p_ray);
			std::string ToString(void) const;
			//end virtual
			
		protected:
			//methods declared by this class follow below
			void BuildHierarchy(KDTreeMeshExNode *p_pNode, List<IndexedTriangle*> &p_objectList, int p_nAxis, int p_nDepth = 0);
			void BuildHierarchy_S2(KDTreeMeshExNode *p_pNode, List<IndexedTriangle*> &p_objectList, AxisAlignedBoundingBox &p_boundingBox, int p_nAxis, int p_nDepth = 0);

			
		private:
			// Node bounding box
			KDTreeMeshExNode * m_rootNode;
			AxisAlignedBoundingBox m_boundingBox;
			boost::shared_ptr< MemoryManager<KDTreeMeshExNode> > m_pMem;
			unsigned int m_nMaxLeafObjects; //maximum objects per leaf
			unsigned int m_nMaxTreeDepth;
			float m_fMinNodeWidth;
		};
	}
}
