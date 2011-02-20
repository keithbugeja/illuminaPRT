//----------------------------------------------------------------------------------------------
//	Filename:	Acceleration.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
// TODO: 
//	1. Split A.S. into separate files
//  2. Possibly add Intersects method for rays in base class
//	3. Semantics are unclear - kd-tree stores pointer on insert, not an instance copy
//	   Perhaps kd-tree->ObjectList should keep instance copies, and nodes keep pointers instead
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "Geometry/BoundingBox.h"
#include "Threading/List.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------
		template <class T>
		class IAccelerationStructureLookupMethod
		{
		public:
			virtual bool operator()(const Vector3 &p_lookupPoint, float p_fMaxDistance, T &p_element) = 0;
		};
		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------
		template <class T>
		class IAccelerationStructure
		{
		public:
			virtual void Remove(T &p_element) = 0;
			virtual void Insert(T &p_element) = 0;

			virtual bool Build(void) = 0;
			virtual bool Update(void) = 0;

			virtual bool Lookup(const Vector3 &p_point, float p_fRadius, IAccelerationStructureLookupMethod<T> &p_lookupMethod) = 0;

			virtual float GetIntegrityScore(void) = 0;
		};
		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------
		template <class T>
		class ITreeAccelerationStructure 
			: public IAccelerationStructure<T>
		{
		public:
			enum NodeType
			{
				Internal	= 0,
				Leaf		= 1
			};

		protected:
			enum PartitionType
			{
				SpatialMedian,
				SurfaceAreaHeuristic
			};

			//----------------------------------------------------------------------------------------------
			void ComputeBounds(const List<T*> &p_objectList, 
							   AxisAlignedBoundingBox &p_aabb, 
							   float p_fMinEpsilon = 0.0f, 
							   float p_fMaxEpsilon = 0.0f)
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
			int Distribute(const List<T*> &p_objectList, float p_fPartition, 
						   int p_nAxis, List<T*> &p_outLeftList, List<T*> &p_outRightList)
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
			int Distribute(const List<T*> &p_objectList, 
						   AxisAlignedBoundingBox &p_leftAABB, AxisAlignedBoundingBox &p_rightAABB, 
						   List<T*> &p_outLeftList, List<T*> &p_outRightList)
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
			float FindPartitionPlane(const List<T*> &p_objectList, 
									 AxisAlignedBoundingBox &p_aabb, 
									 int p_nAxis, PartitionType p_partition) 
			{
				switch(p_partition)
				{
					case SurfaceAreaHeuristic:
						return FindPartitionPlaneSAH(p_objectList, p_aabb, p_nAxis);

					case SpatialMedian:
					default:
						return FindPartitionPlaneSpatialMedian(p_objectList, p_aabb, p_nAxis);
				}
			}

			//----------------------------------------------------------------------------------------------
			float FindPartitionPlaneSpatialMedian(const List<T*> &p_objectList, 
					AxisAlignedBoundingBox &p_aabb, int p_nAxis) 
			{
				return p_aabb.GetCentre()[p_nAxis];
			}
			//----------------------------------------------------------------------------------------------
			float FindPartitionPlaneSAH(const List<T*> &p_objectList, 
										AxisAlignedBoundingBox &p_aabb, int p_nAxis)
			{
				const int Bins = 128;

				int minBins[Bins], 
					maxBins[Bins];

				for (int j = 0; j < Bins; j++)
					maxBins[j] = minBins[j] = 0;

				float extent = p_aabb.GetMaxExtent(p_nAxis) - p_aabb.GetMinExtent(p_nAxis),
					start = p_aabb.GetMinExtent(p_nAxis);

				int count = (int)p_objectList.Size();

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
		};
		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------
		template <class T>
		struct KDTreeNode
		{
			AxisAlignedBoundingBox BoundingBox;

			unsigned char Type	: 1;
			unsigned char Axis	: 2;

			float Partition;
			KDTreeNode *ChildNode[2];
			List<T> ObjectList;

			KDTreeNode(void) { ChildNode[0] = ChildNode[1] = NULL; }
			~KDTreeNode() { }
		};
		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------
		template <class T>
		class KDTreeLookupMethod 
			: public IAccelerationStructureLookupMethod<T>
		{
		public:
			bool operator()(const Vector3 &p_lookupPoint, float p_fMaxDistance, T &p_element) 
			{
				return false; 
			}
		};

		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------
		// T should implement: IBoundingVolume* GetBoundingVolume(void)
		template <class T>
		class KDTree 
			: public ITreeAccelerationStructure<T>
		{
			int m_nMaxTreeDepth,
				m_nMaxLeafObjects;

			float m_fMinNodeWidth;

			KDTreeNode<T*> RootNode;
			List<T*> ObjectList;
		
		protected:
			//----------------------------------------------------------------------------------------------
			KDTreeNode<T*>* RequestNode(void)
			{
				return new KDTreeNode<T*>();
			}
			//----------------------------------------------------------------------------------------------
			int ReleaseNode(KDTreeNode<T*> *p_pNode)
			{
				int nodesFreed = 0;

				if (p_pNode != NULL && p_pNode->Type == Internal)
				{
					nodesFreed += ReleaseNode(p_pNode->ChildNode[0]);
					nodesFreed += ReleaseNode(p_pNode->ChildNode[1]);
				}
				else
				{
					Safe_Delete(p_pNode);
					nodesFreed++;
				}

				return nodesFreed;
			}
			//----------------------------------------------------------------------------------------------
		public:
			KDTree(int p_nMaxLeafObjects = 10, int p_nMaxTreeDepth = 16, float p_fMinNodeWidth = 1e-2)
				: m_nMaxLeafObjects(p_nMaxLeafObjects)
				, m_nMaxTreeDepth(p_nMaxTreeDepth)
				, m_fMinNodeWidth(p_fMinNodeWidth)
			{ }

			KDTree::~KDTree(void)
			{
				ReleaseNode(RootNode.ChildNode[0]);
				ReleaseNode(RootNode.ChildNode[1]);
			}

			void Remove(T &p_element) {
				throw new Exception("Method not supported!");
			}

			void Insert(T &p_element) {
				ObjectList.PushBack(&p_element);
			}

			float GetIntegrityScore(void) {
				return 0;
			}

			bool Build(void) 
			{
				ComputeBounds(ObjectList, RootNode.BoundingBox, 1e-6f, 1e-6f);
				const Vector3 &size = RootNode.BoundingBox.GetExtent();

				int axis;
				if (size.X > size.Y) axis = size.X > size.Z ? 0 : 2;
				else axis = size.Y > size.Z ? 1 : 2;

				BuildHierarchy(&RootNode, ObjectList, axis, 0);
				return true;
			}

			void BuildHierarchy(KDTreeNode<T*> *p_pNode, List<T*> &p_objectList, int p_nAxis, int p_nDepth)
			{
				// If we have enough objects, we consider this node a leaf
				if ((int)p_objectList.Size() <= m_nMaxLeafObjects || p_nDepth == m_nMaxTreeDepth || p_pNode->BoundingBox.GetRadius() <= m_fMinNodeWidth)
				{
					p_pNode->Type = Leaf; 
					p_pNode->ObjectList.PushBack(p_objectList);
				}
				else
				{
					p_pNode->Type = Internal;
					p_pNode->Axis = p_nAxis;
					p_pNode->Partition = FindPartitionPlane(p_objectList, p_pNode->BoundingBox, p_nAxis, SurfaceAreaHeuristic);

					List<T*> leftList, rightList;
					leftList.Clear(); rightList.Clear(); 

					p_pNode->ChildNode[0] = RequestNode();
					p_pNode->ChildNode[1] = RequestNode();

					AxisAlignedBoundingBox 
						&leftAABB = p_pNode->ChildNode[0]->BoundingBox,
						&rightAABB = p_pNode->ChildNode[1]->BoundingBox;

					leftAABB = p_pNode->BoundingBox;
					rightAABB = p_pNode->BoundingBox;

					leftAABB.SetMaxExtent(p_nAxis, p_pNode->Partition);
					rightAABB.SetMinExtent(p_nAxis, p_pNode->Partition);

					Distribute(p_objectList, p_pNode->Partition, p_pNode->Axis, leftList, rightList);

					int nAxis = (p_nAxis + 1) % 3,
						nDepth = p_nDepth + 1;

					BuildHierarchy(p_pNode->ChildNode[0], leftList, nAxis, nDepth);
					BuildHierarchy(p_pNode->ChildNode[1], rightList, nAxis, nDepth);
				}
			}

			bool Update(void) {
				return true;
			}

			//// Lookup for elements in range
			//bool Lookup(const IBoundingVolume *p_pBoundingVolume, IAccelerationStructureLookupMethod<T> &p_lookupMethod) {
			//	return true;
			//}

			// Lookup for elements in range
			bool Lookup(const Vector3 &p_point, float p_fRadius, IAccelerationStructureLookupMethod<T> &p_lookupMethod) {
				return Lookup(&RootNode, p_point, p_fRadius, p_lookupMethod);
			}

			bool Lookup(KDTreeNode<T*> *p_pNode, const Vector3 &p_point, float p_fRadius, IAccelerationStructureLookupMethod<T> &p_lookupMethod) 
			{
				if (p_pNode->Type == Internal)
				{
					int axis = p_pNode->Axis;

					float min = p_point[axis] - p_fRadius,
						max = p_point[axis] + p_fRadius;

					bool leftChild = false, 
						rightChild = false;

					// Intersects both half-spaces
					if (p_pNode->Partition >= min && p_pNode->Partition <= max) {
						leftChild = rightChild = true;
					} 
					// Intersects left half-space
					else if (p_pNode->Partition > max) {
						leftChild = true;
					}
					// Intersects right half-space
					else if (p_pNode->Partition < min) {
						rightChild = true;
					}

					if (leftChild && p_pNode->ChildNode[0] != NULL)
						return Lookup(p_pNode->ChildNode[0], p_point, p_fRadius, p_lookupMethod);

					if (rightChild && p_pNode->ChildNode[1] != NULL)
						return Lookup(p_pNode->ChildNode[1], p_point, p_fRadius, p_lookupMethod);
				}

				int count = p_pNode->ObjectList.Size();
				
				if (count == 0)
					return false;

				bool result = false;

				for (int objIdx = 0; objIdx < count; ++objIdx)
					result |= p_lookupMethod(p_point, p_fRadius, *(p_pNode->ObjectList[objIdx]));

				return result;
			}
		};
	} 
}