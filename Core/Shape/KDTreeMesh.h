//----------------------------------------------------------------------------------------------
//	Filename:	KDTreeMesh.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <stack>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include "Shape/TriangleMesh.h"
#include "Shape/TreeMesh.h"
#include "Maths/Random.h"

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
		template<class T>
		struct KDTreeNode
		{
			// Node Type
			TreeMeshNodeType Type;

			// Node bounding box
			AxisAlignedBoundingBox BoundingBox;

			// Partition Axis
			int Axis;

			// Partition Point
			float Partition;

			// Only if an internal node
			KDTreeNode *m_pChild[2];

			// Only if a leaf
			List<T> TriangleList;

			// Constructor
			KDTreeNode() { }
			~KDTreeNode() { }
		};

		//----------------------------------------------------------------------------------------------
		// Stack Element for stack-based traversal of KD-Tree
		//----------------------------------------------------------------------------------------------
		template<class T>
		struct KDTreeStackElement
		{
		public:
			float Min;
			float Max;
			KDTreeNode<T> *pNode;

			KDTreeStackElement(KDTreeNode<T> *p_pNode, float p_fMin, float p_fMax)
				: Min(p_fMin)
				, Max(p_fMax)
				, pNode(p_pNode)
			{ }

			KDTreeStackElement(const KDTreeStackElement &p_stackElement)
				: Min(p_stackElement.Min)
				, Max(p_stackElement.Max)
				, pNode(p_stackElement.pNode)
			{ }
		};

		//----------------------------------------------------------------------------------------------
		// KD-Tree Mesh
		//----------------------------------------------------------------------------------------------
		template<class T, class U> 
		class KDTreeMesh
			: public ITriangleMesh<T, U>
		{
		protected:
			TreeMeshStatistics m_statistics;

			KDTreeNode<T*> m_rootNode;
			int m_nMaxLeafObjects;
			int m_nMaxTreeDepth;
			float m_nMinNodeWidth;

		protected:
			//----------------------------------------------------------------------------------------------
			// Helper functions for allocation and deallocation of KDTree nodes
			//----------------------------------------------------------------------------------------------
			KDTreeNode<T*>* RequestNode(void)
			{
				return new KDTreeNode<T*>();
			}

			int ReleaseNode(KDTreeNode<T*> *p_pNode)
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
			//----------------------------------------------------------------------------------------------
			// Constructors and destructor
			//----------------------------------------------------------------------------------------------
			KDTreeMesh(void)
				: ITriangleMesh<T, U>() 
				, m_nMaxLeafObjects(20)
				, m_nMaxTreeDepth(20)
			{ }
			//----------------------------------------------------------------------------------------------
			KDTreeMesh(int p_nMaxObjectsPerLeaf, int p_nMaxTreeDepth)
				: ITriangleMesh<T, U>() 
				, m_nMaxLeafObjects(p_nMaxObjectsPerLeaf)
				, m_nMaxTreeDepth(p_nMaxTreeDepth)
			{ }
			//----------------------------------------------------------------------------------------------
			KDTreeMesh(const std::string &p_strName, int p_nMaxObjectsPerLeaf, int p_nMaxTreeDepth)
				: ITriangleMesh<T, U>(p_strName) 
				, m_nMaxLeafObjects(p_nMaxObjectsPerLeaf)
				, m_nMaxTreeDepth(p_nMaxTreeDepth)
			{ }
			//----------------------------------------------------------------------------------------------
			~KDTreeMesh()
			{
				ReleaseNode(m_rootNode.m_pChild[0]);
				ReleaseNode(m_rootNode.m_pChild[1]);
			}
			//----------------------------------------------------------------------------------------------
			// Method for creating an instance of the KDTreeMesh
			// TODO: Should be replaced through ICloneable interface inherited through Object.
			//----------------------------------------------------------------------------------------------
			boost::shared_ptr<ITriangleMesh<T, U>> CreateInstance(void) {
				return boost::shared_ptr<ITriangleMesh<T, U>>(new KDTreeMesh<T, U>());
			}
			//----------------------------------------------------------------------------------------------
			// Compile : This method is used to transform the raw triangle mesh into a kD-Tree 
			// acceleration structure.
			//----------------------------------------------------------------------------------------------
			bool Compile(void) 
			{
				// Area
				ITriangleMesh<T, U>::ComputeArea();

				// Generate a list of triangle pointers 
				int objectCount = (int)ITriangleMesh<T, U>::TriangleList.Size();
				List<T*> triangleList(objectCount);

				for (int idx = 0; idx < objectCount; idx++) {
					triangleList.PushBack(&ITriangleMesh<T, U>::TriangleList[idx]);
				}

				// Compute the bounds for triangle list
				ComputeBounds(triangleList, m_rootNode.BoundingBox);

				// Compute the minimum dimensions constraint for a node 				
				// TODO: Should be parameterised!
				m_nMinNodeWidth = m_rootNode.BoundingBox.GetRadius() / 1000.0f;
				
				// Build kd-tree hierarchy
				BuildHierarchy(&m_rootNode, triangleList, 0); 

				// Update Stats
				m_statistics.m_triangleCount = objectCount;

				return true;
			}
			//----------------------------------------------------------------------------------------------
			// Returns the result of an intersection between a ray and the kD-Tree. The method also
			// populates a DifferentialSurface structure with all the details of the intersected
			// surface.
			//----------------------------------------------------------------------------------------------
			bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
			{
				Ray ray(p_ray);

				//return Intersect_Recursive(&m_rootNode, ray, p_fTime, p_surface);
				return Intersect_Stack(&m_rootNode, ray, p_fTime, p_surface);
			}
			//----------------------------------------------------------------------------------------------
			// Performs a quick intersection test, returning whether an intersection has occurred
			// or not, but providing no further details as to the intersection itself.
			//----------------------------------------------------------------------------------------------
			bool Intersects(const Ray &p_ray, float p_fTime)
			{
				Ray ray(p_ray);

				return Intersect_Recursive(&m_rootNode, ray, p_fTime);
			}
			//----------------------------------------------------------------------------------------------
			// Returns a literal with information on the acceleration structure.
			//----------------------------------------------------------------------------------------------
			std::string ToString(void) const
			{
				return boost::str(boost::format("\nKDTreeMesh %s") % m_statistics.ToString());
			}

		protected:
			//----------------------------------------------------------------------------------------------
			// Performs intersection testing using a stack-based tree traversal method.
			//----------------------------------------------------------------------------------------------
			bool Intersect_Stack(KDTreeNode<T*> *p_pNode, Ray &p_ray, float p_fTime)
			{
				AxisAlignedBoundingBox *pAABB = 
					&p_pNode->BoundingBox;

				float tIn, tOut, tSplit,
					intercept, direction;

				int count, halfspace;

				if (!pAABB->Intersects(p_ray, tIn, tOut))
					return false;

				tIn = Maths::Max(0, tIn);

				KDTreeStackElement<T*> rootElement(p_pNode, tIn, tOut);
				std::stack<KDTreeStackElement<T*>> traverseStack;
				traverseStack.push(rootElement);

				KDTreeNode<T*> *pNode;

				while(!traverseStack.empty())
				{
					// Get next element
					KDTreeStackElement<T*> nodeElement(traverseStack.top());
					traverseStack.pop();

					pNode = nodeElement.pNode;
					tOut = nodeElement.Max;
					tIn = nodeElement.Min;

					while (pNode->Type == /*TreeMeshNodeType::*/Internal)
					{
						direction = p_ray.Direction[pNode->Axis];
						intercept = p_ray.Origin[pNode->Axis] + tIn * direction;
						halfspace = (intercept > pNode->Partition);

						if (direction == 0.0f)
						{
							pNode = pNode->m_pChild[halfspace];
						}
						else
						{
							tSplit = tIn + (pNode->Partition - intercept) / direction;

							// split is outside region
							if (tSplit < tIn || tSplit > tOut)
							{
								pNode = pNode->m_pChild[halfspace];
							}
							else
							{
								KDTreeStackElement<T*> nodeElementHS(pNode->m_pChild[halfspace^1], tSplit, tOut);
								traverseStack.push(nodeElementHS);

								pNode = pNode->m_pChild[halfspace];
								tOut = tSplit;
							}
						}
					}

					//---> Intersection tests at leaf
					if ((count = pNode->TriangleList.Size()) > 0)
					{
						for (int n = 0; n < count; n++)
						{
							if (pNode->TriangleList[n]->Intersect(p_ray, p_fTime))
								return true;
						}
					}
				}

				return false;
			}
			//----------------------------------------------------------------------------------------------
			// Performs intersection testing using stack-based traversal.
			//----------------------------------------------------------------------------------------------
			bool Intersect_Stack(KDTreeNode<T*> *p_pNode, Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
			{
				AxisAlignedBoundingBox *pAABB = 
					&p_pNode->BoundingBox;

				float tIn, tOut, tHit, tSplit,
					intercept, direction;

				int count, halfspace;

				if (!pAABB->Intersects(p_ray, tIn, tOut))
					return false;

				tHit = tOut;
				tIn = Maths::Max(0, tIn);

				bool bIntersect = false;

				KDTreeStackElement<T*> rootElement(p_pNode, tIn, tOut);
				std::stack<KDTreeStackElement<T*>> traverseStack;
				traverseStack.push(rootElement);

				KDTreeNode<T*> *pNode;

				while(!traverseStack.empty())
				{
					// Get next element
					KDTreeStackElement<T*> nodeElement(traverseStack.top());
					traverseStack.pop();

					pNode = nodeElement.pNode;
					tOut = nodeElement.Max;
					tIn = nodeElement.Min;

					while (pNode->Type == Internal)
					{
						direction = p_ray.Direction[pNode->Axis];
						intercept = p_ray.Origin[pNode->Axis] + tIn * direction;
						halfspace = (intercept > pNode->Partition);

						if (direction == 0.0f)
						{
							pNode = pNode->m_pChild[halfspace];
						}
						else
						{
							tSplit = tIn + (pNode->Partition - intercept) / direction;

							// split is outside region
							if (tSplit < tIn || tSplit > tOut)
							{
								pNode = pNode->m_pChild[halfspace];
							}
							else
							{
								KDTreeStackElement<T*> nodeElementHS(pNode->m_pChild[halfspace^1], tSplit, tOut);
								traverseStack.push(nodeElementHS);

								pNode = pNode->m_pChild[halfspace];
								tOut = tSplit;
							}
						}
					}

					//---> Intersection tests at leaf
					if ((count = (int)pNode->TriangleList.Size()) > 0)
					{
						p_ray.Min = tIn;
						p_ray.Max = tHit;

						for (int n = 0; n < count; n++)
						{
							if (pNode->TriangleList[n]->Intersects(p_ray, p_fTime, p_surface))
							{
								p_ray.Max = 
									tHit = Maths::Min(tHit, p_surface.Distance);

								if (tHit <= tOut + Maths::Epsilon)
									bIntersect = true;
							}
						}
					}

					if (bIntersect)
						return true;
				}

				return bIntersect;
			}
			//----------------------------------------------------------------------------------------------
			// Performs intersection testing using recursive traversal.
			//----------------------------------------------------------------------------------------------
			bool Intersect_Recursive(KDTreeNode<T*> *p_pNode, Ray &p_ray, float p_fTime)
			{
				float in, out;

				if (p_pNode->BoundingBox.Intersects(p_ray, in, out))
				{
					if (p_pNode->Type == Internal)
						return Intersect_Recursive(p_pNode->m_pChild[0], p_ray, p_fTime) || Intersect_Recursive(p_pNode->m_pChild[1], p_ray, p_fTime);

					int count = (int)p_pNode->TriangleList.Size();

					if (count == 0) 
						return false;

					for (int n = 0; n < count; n++)
					{
						if (p_pNode->TriangleList[n]->Intersects(p_ray, p_fTime))
							return true;
					}
				}

				return false;
			}
			//----------------------------------------------------------------------------------------------
			// Performs intersection testing using recursive traversal.
			//----------------------------------------------------------------------------------------------
			bool Intersect_Recursive(KDTreeNode<T*> *p_pNode, Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
			{
				
				float in, out;

				if (p_pNode->BoundingBox.Intersects(p_ray, in, out))
				{
					in = in < 0 ? 0 : in;

					// Traverse internal nodes
					if (p_pNode->Type == Internal)
					{
						float direction = p_ray.Direction[p_pNode->Axis],
							intercept = p_ray.Origin[p_pNode->Axis] + in * direction;

						int halfspace = (intercept > p_pNode->Partition);

						if (direction != 0.0f)
						{
							float tSplit = in + (p_pNode->Partition - intercept) / direction;

							// split is outside region
							if (tSplit < in || tSplit > out)
								return Intersect_Recursive(p_pNode->m_pChild[halfspace], p_ray, p_fTime, p_surface);
							else
								return Intersect_Recursive(p_pNode->m_pChild[halfspace], p_ray, p_fTime, p_surface) || 
								Intersect_Recursive(p_pNode->m_pChild[halfspace^1], p_ray, p_fTime, p_surface);
						}
						else
						{
							return Intersect_Recursive(p_pNode->m_pChild[halfspace], p_ray, p_fTime, p_surface);
						}
					}

					// Test geometry at leaf
					bool bIntersect = false;
					int count = (int)p_pNode->TriangleList.Size();

					if (count > 0)
					{
						for (int n = 0; n < count; n++)
						{
							if (p_pNode->TriangleList[n]->Intersects(p_ray, p_fTime, p_surface))
							{
								p_ray.Max = Maths::Min(p_ray.Max, p_surface.Distance);

								if (p_surface.Distance <= out + Maths::Epsilon)
									bIntersect = true;
							}
						}
					}

					return bIntersect;
				}

				return false;
			}
			//----------------------------------------------------------------------------------------------
			// Computes the axis-aligned bounding box for the specified list of triangles.
			// TODO: This could be useful at a higher level, to be shared by tree/hierarchy-based 
			//		 acceleration structures.
			//----------------------------------------------------------------------------------------------
			void ComputeBounds(const List<T*> &p_objectList, AxisAlignedBoundingBox &p_aabb, float p_fMinEpsilon = 0.0f, float p_fMaxEpsilon = 0.0f)
			{
				p_aabb.Invalidate();

				if (p_objectList.Size() > 0)
				{
					p_aabb.ComputeFromVolume(*(p_objectList[0]->GetBoundingVolume()));

					for (int idx = 1, count = (int)p_objectList.Size(); idx < count; idx++) {
						p_aabb.Union(*(p_objectList[idx]->GetBoundingVolume()));
					}

					p_aabb.SetMinExtent(p_aabb.GetMinExtent() - p_fMinEpsilon);
					p_aabb.SetMaxExtent(p_aabb.GetMaxExtent() + p_fMaxEpsilon);
				}
			}
			//----------------------------------------------------------------------------------------------
			// Distributes the provided triangle list between two objects depending on a the partitioning
			// specified by the parameters (Axis, Partition).
			// TODO: Same as above, consider moving upwards (to a TreeTriangleMesh base or something similar).
			//----------------------------------------------------------------------------------------------
			int Distribute(const List<T*> &p_objectList, float p_fPartition, int p_nAxis, List<T*> &p_outLeftList, List<T*> &p_outRightList)
			{
				int count = (int)p_objectList.Size();
				for (int n = 0; n < count; n++)
				{
					float min = p_objectList[n]->GetBoundingVolume()->GetMinExtent(p_nAxis),
						max = p_objectList[n]->GetBoundingVolume()->GetMaxExtent(p_nAxis);

					if (p_fPartition >= min) p_outLeftList.PushBack(p_objectList[n]);
					if (p_fPartition <= max) p_outRightList.PushBack(p_objectList[n]);
				}

				return (int)p_outLeftList.Size();
			}
			//----------------------------------------------------------------------------------------------
			// Distributes the provided triangle list between two objects depending on which axis-aligned
			// bounding box they intersect (left AABB or right AABB).
			// TODO: Same as above, consider moving upwards (to a TreeTriangleMesh base or something similar).
			//----------------------------------------------------------------------------------------------
			int Distribute(const List<T*> &p_objectList, AxisAlignedBoundingBox &p_leftAABB, AxisAlignedBoundingBox &p_rightAABB, List<T*> &p_outLeftList, List<T*> &p_outRightList)
			{
				int count = (int)p_objectList.Size();
				for (int n = 0; n < count; n++)
				{
					if (p_objectList[n]->GetBoundingVolume()->Intersects(p_leftAABB))
						p_outLeftList.PushBack(p_objectList[n]);

					if (p_objectList[n]->GetBoundingVolume()->Intersects(p_rightAABB))
						p_outRightList.PushBack(p_objectList[n]);
				}

				return (int)p_outLeftList.Size();
			}
			//----------------------------------------------------------------------------------------------
			// Determines a plane by which to partition two half spaces.
			//----------------------------------------------------------------------------------------------
			float FindPartitionPlane(const List<T*> &p_objectList, AxisAlignedBoundingBox &p_aabb, int p_nAxis)
			{
				//float fPartition = FindPartitionPlane_Centroid(p_objectList, p_nAxis);
				//float fPartition = FindPartitionPlane_SpatialMedian(p_objectList, p_aabb, p_nAxis);
				float fPartition = FindPartitionPlane_SAH(p_objectList, p_aabb, p_nAxis);

				//std::cout << "Partitioning at [" << fPartition << "]" << std::endl;
				//std::cout << "In terms of AABB : [" << (fPartition - p_aabb.GetMinExtent(p_nAxis)) / (p_aabb.GetMaxExtent(p_nAxis) - p_aabb.GetMinExtent(p_nAxis)) << "]" << std::endl;
				return fPartition;
			}
			//----------------------------------------------------------------------------------------------
			// Returns a partition based on the spatian median of the object list.
			//----------------------------------------------------------------------------------------------
			float FindPartitionPlane_SpatialMedian(const List<T*> &p_objectList, AxisAlignedBoundingBox &p_aabb, int p_nAxis)
			{
				return p_aabb.GetCentre()[p_nAxis];
			}
			//----------------------------------------------------------------------------------------------
			// Returns a partition baed on the (modified) SAH applied to the object list.
			//----------------------------------------------------------------------------------------------
			float FindPartitionPlane_SAH(const List<T*> &p_objectList, AxisAlignedBoundingBox &p_aabb, int p_nAxis)
			{
				const int Bins = 128;

				int minBins[Bins], 
					maxBins[Bins];

				for (int j = 0; j < Bins; j++)
					maxBins[j] = minBins[j] = 0;

				float extent = p_aabb.GetMaxExtent(p_nAxis) - p_aabb.GetMinExtent(p_nAxis),
					start = p_aabb.GetMinExtent(p_nAxis);

				int count = (int)p_objectList.Size();
				//std::cout << "Prim count : " << count << std::endl;

				for (int n = 0; n < count; n++)
				{
					IBoundingVolume* pAABB = p_objectList[n]->GetBoundingVolume();

					int left = (int)(Bins * ((pAABB->GetMinExtent(p_nAxis) - start) / extent)),
						right = (int)(Bins * ((pAABB->GetMaxExtent(p_nAxis) - start) / extent));

					if (left >= 0 && left < Bins)
						minBins[left]++;

					if (right >= 0 && right < Bins)
						maxBins[right]++;
				}

				int leftPrims, rightPrims, bestSplit;
				float cost, bestCost = Maths::Maximum;

				for (int j = 0; j < Bins; j++)
				{
					leftPrims = rightPrims = 1;

					for (int k = 0; k <=j; k++)
						leftPrims += minBins[k];

					for (int k = j; k < Bins; k++)
						rightPrims += maxBins[k];

					cost = (float)((rightPrims * (Bins - j) + leftPrims * j)) / Bins;

					if (cost < bestCost)
					{
						bestCost = cost;
						bestSplit = j;
					}
				}

				return start + (bestSplit * extent) / Bins;
			}
			//----------------------------------------------------------------------------------------------
			// Builds the kd-tree hierarchy
			//----------------------------------------------------------------------------------------------
			void BuildHierarchy(KDTreeNode<T*> *p_pNode, List<T*> &p_objectList, int p_nAxis, int p_nDepth = 0)
			{
				ComputeBounds(p_objectList, p_pNode->BoundingBox, 0.0001f, 0.0001f);
				const Vector3 &size = p_pNode->BoundingBox.GetExtent();
				if (size.X > size.Y) p_nAxis = size.X > size.Z ? 0 : 2;
				else p_nAxis = size.Y > size.Z ? 1 : 2;

				BuildHierarchy_S2(p_pNode, p_objectList, p_nAxis, 0);
			}
			//----------------------------------------------------------------------------------------------
			// Builds the kd-tree hierarchy
			//----------------------------------------------------------------------------------------------
			void BuildHierarchy_S2(KDTreeNode<T*> *p_pNode, List<T*> &p_objectList, int p_nAxis, int p_nDepth = 0)
			{
				// Update stats
				m_statistics.m_maxTreeDepth = Maths::Min(p_nDepth, m_statistics.m_maxTreeDepth);

				// If we have enough objects, we consider this node a leaf
				if ((int)p_objectList.Size() <= m_nMaxLeafObjects || p_nDepth == m_nMaxTreeDepth || p_pNode->BoundingBox.GetRadius() <= m_nMinNodeWidth)
				{
					//std::cout << "Adding leaf node [" << p_objectList.Size() << ", " << p_nDepth << "]" << std::endl;
					p_pNode->Type = Leaf; 
					p_pNode->TriangleList.PushBack(p_objectList);

					m_statistics.m_leafNodeCount++;
					m_statistics.m_minTreeDepth = Maths::Min(m_statistics.m_minTreeDepth, p_nDepth);
					m_statistics.m_minLeafTriangleCount = Maths::Min(m_statistics.m_minLeafTriangleCount, (int)p_objectList.Size());
					m_statistics.m_maxLeafTriangleCount = Maths::Min(m_statistics.m_maxLeafTriangleCount, (int)p_objectList.Size());
				}
				else
				{
					//std::cout << "Adding internal node [" << p_objectList.Size() << ", " << p_nDepth << "]" << std::endl;
					p_pNode->Type = Internal;
					p_pNode->Axis = p_nAxis;
					p_pNode->Partition = FindPartitionPlane(p_objectList, p_pNode->BoundingBox, p_nAxis);

					List<T*> leftList, rightList;
					leftList.Clear(); rightList.Clear(); 

					p_pNode->m_pChild[0] = RequestNode();
					p_pNode->m_pChild[1] = RequestNode();

					AxisAlignedBoundingBox 
						&leftAABB = p_pNode->m_pChild[0]->BoundingBox,
						&rightAABB = p_pNode->m_pChild[1]->BoundingBox;

					leftAABB = p_pNode->BoundingBox;
					rightAABB = p_pNode->BoundingBox;

					leftAABB.SetMaxExtent(p_nAxis, p_pNode->Partition);
					rightAABB.SetMinExtent(p_nAxis, p_pNode->Partition);

					//Distribute(p_objectList, leftAABB, rightAABB, leftList, rightList);
					Distribute(p_objectList, p_pNode->Partition, p_pNode->Axis, leftList, rightList);

					int nAxis = (p_nAxis + 1) % 3,
						nDepth = p_nDepth + 1;

					BuildHierarchy_S2(p_pNode->m_pChild[0], leftList, nAxis, nDepth);
					BuildHierarchy_S2(p_pNode->m_pChild[1], rightList, nAxis, nDepth);

					m_statistics.m_internalNodeCount++;
				}
			}
			//----------------------------------------------------------------------------------------------
		};
	} 
}
