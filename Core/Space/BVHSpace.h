//----------------------------------------------------------------------------------------------
//	Filename:	BVHSpace.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>

#include "Geometry/BoundingBox.h"
#include "Space/Space.h"

namespace Illumina 
{
	namespace Core
	{
		/* This section has been commented, until the ongoing 
		 * changes in the ISpace interface are complete.
		 */

		/*
		struct BVHSpaceNode
		{
			static const int Leaf		= 0x00;
			static const int Internal	= 0x01;

			AxisAlignedBoundingBox BoundingBox;

			int NodeType;

			BVHSpaceNode *m_pChild[2];

			List<IPrimitive*> PrimitiveList;

			BVHSpaceNode() { }
			~BVHSpaceNode() { }

			void Intersects(const Ray &p_ray, List<IPrimitive*> &p_outList) const
			{
				if (BoundingBox.Intersects(p_ray) == false) return;

				if (NodeType == Internal)
				{
					m_pChild[0]->Intersects(p_ray, p_outList);
					m_pChild[1]->Intersects(p_ray, p_outList);
				}
				else
					p_outList.PushBack(PrimitiveList);
			}

			void Intersects(const Frustum& p_frustum, List<IPrimitive*> &p_outList) const
			{
				if (NodeType == Internal)
				{
						m_pChild[0]->Intersects(p_frustum, p_outList);
						m_pChild[1]->Intersects(p_frustum, p_outList);
				}
				else
					p_outList.PushBack(PrimitiveList);
			}
		};

		class BVHSpace 
			: public ISpace
		{
		protected:
			int m_nMaxLeafObjects;
			BVHSpaceNode m_rootNode;

		public:
			void Initialise(void) {
				m_nMaxLeafObjects = 5; 
			}
			
			void Build(void) 
			{
				int objectCount = (int)PrimitiveList.Size();
				List<IPrimitive*> primitiveList(objectCount);

				for (int idx = 0; idx < objectCount; idx++) {
					primitiveList.PushBack(PrimitiveList[idx]);
				}

				BuildHierarchy(&m_rootNode, primitiveList, 0);
			}

			void Update(void) 
			{ }

			bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface& p_surface, float& p_fTestDensity) 
			{
				bool bIntersect = false;

				Ray ray(p_ray);
				List<IPrimitive*> intersectionList;

				// Find possible intersected primitives
				m_rootNode.Intersects(ray, intersectionList);
				
				// No intersection if zero primitives returned 
				if (intersectionList.Size() == 0) 
					return false;

				// Go through list and refine intersection testing
				for (int idx = 0, count = (int)intersectionList.Size(); idx < count; idx++)
				{
					const IPrimitive* pPrimitive = intersectionList[idx];

					if (pPrimitive->Intersect(ray, p_fTime, p_surface, p_fTestDensity))
					{
						ray.Max = p_surface.Distance;
						bIntersect = true;
					}
				}

				return bIntersect;
			}


			bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface) const
			{
				bool bIntersect = false;

				Ray ray(p_ray);
				List<IPrimitive*> intersectionList;

				// Find possible intersected primitives
				m_rootNode.Intersects(ray, intersectionList);
				
				// No intersection if zero primitives returned 
				if (intersectionList.Size() == 0) 
					return false;

				// Go through list and refine intersection testing
				for (int idx = 0, count = (int)intersectionList.Size(); idx < count; idx++)
				{
					const IPrimitive* pPrimitive = intersectionList[idx];

					if (pPrimitive->Intersect(ray, p_fTime, p_surface))
					{
						ray.Max = p_surface.Distance;
						bIntersect = true;
					}
				}

				return bIntersect;
			}

			bool Intersects(const Ray &p_ray, float p_fTime) const
			{
				List<IPrimitive*> intersectionList;
				m_rootNode.Intersects(p_ray, intersectionList);
				
				for (int idx = 0, count = (int)intersectionList.Size(); idx < count; idx++)
				{
					if (intersectionList[idx]->Intersect(p_ray, p_fTime))
						return true;
				}

				return false;		
			}
		
		protected:
			void ComputeNodeBounds(const List<IPrimitive*> &p_primitiveList, AxisAlignedBoundingBox &p_aabb)
			{
				p_aabb.Invalidate();

				if (p_primitiveList.Size() > 0)
				{
					BoundingVolumePtr pWorldBounds(p_primitiveList[0]->GetWorldBounds());

					p_aabb.ComputeFromVolume(*pWorldBounds);

					for (int idx = 1, count = (int)p_primitiveList.Size(); idx < count; idx++) 
					{
						pWorldBounds = p_primitiveList[idx]->GetWorldBounds();
						p_aabb.Union(*pWorldBounds);
					}
				}
			}

			void PartitionObjects(const List<IPrimitive*> &p_objectList, int p_nPartitionPlane, List<IPrimitive*> &p_outLeftList, List<IPrimitive*> &p_outRightList)
			{
				// Initialise centroid for object cluster
				Vector3 Centroid(0.0f);

				// Calculate cluster centroid
				int objectCount = (int)p_objectList.Size();
				BoundingVolumePtr pBV;

				for (int idx = 0; idx < objectCount; idx++)
				{
					pBV = p_objectList[idx]->GetWorldBounds();
					Vector3::Add(Centroid, pBV->GetCentre(), Centroid);
				}

				Centroid /= (float)objectCount;

				// Clear left and right lists
				p_outLeftList.Clear();
				p_outRightList.Clear();

				// Get partitioning value
				float fPartition = Centroid[p_nPartitionPlane];

				for (int idx = 0; idx < objectCount; idx++)
				{
					pBV = p_objectList[idx]->GetWorldBounds();
					
					if (pBV->GetCentre()[p_nPartitionPlane] > fPartition)
						p_outRightList.PushBack(p_objectList[idx]);
					else
						p_outLeftList.PushBack(p_objectList[idx]);
				}
			}

			void BuildHierarchy(BVHSpaceNode *p_node, List<IPrimitive*> &p_objectList, int p_nPartitionPlane)
			{
				// Compute the node bounds for the given object list
				ComputeNodeBounds(p_objectList, p_node->BoundingBox);

				// If we have enough object, we consider this node a leaf
				if ((int)p_objectList.Size() < m_nMaxLeafObjects)
				{
					p_node->NodeType = BVHSpaceNode::Leaf; 
					p_node->PrimitiveList.PushBack(p_objectList);
				}
				else
				{
					// Parition objects in two list, and recurse
					p_node->NodeType = BVHSpaceNode::Internal;

					List<IPrimitive*> leftList, rightList; 
					PartitionObjects(p_objectList, p_nPartitionPlane, leftList, rightList);

					p_node->m_pChild[0] = new BVHSpaceNode();
					p_node->m_pChild[1] = new BVHSpaceNode();

					int nPartitionPlane = (p_nPartitionPlane + 1) % 3;

					BuildHierarchy(p_node->m_pChild[0], leftList, nPartitionPlane);
					BuildHierarchy(p_node->m_pChild[1], rightList, nPartitionPlane);
				}
			}
		};
		*/
	} 
}