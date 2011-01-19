//----------------------------------------------------------------------------------------------
//	Filename:	AggregateMesh.h

//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
#pragma once

#include <stack>
#include <algorithm>

#include <boost/shared_ptr.hpp>

#include "Shape/TriangleMesh.h"
#include "Shape/TreeMesh.h"

namespace Illumina 
{
	namespace Core
	{
		/*
		// Bounding volume hierarchy node
		template<class T>
		struct BVHNode
		{
			// Node bounding box
			AxisAlignedBoundingBox BoundingBox;

			// Node Type
			TreeMeshNodeType Type;

			// Only if an internal node
			BVHNode *m_pChild[2];

			// Only if a leaf
			List<T> TriangleList;

			// Construction and destruction
			BVHNode() { }
			~BVHNode() { }
		};

		// Bounding Volume Hierarchy Mesh
		template<class T, class U> 
		class BVHMesh
			: public ITriangleMesh<T, U>
		{
		protected:
			TreeMeshStatistics m_statistics;

			int m_nMaxLeafObjects,
				m_nMaxDepth;

			BVHNode<T*> m_rootNode;

		protected:
			//----------------------------------------------------------------------------------------------
			// Methods for requesting and freeing nodes
			//----------------------------------------------------------------------------------------------
			BVHNode<T*>* RequestNode(void)
			{
				return new BVHNode<T*>();
			}

			int ReleaseNode(BVHNode<T*> *p_pNode)
			{
				int nodesFreed = 0;

				if (p_pNode->Type == Internal)
				{
					nodesFreed += ReleaseNode(p_pNode->m_pChild[0]);
					nodesFreed += ReleaseNode(p_pNode->m_pChild[1]);
				}
				else
				{
					Safe_Delete(p_pNode);
					nodesFreed++;
				}

				return nodesFreed;
			}

		public:
			BVHMesh(void)
				: m_nMaxLeafObjects(20)
				, m_nMaxDepth(20)
			{ }

			BVHMesh(int p_nMaxObjectsPerLeaf, int p_nMaxDepth = 20)
				: m_nMaxLeafObjects(p_nMaxObjectsPerLeaf)
				, m_nMaxDepth(p_nMaxDepth)
			{ }

			~BVHMesh()
			{
				ReleaseNode(m_rootNode.m_pChild[0]);
				ReleaseNode(m_rootNode.m_pChild[1]);
			}

			boost::shared_ptr<ITriangleMesh<T, U>> CreateInstance(void) {
				return boost::shared_ptr<ITriangleMesh<T, U>>(new BVHMesh<T, U>());
			}

			bool Compile(void) 
			{
				// Create a list of pointers to indexed triangles
				int objectCount = (int)ITriangleMesh<T, U>::TriangleList.Size();
				List<T*> triangleList(objectCount);

				for (int idx = 0; idx < objectCount; idx++) {
					triangleList.PushBack(&ITriangleMesh<T, U>::TriangleList[idx]);
				}

				// Build bounding volume hierarchy
				BuildHierarchy(&m_rootNode, triangleList, 0); 

				// Update Stats
				m_statistics.m_triangleCount = objectCount;

				return true;
			}

			bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
			{
				Ray ray(p_ray);

				return Intersect_Stack(&m_rootNode, ray, p_fTime, p_surface);
			}

			bool Intersects(const Ray &p_ray, float p_fTime)
			{
				Ray ray(p_ray);

				return Intersect_Stack(&m_rootNode, ray, p_fTime);
			}

			std::string ToString(void) const
			{
				return boost::str(boost::format("\nBVHMesh %s") % m_statistics.ToString());
			}

		protected:
			bool Intersect_Stack(BVHNode<T*> *p_pNode, Ray &p_ray, float p_fTime)
			{
				if (!p_pNode->BoundingBox.Intersects(p_ray))
					return false;

				std::stack<BVHNode<T*>*> traverseStack;
				traverseStack.push(p_pNode);

				BVHNode<T*> *pNode;
				int count;

				while(!traverseStack.empty())
				{
					pNode = traverseStack.top();
					traverseStack.pop();

					while(pNode->Type == Internal)
					{
						if (pNode->m_pChild[1]->BoundingBox.Intersects(p_ray))
							traverseStack.push(pNode->m_pChild[1]);

						if (pNode->m_pChild[0]->BoundingBox.Intersects(p_ray))
							pNode = pNode->m_pChild[0];
						else
							break;
					}

					if ((count = (int)pNode->TriangleList.Size()) > 0) 
					{
						for (int n = 0; n < count; n++)
						{
							if (pNode->TriangleList[n]->Intersects(p_ray, p_fTime))
								return true;
						}
					}
				}

				return false;
			}

			bool Intersect_Stack(BVHNode<T*> *p_pNode, Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
			{
				if (!p_pNode->BoundingBox.Intersects(p_ray))
					return false;

				std::stack<BVHNode<T*>*> traverseStack;
				traverseStack.push(p_pNode);

				bool bIntersect = false;
				BVHNode<T*> *pNode;
				int count;

				while(!traverseStack.empty())
				{
					pNode = traverseStack.top();
					traverseStack.pop();

					while(pNode->Type == Internal)
					{
						if (pNode->m_pChild[1]->BoundingBox.Intersects(p_ray))
							traverseStack.push(pNode->m_pChild[1]);

						if (pNode->m_pChild[0]->BoundingBox.Intersects(p_ray))
							pNode = pNode->m_pChild[0];
						else
							break;
					}

					if ((count = (int)pNode->TriangleList.Size()) > 0) 
					{
						for (int n = 0; n < count; n++)
						{
							if (pNode->TriangleList[n]->Intersects(p_ray, p_fTime, p_surface))
							{
								p_ray.Max = Maths::Min(p_ray.Max, p_surface.Distance);
								bIntersect = true;
							}
						}
					}
				}

				return bIntersect;
			}

			bool Intersect_Recursive(BVHNode<T*> *p_pNode, Ray &p_ray, float p_fTime)
			{
				float in, out;

				if (p_pNode->BoundingBox.Intersect(p_ray, in, out))
				{
					if (p_pNode->Type == Internal)
						return Intersect_Recursive(p_pNode->m_pChild[0], p_ray, p_fTime) || Intersect_Recursive(p_pNode->m_pChild[1], p_ray, p_fTime);

					int count = (int)p_pNode->TriangleList.Size();

					if (count == 0) 
						return false;

					for (int n = 0; n < count; n++)
					{
						if (p_pNode->TriangleList[n]->Intersect(p_ray, p_fTime))
							return true;
					}
				}

				return false;
			}

			bool Intersect_Recursive(BVHNode<T*> *p_pNode, Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
			{
				float in, out;

				if (p_pNode->BoundingBox.Intersect(p_ray, in, out))
				{
					if (p_pNode->Type == Internal)
						return Intersect_Recursive(p_pNode->m_pChild[0], p_ray, p_fTime, p_surface) | Intersect_Recursive(p_pNode->m_pChild[1], p_ray, p_fTime, p_surface);

					int count = (int)p_pNode->TriangleList.Size();

					if (count == 0) 
						return false;

					bool bIntersect = false;

					for (int n = 0; n < count; n++)
					{
						if (p_pNode->TriangleList[n]->Intersect(p_ray, p_fTime, p_surface))
						{
							p_ray.Max = Maths::Min(p_ray.Max, p_surface.Distance);
							bIntersect = true;
						}
					}

					return bIntersect;
				}

				return false;
			}

			void ComputeBounds(const List<T*> &p_objectList, AxisAlignedBoundingBox &p_aabb)
			{
				p_aabb.Invalidate();

				if (p_objectList.Size() > 0)
				{
					p_aabb.ComputeFromVolume(*(p_objectList[0]->GetBoundingVolume()));

					for (int idx = 1, count = (int)p_objectList.Size(); idx < count; idx++) {
						p_aabb.Union(*(p_objectList[idx]->GetBoundingVolume()));
					}
				}
			}

			int Split(const List<T*> &p_objectList, float p_fPartition, int p_nAxis, List<T*> &p_outLeftList, List<T*> &p_outRightList)
			{
				int count = (int)p_objectList.Size();
				for (int n = 0; n < count; n++)
				{
					if (p_objectList[n]->GetBoundingVolume()->GetCentre()[p_nAxis] >= p_fPartition)
						p_outRightList.PushBack(p_objectList[n]);
					else 
						p_outLeftList.PushBack(p_objectList[n]);
				}

				return (int)p_outLeftList.Size();
			}

			float FindPartitionPlane(const List<T*> &p_objectList, int p_nAxis)
			{
				return FindPartitionPlane_Centroid(p_objectList, p_nAxis);
			}

			float FindPartitionPlane_Centroid(const List<T*> &p_objectList, int p_nAxis)
			{
				// Initialise centroid for object cluster
				Vector3 Centroid(0.0f);

				// Calculate cluster centroid
				int objectCount = (int)p_objectList.Size();

				AxisAlignedBoundingBox *pAABB = NULL;

				for (int idx = 0; idx < objectCount; idx++)
				{
					pAABB = (AxisAlignedBoundingBox*)p_objectList[idx]->GetBoundingVolume();
					Vector3::Add(Centroid, pAABB->GetCentre(), Centroid);
				}

				Centroid /= (float)objectCount;
				return Centroid[p_nAxis];
			}

			void BuildHierarchy(BVHNode<T*> *p_pNode, List<T*> &p_objectList, int p_nAxis, int p_nDepth = 0)
			{
				// Update stats
				m_statistics.m_maxTreeDepth = Maths::Max(p_nDepth, m_statistics.m_maxTreeDepth);

				// Compute the node bounds for the given object list
				ComputeBounds(p_objectList, p_pNode->BoundingBox);

				// If we have enough objects, we consider this node a leaf
				if ((int)p_objectList.Size() <= m_nMaxLeafObjects || p_nDepth == m_nMaxDepth)
				{
					p_pNode->Type = Leaf; 
					p_pNode->TriangleList.PushBack(p_objectList);

					m_statistics.m_leafNodeCount++;
					m_statistics.m_minTreeDepth = Maths::Min(m_statistics.m_minTreeDepth, p_nDepth);
					m_statistics.m_minLeafTriangleCount = Maths::Min(m_statistics.m_minLeafTriangleCount, (int)p_objectList.Size());
					m_statistics.m_maxLeafTriangleCount = Maths::Max(m_statistics.m_maxLeafTriangleCount, (int)p_objectList.Size());
				}
				else
				{
					List<T*> leftList, rightList;
					leftList.Clear(); rightList.Clear();
					float fPartition = FindPartitionPlane(p_objectList, p_nAxis);
					Split(p_objectList, fPartition, p_nAxis, leftList, rightList);

					if (leftList.Size() == 0 || rightList.Size() == 0)
					{
						p_pNode->Type = Leaf; 
						p_pNode->TriangleList.PushBack(p_objectList);

						m_statistics.m_leafNodeCount++;
						m_statistics.m_minTreeDepth = Maths::Min(m_statistics.m_minTreeDepth, p_nDepth);
						m_statistics.m_minLeafTriangleCount = Maths::Min(m_statistics.m_minLeafTriangleCount, (int)p_objectList.Size());
						m_statistics.m_maxLeafTriangleCount = Maths::Max(m_statistics.m_maxLeafTriangleCount, (int)p_objectList.Size());
					}
					else
					{
						p_pNode->Type = Internal;

						p_pNode->m_pChild[0] = RequestNode();
						p_pNode->m_pChild[1] = RequestNode();

						int nAxis = (p_nAxis + 1) % 3,
							nDepth = p_nDepth + 1;

						BuildHierarchy(p_pNode->m_pChild[0], leftList, nAxis, nDepth);
						BuildHierarchy(p_pNode->m_pChild[1], rightList, nAxis, nDepth);

						m_statistics.m_internalNodeCount++;
					}
				}
			}
		};
		*/
	} 
}