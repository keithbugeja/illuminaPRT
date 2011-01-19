//----------------------------------------------------------------------------------------------
//	Filename:	KDTreeMesh.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <stack>
#include <algorithm>

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
		/*
		template<class T>
		struct BIHNode
		{
			AxisAlignedBoundingBox BoundingBox;

			// Node Type
			TreeMeshNodeType Type;

			// Slab Axis
			int Axis;

			// Slab planes
			float Clip[2];

			// Only if an internal node
			BIHNode *m_pChild[2];

			// Only if a leaf
			List<T> TriangleList;

			// Constructor
			BIHNode() { }
			~BIHNode() { }
		};

		// Parallel BIH implementation
		template<class T, class U>
		class PBIHMesh
			: public ITriangleMesh<T, U>
		{
		protected:
			TreeMeshStatistics m_statistics;

			BIHNode<T*> m_rootNode;
			int m_nMaxLeafObjects;
			int m_nMaxTreeDepth;

		protected:
			BIHNode<T*>* RequestNode(void)
			{
				return new BIHNode<T*>();
			}

			int ReleaseNode(BIHNode<T*> *p_pNode)
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
			PBIHMesh(void)
				: m_nMaxLeafObjects(20)
				, m_nMaxTreeDepth(20)
			{ }

			PBIHMesh(int p_nMaxObjectsPerLeaf, int p_nMaxTreeDepth)
				: m_nMaxLeafObjects(p_nMaxObjectsPerLeaf)
				, m_nMaxTreeDepth(p_nMaxTreeDepth)
			{ }

			~PBIHMesh()
			{
				ReleaseNode(m_rootNode.m_pChild[0]);
				ReleaseNode(m_rootNode.m_pChild[1]);
			}

			boost::shared_ptr<ITriangleMesh<T, U>> CreateInstance(void) {
				return boost::shared_ptr<ITriangleMesh<T, U>>(new PBIHMesh<T, U>());
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

				//std::cout << "Compilation complete" << std::endl << ToString() << std::endl;

				return true;
			}

			bool Rebuild(void)
			{
				return true;
			}

			bool Update(void)
			{
				return true;
			}

			bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface, float &p_fTestDensity)
			{
				return Intersects(p_ray, p_fTime, p_surface);
			}

			bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
			{
				float in, out;

				if (!ITriangleMesh<T, U>::m_boundingBox.Intersects(p_ray, in, out))
					return false;

				in = Maths::Max(p_ray.Min, in);
				out = Maths::Min(p_ray.Max, out);

				Ray ray(p_ray);

				return Intersect_Recursive(&m_rootNode, ray, p_fTime, p_surface, in, out);
			}

			bool Intersects(const Ray &p_ray, float p_fTime)
			{
				float in, out;

				if (!ITriangleMesh<T, U>::m_boundingBox.Intersects(p_ray, in, out))
					return false;

				in = Maths::Max(p_ray.Min, in);
				out = Maths::Min(p_ray.Max, out);

				Ray ray(p_ray);

				return Intersect_Recursive(&m_rootNode, ray, p_fTime, in, out);
			}

			std::string ToString(void) const
			{
				return boost::str(boost::format("\nBIHMesh %s") % m_statistics.ToString());
			}

		protected:
			bool Intersect_Recursive(BIHNode<T*> *p_pNode, Ray &p_ray, float p_fTime, float p_fTIn, float p_fTOut)
			{
				BOOST_ASSERT(p_pNode != NULL);

				// Perform a preliminary test for early out
				float direction = p_ray.Direction[p_pNode->Axis],
					origin = p_ray.Origin[p_pNode->Axis],
					rcpDirection = 1.0f / direction;

				float clip_min = p_pNode->BoundingBox.GetMinExtent(p_pNode->Axis),
					clip_max = p_pNode->BoundingBox.GetMaxExtent(p_pNode->Axis),
					intercept_min = (clip_min - origin) * rcpDirection,
					intercept_max = (clip_max - origin) * rcpDirection;

				if ((intercept_min < p_fTIn && intercept_max < p_fTIn) || (intercept_min > p_fTOut && intercept_max > p_fTOut))
					return false;

				// Internal node
				if (p_pNode->Type == Internal)
				{
					BIHNode<T*> *pLeftNode,
						*pRightNode;

					float clip_0,
						clip_1;

					if (direction < 0.0f)
					{
						pLeftNode = p_pNode->m_pChild[1];
						pRightNode = p_pNode->m_pChild[0];

						clip_0 = p_pNode->Clip[1];
						clip_1 = p_pNode->Clip[0];
					}
					else
					{
						pLeftNode = p_pNode->m_pChild[0];
						pRightNode = p_pNode->m_pChild[1];

						clip_0 = p_pNode->Clip[0];
						clip_1 = p_pNode->Clip[1];
					}

					float intercept_0 = (clip_0 - origin) * rcpDirection,
						intercept_1 = (clip_1 - origin) * rcpDirection;

					// Ray falls within blank interval
					if (intercept_0 < p_fTIn && intercept_1 > p_fTOut)
						return false;

					// Ray starts behind left clip plane
					intercept_0 = Maths::Min(intercept_0, p_fTOut);

					if (p_fTIn < intercept_0)
					{
						if (Intersect_Recursive(pLeftNode, p_ray, p_fTime, p_fTIn, intercept_0))
							return true;
					}

					// Ray ends behind right clip plane
					intercept_1 = Maths::Max(intercept_1, p_fTIn);

					if (p_fTOut > intercept_1)
					{
						if (Intersect_Recursive(pRightNode, p_ray, p_fTime, intercept_1, p_fTOut))
							return true;
					}

					return false;
				}

				// Test geometry at leaf
				int count = (int)p_pNode->TriangleList.Size();

				if (count > 0)
				{
					for (int n = 0; n < count; n++)
					{
						if (p_pNode->TriangleList[n]->Intersects(p_ray, p_fTime))
							return true;
					}
				}

				return false;
			}

			bool Intersect_Recursive(BIHNode<T*> *p_pNode, Ray &p_ray, float p_fTime, DifferentialSurface &p_surface, float p_fTIn, float p_fTOut)
			{
				BOOST_ASSERT(p_pNode != NULL);

				bool bIntersect = false;

				// Perform a preliminary test for early out
				float direction = p_ray.Direction[p_pNode->Axis],
					origin = p_ray.Origin[p_pNode->Axis],
					rcpDirection = 1.0f / direction;

				float clip_min = p_pNode->BoundingBox.GetMinExtent(p_pNode->Axis),
					clip_max = p_pNode->BoundingBox.GetMaxExtent(p_pNode->Axis),
					intercept_min = (clip_min - origin) * rcpDirection,
					intercept_max = (clip_max - origin) * rcpDirection;

				if ((intercept_min < p_fTIn && intercept_max < p_fTIn) || (intercept_min > p_fTOut && intercept_max > p_fTOut))
					return false;

				// Internal node
				if (p_pNode->Type == Internal)
				{
					BIHNode<T*> *pLeftNode,
						*pRightNode;

					float clip_0,
						clip_1;

					if (direction < 0.0f)
					{
						pLeftNode = p_pNode->m_pChild[1];
						pRightNode = p_pNode->m_pChild[0];

						clip_0 = p_pNode->Clip[1];
						clip_1 = p_pNode->Clip[0];
					}
					else
					{
						pLeftNode = p_pNode->m_pChild[0];
						pRightNode = p_pNode->m_pChild[1];

						clip_0 = p_pNode->Clip[0];
						clip_1 = p_pNode->Clip[1];
					}

					float intercept_0 = (clip_0 - origin) * rcpDirection,
						intercept_1 = (clip_1 - origin) * rcpDirection;

					// Ray falls within blank interval
					if (intercept_0 < p_fTIn && intercept_1 > p_fTOut)
						return false;

					// Ray starts behind left clip plane
					intercept_0 = Maths::Min(intercept_0, p_fTOut);

					if (p_fTIn < intercept_0)
					{
						bIntersect |= Intersect_Recursive(pLeftNode, p_ray, p_fTime, p_surface, p_fTIn, intercept_0);

						if (bIntersect && p_surface.Distance < intercept_1)
							return true;
					}

					// Ray ends behind right clip plane
					intercept_1 = Maths::Max(intercept_1, p_fTIn);

					if (p_fTOut > intercept_1)
						bIntersect |= Intersect_Recursive(pRightNode, p_ray, p_fTime, p_surface, intercept_1, p_fTOut);

					return bIntersect;
				}

				// Test geometry at leaf
				int count = (int)p_pNode->TriangleList.Size();

				if (count > 0)
				{
					for (int n = 0; n < count; n++)
					{
						if (p_pNode->TriangleList[n]->Intersects(p_ray, p_fTime, p_surface))
						{
							p_ray.Max = Maths::Min(p_ray.Max, p_surface.Distance);
							bIntersect = true;
						}
					}
				}

				return bIntersect;
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

			int Split(const List<T*> &p_objectList, float p_fSplitPlane, int p_nAxis, float& p_leftClip, float& p_rightClip, List<T*> &p_outLeftList, List<T*> &p_outRightList)
			{
				int count = (int)p_objectList.Size();

				p_leftClip = -Maths::Maximum;
				p_rightClip = Maths::Maximum;

				float centrePoint;

				for (int n = 0; n < count; n++)
				{
					centrePoint = p_objectList[n]->GetBoundingVolume()->GetCentre()[p_nAxis];

					// Add to right child
					if (centrePoint > p_fSplitPlane)
					{
						p_outRightList.PushBack(p_objectList[n]);
						p_rightClip = Maths::Min(p_rightClip, p_objectList[n]->GetBoundingVolume()->GetMinExtent(p_nAxis));
					}
					else
					{
						p_outLeftList.PushBack(p_objectList[n]);
						p_leftClip = Maths::Max(p_leftClip, p_objectList[n]->GetBoundingVolume()->GetMaxExtent(p_nAxis));
					}
				}

				return (int)p_outLeftList.Size();
			}

			float FindPartitionPlane(const List<T*> &p_objectList, int p_nAxis, float p_fAxisLength, float p_fAxisCentre)
			{
				return 1.0f;
			}

			float FindPartitionPlane(const List<T*> &p_objectList, int p_nAxis)
			{
				//float fPartition = FindPartitionPlane_Centroid(p_objectList, p_nAxis);
				//float fPartition = FindPartitionPlane_SpatialMedian(p_objectList, p_nAxis);
				float fPartition = FindPartitionPlane_SAH(p_objectList, p_nAxis);

				//std::cout << "Partitioning at [" << fPartition << "]" << std::endl;
				//std::cout << "In terms of AABB : [" << (fPartition - p_aabb.GetMinExtent(p_nAxis)) / (p_aabb.GetMaxExtent(p_nAxis) - p_aabb.GetMinExtent(p_nAxis)) << "]" << std::endl;
				return fPartition;
			}

			float FindPartitionPlane_SpatialMedian(const List<T*> &p_objectList, int p_nAxis)
			{
				AxisAlignedBoundingBox aabb;
				ComputeBounds(p_objectList, aabb);
				return aabb.GetCentre()[p_nAxis];
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

			float FindPartitionPlane_SAH(const List<T*> &p_objectList, int p_nAxis)
			{
				const int Bins = 128;

				int minBins[Bins],
					maxBins[Bins];

				for (int j = 0; j < Bins; j++)
					maxBins[j] = minBins[j] = 0;

				AxisAlignedBoundingBox aabb;
				ComputeBounds(p_objectList, aabb);

				float extent = aabb.GetMaxExtent(p_nAxis) - aabb.GetMinExtent(p_nAxis),
					start = aabb.GetMinExtent(p_nAxis);

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
					//cost = (float)rightPrims / (float)(j) +
					//	(float)leftPrims / (float)(Bins - j);
					// + 1.0f - (count / (leftPrims + rightPrims)) ;

					if (cost < bestCost)
					{
						bestCost = cost;
						bestSplit = j;
					}
				}

				return start + (bestSplit * extent) / Bins;
			}

			void BuildHierarchy(BIHNode<T*> *p_pNode, List<T*> &p_objectList, int p_nAxis, int p_nDepth = 0)
			{
				ComputeBounds(p_objectList, ITriangleMesh<T, U>::m_boundingBox);
				const Vector3 &size = ITriangleMesh<T, U>::m_boundingBox.GetExtent();
				if (size.X > size.Y) p_nAxis = size.X > size.Z ? 0 : 2;
				else p_nAxis = size.Y > size.Z ? 1 : 2;

				AxisAlignedBoundingBox aabb(ITriangleMesh<T, U>::m_boundingBox);

				BuildHierarchy_S1(p_pNode, p_objectList, aabb, p_nAxis, 0);
				//BuildHierarchy_S2(p_pNode, p_objectList, p_nAxis, 0);
			}

			void BuildHierarchy_S1(BIHNode<T*> *p_pNode, List<T*> &p_objectList, AxisAlignedBoundingBox &p_aabb, int p_nAxis, int p_nDepth = 0)
			{
				// Update stats
				m_statistics.m_maxTreeDepth = Maths::Max(p_nDepth, m_statistics.m_maxTreeDepth);

				ComputeBounds(p_objectList, p_pNode->BoundingBox);

				// If we have enough objects, we consider this node a leaf
				if ((int)p_objectList.Size() <= m_nMaxLeafObjects || p_nDepth == m_nMaxTreeDepth)
				{
					//std::cout << "Adding leaf node [" << p_objectList.Size() << ", " << p_nDepth << "]" << std::endl;
					p_pNode->Type = Leaf;
					p_pNode->Axis = p_nAxis;
					p_pNode->TriangleList.PushBack(p_objectList);

					m_statistics.m_leafNodeCount++;
					m_statistics.m_minTreeDepth = Maths::Min(m_statistics.m_minTreeDepth, p_nDepth);
					m_statistics.m_minLeafTriangleCount = Maths::Min(m_statistics.m_minLeafTriangleCount, (int)p_objectList.Size());
					m_statistics.m_maxLeafTriangleCount = Maths::Max(m_statistics.m_maxLeafTriangleCount, (int)p_objectList.Size());
				}
				else
				{
					//std::cout << "Adding internal node [" << p_objectList.Size() << ", " << p_nDepth << "]" << std::endl;
					List<T*> leftList, rightList;

					//float fSplitPlane = FindPartitionPlane(p_objectList, p_nAxis);
					float fSplitPlane = p_aabb.GetCentre()[p_nAxis];
					Split(p_objectList, fSplitPlane, p_nAxis, p_pNode->Clip[0], p_pNode->Clip[1], leftList, rightList);

					AxisAlignedBoundingBox leftAABB(p_aabb),
						rightAABB(p_aabb);

					leftAABB.SetMaxExtent(p_nAxis, fSplitPlane);
					rightAABB.SetMinExtent(p_nAxis, fSplitPlane);

					p_pNode->Type = Internal;
					p_pNode->Axis = p_nAxis;

					p_pNode->m_pChild[0] = RequestNode();
					p_pNode->m_pChild[1] = RequestNode();

					int nAxis = (p_nAxis + 1) % 3,
						nDepth = p_nDepth + 1;

					if (leftList.Size() > 0)
						BuildHierarchy_S1(p_pNode->m_pChild[0], leftList, leftAABB, nAxis, nDepth);
					else
						p_pNode->Clip[0] = -Maths::Maximum;

					if (rightList.Size() > 0)
						BuildHierarchy_S1(p_pNode->m_pChild[1], rightList, rightAABB, nAxis, nDepth);
					else
						p_pNode->Clip[1] = Maths::Maximum;

					m_statistics.m_internalNodeCount++;
				}
			}

			void BuildHierarchy_S2(BIHNode<T*> *p_pNode, List<T*> &p_objectList, int p_nAxis, int p_nDepth = 0)
			{
				// Update stats
				m_statistics.m_maxTreeDepth = Maths::Min(p_nDepth, m_statistics.m_maxTreeDepth);

				ComputeBounds(p_objectList, p_pNode->BoundingBox);

				// If we have enough objects, we consider this node a leaf
				if ((int)p_objectList.Size() <= m_nMaxLeafObjects || p_nDepth == m_nMaxTreeDepth)
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
					List<T*> leftList, rightList;
					float fSplitPlane = FindPartitionPlane(p_objectList, p_nAxis);
					Split(p_objectList, fSplitPlane, p_nAxis, p_pNode->Clip[0], p_pNode->Clip[1], leftList, rightList);

					p_pNode->Type = Internal;
					p_pNode->Axis = p_nAxis;

					p_pNode->m_pChild[0] = RequestNode();
					p_pNode->m_pChild[1] = RequestNode();

					int nAxis = (p_nAxis + 1) % 3,
						nDepth = p_nDepth + 1;

					BuildHierarchy_S2(p_pNode->m_pChild[0], leftList, nAxis, nDepth);
					BuildHierarchy_S2(p_pNode->m_pChild[1], rightList, nAxis, nDepth);

					m_statistics.m_internalNodeCount++;
				}
			}
		};

		// Single-threaded BIH implementation
		template<class T, class U>
		class BIHMesh
			: public ITriangleMesh<T, U>
		{
		protected:
			TreeMeshStatistics m_statistics;

			BIHNode<T*> m_rootNode;
			int m_nMaxLeafObjects;
			int m_nMaxTreeDepth;

		protected:
			BIHNode<T*>* RequestNode(void)
			{
				return new BIHNode<T*>();
			}

			int ReleaseNode(BIHNode<T*> *p_pNode)
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
			BIHMesh(void)
				: m_nMaxLeafObjects(20)
				, m_nMaxTreeDepth(20)
			{ }

			BIHMesh(int p_nMaxObjectsPerLeaf, int p_nMaxTreeDepth)
				: m_nMaxLeafObjects(p_nMaxObjectsPerLeaf)
				, m_nMaxTreeDepth(p_nMaxTreeDepth)
			{ }

			~BIHMesh()
			{
				ReleaseNode(m_rootNode.m_pChild[0]);
				ReleaseNode(m_rootNode.m_pChild[1]);
			}

			boost::shared_ptr<ITriangleMesh<T, U>> CreateInstance(void) {
				return boost::shared_ptr<ITriangleMesh<T, U>>(new BIHMesh<T, U>());
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

				//std::cout << "Compilation complete" << std::endl << ToString() << std::endl;

				return true;
			}

			bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface, float &p_fTestDensity)
			{
				return Intersects(p_ray, p_fTime, p_surface);
			}

			bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
			{
				float in, out;

				if (!ITriangleMesh<T, U>::m_boundingBox.Intersects(p_ray, in, out))
					return false;

				in = Maths::Max(p_ray.Min, in);
				out = Maths::Min(p_ray.Max, out);

				Ray ray(p_ray);

				return Intersect_Recursive(&m_rootNode, ray, p_fTime, p_surface, in, out);
			}

			bool Intersects(const Ray &p_ray, float p_fTime)
			{
				float in, out;

				if (!ITriangleMesh<T, U>::m_boundingBox.Intersects(p_ray, in, out))
					return false;

				in = Maths::Max(p_ray.Min, in);
				out = Maths::Min(p_ray.Max, out);

				Ray ray(p_ray);

				return Intersect_Recursive(&m_rootNode, ray, p_fTime, in, out);
			}

			std::string ToString(void) const
			{
				return boost::str(boost::format("\nBIHMesh %s") % m_statistics.ToString());
			}

		protected:
			bool Intersect_Recursive(BIHNode<T*> *p_pNode, Ray &p_ray, float p_fTime, float p_fTIn, float p_fTOut)
			{
				BOOST_ASSERT(p_pNode != NULL);

				// Perform a preliminary test for early out
				float direction = p_ray.Direction[p_pNode->Axis],
					origin = p_ray.Origin[p_pNode->Axis],
					rcpDirection = 1.0f / direction;

				float clip_min = p_pNode->BoundingBox.GetMinExtent(p_pNode->Axis),
					clip_max = p_pNode->BoundingBox.GetMaxExtent(p_pNode->Axis),
					intercept_min = (clip_min - origin) * rcpDirection,
					intercept_max = (clip_max - origin) * rcpDirection;

				if ((intercept_min < p_fTIn && intercept_max < p_fTIn) || (intercept_min > p_fTOut && intercept_max > p_fTOut))
					return false;

				// Internal node
				if (p_pNode->Type == Internal)
				{
					BIHNode<T*> *pLeftNode,
						*pRightNode;

					float clip_0,
						clip_1;

					if (direction < 0.0f)
					{
						pLeftNode = p_pNode->m_pChild[1];
						pRightNode = p_pNode->m_pChild[0];

						clip_0 = p_pNode->Clip[1];
						clip_1 = p_pNode->Clip[0];
					}
					else
					{
						pLeftNode = p_pNode->m_pChild[0];
						pRightNode = p_pNode->m_pChild[1];

						clip_0 = p_pNode->Clip[0];
						clip_1 = p_pNode->Clip[1];
					}

					float intercept_0 = (clip_0 - origin) * rcpDirection,
						intercept_1 = (clip_1 - origin) * rcpDirection;

					// Ray falls within blank interval
					if (intercept_0 < p_fTIn && intercept_1 > p_fTOut)
						return false;

					// Ray starts behind left clip plane
					intercept_0 = Maths::Min(intercept_0, p_fTOut);

					if (p_fTIn < intercept_0)
					{
						if (Intersect_Recursive(pLeftNode, p_ray, p_fTime, p_fTIn, intercept_0))
							return true;
					}

					// Ray ends behind right clip plane
					intercept_1 = Maths::Max(intercept_1, p_fTIn);

					if (p_fTOut > intercept_1)
					{
						if (Intersect_Recursive(pRightNode, p_ray, p_fTime, intercept_1, p_fTOut))
							return true;
					}

					return false;
				}

				// Test geometry at leaf
				int count = (int)p_pNode->TriangleList.Size();

				if (count > 0)
				{
					for (int n = 0; n < count; n++)
					{
						if (p_pNode->TriangleList[n]->Intersects(p_ray, p_fTime))
							return true;
					}
				}

				return false;
			}

			bool Intersect_Recursive(BIHNode<T*> *p_pNode, Ray &p_ray, float p_fTime, DifferentialSurface &p_surface, float p_fTIn, float p_fTOut)
			{
				BOOST_ASSERT(p_pNode != NULL);

				bool bIntersect = false;

				// Perform a preliminary test for early out
				float direction = p_ray.Direction[p_pNode->Axis],
					origin = p_ray.Origin[p_pNode->Axis],
					rcpDirection = 1.0f / direction;

				float clip_min = p_pNode->BoundingBox.GetMinExtent(p_pNode->Axis),
					clip_max = p_pNode->BoundingBox.GetMaxExtent(p_pNode->Axis),
					intercept_min = (clip_min - origin) * rcpDirection,
					intercept_max = (clip_max - origin) * rcpDirection;

				if ((intercept_min < p_fTIn && intercept_max < p_fTIn) || (intercept_min > p_fTOut && intercept_max > p_fTOut))
					return false;

				// Internal node
				if (p_pNode->Type == Internal)
				{
					BIHNode<T*> *pLeftNode,
						*pRightNode;

					float clip_0,
						clip_1;

					if (direction < 0.0f)
					{
						pLeftNode = p_pNode->m_pChild[1];
						pRightNode = p_pNode->m_pChild[0];

						clip_0 = p_pNode->Clip[1];
						clip_1 = p_pNode->Clip[0];
					}
					else
					{
						pLeftNode = p_pNode->m_pChild[0];
						pRightNode = p_pNode->m_pChild[1];

						clip_0 = p_pNode->Clip[0];
						clip_1 = p_pNode->Clip[1];
					}

					float intercept_0 = (clip_0 - origin) * rcpDirection,
						intercept_1 = (clip_1 - origin) * rcpDirection;

					// Ray falls within blank interval
					if (intercept_0 < p_fTIn && intercept_1 > p_fTOut)
						return false;

					// Ray starts behind left clip plane
					intercept_0 = Maths::Min(intercept_0, p_fTOut);

					if (p_fTIn < intercept_0)
					{
						bIntersect |= Intersect_Recursive(pLeftNode, p_ray, p_fTime, p_surface, p_fTIn, intercept_0);

						if (bIntersect && p_surface.Distance < intercept_1)
							return true;
					}

					// Ray ends behind right clip plane
					intercept_1 = Maths::Max(intercept_1, p_fTIn);

					if (p_fTOut > intercept_1)
						bIntersect |= Intersect_Recursive(pRightNode, p_ray, p_fTime, p_surface, intercept_1, p_fTOut);

					return bIntersect;
				}

				// Test geometry at leaf
				int count = (int)p_pNode->TriangleList.Size();

				if (count > 0)
				{
					for (int n = 0; n < count; n++)
					{
						if (p_pNode->TriangleList[n]->Intersects(p_ray, p_fTime, p_surface))
						{
							p_ray.Max = Maths::Min(p_ray.Max, p_surface.Distance);
							bIntersect = true;
						}
					}
				}

				return bIntersect;
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

			int Split(const List<T*> &p_objectList, float p_fSplitPlane, int p_nAxis, float& p_leftClip, float& p_rightClip, List<T*> &p_outLeftList, List<T*> &p_outRightList)
			{
				int count = (int)p_objectList.Size();

				p_leftClip = -Maths::Maximum;
				p_rightClip = Maths::Maximum;

				float centrePoint;

				for (int n = 0; n < count; n++)
				{
					centrePoint = p_objectList[n]->GetBoundingVolume()->GetCentre()[p_nAxis];

					// Add to right child
					if (centrePoint > p_fSplitPlane)
					{
						p_outRightList.PushBack(p_objectList[n]);
						p_rightClip = Maths::Min(p_rightClip, p_objectList[n]->GetBoundingVolume()->GetMinExtent(p_nAxis));
					}
					else
					{
						p_outLeftList.PushBack(p_objectList[n]);
						p_leftClip = Maths::Max(p_leftClip, p_objectList[n]->GetBoundingVolume()->GetMaxExtent(p_nAxis));
					}
				}

				return (int)p_outLeftList.Size();
			}

			float FindPartitionPlane(const List<T*> &p_objectList, int p_nAxis, float p_fAxisLength, float p_fAxisCentre)
			{
				return 1.0f;
			}

			float FindPartitionPlane(const List<T*> &p_objectList, int p_nAxis)
			{
				//float fPartition = FindPartitionPlane_Centroid(p_objectList, p_nAxis);
				//float fPartition = FindPartitionPlane_SpatialMedian(p_objectList, p_nAxis);
				float fPartition = FindPartitionPlane_SAH(p_objectList, p_nAxis);

				//std::cout << "Partitioning at [" << fPartition << "]" << std::endl;
				//std::cout << "In terms of AABB : [" << (fPartition - p_aabb.GetMinExtent(p_nAxis)) / (p_aabb.GetMaxExtent(p_nAxis) - p_aabb.GetMinExtent(p_nAxis)) << "]" << std::endl;
				return fPartition;
			}

			float FindPartitionPlane_SpatialMedian(const List<T*> &p_objectList, int p_nAxis)
			{
				AxisAlignedBoundingBox aabb;
				ComputeBounds(p_objectList, aabb);
				return aabb.GetCentre()[p_nAxis];
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

			float FindPartitionPlane_SAH(const List<T*> &p_objectList, int p_nAxis)
			{
				const int Bins = 128;

				int minBins[Bins],
					maxBins[Bins];

				for (int j = 0; j < Bins; j++)
					maxBins[j] = minBins[j] = 0;

				AxisAlignedBoundingBox aabb;
				ComputeBounds(p_objectList, aabb);

				float extent = aabb.GetMaxExtent(p_nAxis) - aabb.GetMinExtent(p_nAxis),
					start = aabb.GetMinExtent(p_nAxis);

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
					//cost = (float)rightPrims / (float)(j) +
					//	(float)leftPrims / (float)(Bins - j);
					// + 1.0f - (count / (leftPrims + rightPrims)) ;

					if (cost < bestCost)
					{
						bestCost = cost;
						bestSplit = j;
					}
				}

				return start + (bestSplit * extent) / Bins;
			}

			void BuildHierarchy(BIHNode<T*> *p_pNode, List<T*> &p_objectList, int p_nAxis, int p_nDepth = 0)
			{
				ComputeBounds(p_objectList, ITriangleMesh<T, U>::m_boundingBox);
				Vector3 &size = ITriangleMesh<T, U>::m_boundingBox.GetExtent();
				if (size.X > size.Y) p_nAxis = size.X > size.Z ? 0 : 2;
				else p_nAxis = size.Y > size.Z ? 1 : 2;

				AxisAlignedBoundingBox aabb(ITriangleMesh<T, U>::m_boundingBox);

				BuildHierarchy_S1(p_pNode, p_objectList, aabb, p_nAxis, 0);
				//BuildHierarchy_S2(p_pNode, p_objectList, p_nAxis, 0);
			}

			void BuildHierarchy_S1(BIHNode<T*> *p_pNode, List<T*> &p_objectList, AxisAlignedBoundingBox &p_aabb, int p_nAxis, int p_nDepth = 0)
			{
				// Update stats
				m_statistics.m_maxTreeDepth = Maths::Min(p_nDepth, m_statistics.m_maxTreeDepth);

				ComputeBounds(p_objectList, p_pNode->BoundingBox);

				// If we have enough objects, we consider this node a leaf
				if ((int)p_objectList.Size() <= m_nMaxLeafObjects || p_nDepth == m_nMaxTreeDepth)
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
					List<T*> leftList, rightList;

					//float fSplitPlane = FindPartitionPlane(p_objectList, p_nAxis);
					float fSplitPlane = p_aabb.GetCentre()[p_nAxis];
					Split(p_objectList, fSplitPlane, p_nAxis, p_pNode->Clip[0], p_pNode->Clip[1], leftList, rightList);

					AxisAlignedBoundingBox leftAABB(p_aabb),
						rightAABB(p_aabb);

					leftAABB.SetMaxExtent(p_nAxis, fSplitPlane);
					rightAABB.SetMinExtent(p_nAxis, fSplitPlane);

					p_pNode->Type = Internal;
					p_pNode->Axis = p_nAxis;

					p_pNode->m_pChild[0] = RequestNode();
					p_pNode->m_pChild[1] = RequestNode();

					int nAxis = (p_nAxis + 1) % 3,
						nDepth = p_nDepth + 1;

					if (leftList.Size() > 0)
						BuildHierarchy_S1(p_pNode->m_pChild[0], leftList, leftAABB, nAxis, nDepth);
					else
						p_pNode->Clip[0] = -Maths::Maximum;

					if (rightList.Size() > 0)
						BuildHierarchy_S1(p_pNode->m_pChild[1], rightList, rightAABB, nAxis, nDepth);
					else
						p_pNode->Clip[1] = Maths::Maximum;

					m_statistics.m_internalNodeCount++;
				}
			}

			void BuildHierarchy_S2(BIHNode<T*> *p_pNode, List<T*> &p_objectList, int p_nAxis, int p_nDepth = 0)
			{
				// Update stats
				m_statistics.m_maxTreeDepth = Maths::Min(p_nDepth, m_statistics.m_maxTreeDepth);

				ComputeBounds(p_objectList, p_pNode->BoundingBox);

				// If we have enough objects, we consider this node a leaf
				if ((int)p_objectList.Size() <= m_nMaxLeafObjects || p_nDepth == m_nMaxTreeDepth)
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
					List<T*> leftList, rightList;
					float fSplitPlane = FindPartitionPlane(p_objectList, p_nAxis);
					Split(p_objectList, fSplitPlane, p_nAxis, p_pNode->Clip[0], p_pNode->Clip[1], leftList, rightList);

					p_pNode->Type = Internal;
					p_pNode->Axis = p_nAxis;

					p_pNode->m_pChild[0] = RequestNode();
					p_pNode->m_pChild[1] = RequestNode();

					int nAxis = (p_nAxis + 1) % 3,
						nDepth = p_nDepth + 1;

					BuildHierarchy_S2(p_pNode->m_pChild[0], leftList, nAxis, nDepth);
					BuildHierarchy_S2(p_pNode->m_pChild[1], rightList, nAxis, nDepth);

					m_statistics.m_internalNodeCount++;
				}
			}
		};
		*/
	}
}
