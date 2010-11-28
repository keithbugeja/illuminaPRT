//----------------------------------------------------------------------------------------------
//	Filename:	HybridMesh.h

//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
#pragma once

#include <algorithm>
#include <stack>
#include <map>

#include <boost/shared_ptr.hpp>
#include "Shape/TriangleMesh.h"
#include "Shape/TreeMesh.h"

namespace Illumina
{
	namespace Core
	{
		// Bounding volume hierarchy node
		template<class T>
		struct GridCell
		{
			// Node bounding box
			AxisAlignedBoundingBox BoundingBox;

			// Only if a leaf
			List<T> TriangleList;

			// Construction and destruction
			GridCell() { }
			~GridCell() { }
		};

		// Bounding Volume Hierarchy Mesh
		template<class T, class U>
		class GridMesh
			: public ITriangleMesh<T, U>
		{
		protected:
			std::map<int, GridCell<T*>*> m_grid;
			AxisAlignedBoundingBox m_gridCell;

			Vector3 m_gridOrigin,
				m_gridCellSize;

			int M[3];

		protected:
			//----------------------------------------------------------------------------------------------
			// Methods for requesting and freeing nodes
			//----------------------------------------------------------------------------------------------
			GridCell<T*>* RequestCell(void)
			{
				return new GridCell<T*>();
			}

			int ReleaseCell(GridCell<T*> *p_pCell)
			{
				Safe_Delete(p_pCell);
				return 1;
			}

			//----------------------------------------------------------------------------------------------
			// Cell index management
			//----------------------------------------------------------------------------------------------
			inline int ComputeHash(int p_x, int p_y, int p_z)
			{
				//std::cout << "Hashing :: " << p_x << ", " << p_y << ", " << p_z << std::endl;

				return (p_x & 0x03FF) | ((p_y & 0x03FF) << 10) | ((p_z & 0x03FF) << 20);
			}

			inline int ComputeHash(const Vector3& p_cellIndex)
			{
				return ComputeHash((int)p_cellIndex.X, (int)p_cellIndex.Y, (int)p_cellIndex.Z);
			}

			inline int ComputeHashFromPosition(float p_x, float p_y, float p_z)
			{
				int x = (int)Maths::Floor((p_x - m_gridOrigin.X + Maths::Epsilon) / m_gridCellSize.X),
					y = (int)Maths::Floor((p_y - m_gridOrigin.Y + Maths::Epsilon) / m_gridCellSize.Y),
					z = (int)Maths::Floor((p_z - m_gridOrigin.Z + Maths::Epsilon) / m_gridCellSize.Z);

				return ComputeHash(x, y, z);
			}

			inline int ComputeHashFromPosition(const Vector3 &p_position)
			{
				return ComputeHashFromPosition(p_position.X, p_position.Y, p_position.Z);
			}

			//----------------------------------------------------------------------------------------------
			// Cell bounding volume
			//----------------------------------------------------------------------------------------------
			inline void ComputeCellAABB(int p_x, int p_y, int p_z, AxisAlignedBoundingBox& p_aabb)
			{
				Vector3 minExtent(m_gridOrigin.X + p_x * m_gridCellSize.X,
					m_gridOrigin.Y + p_y * m_gridCellSize.Y,
					m_gridOrigin.Z + p_z * m_gridCellSize.Z);

				Vector3 maxExtent = minExtent + m_gridCellSize;

				p_aabb.SetExtents(minExtent, maxExtent);
			}

			inline void ComputeCellAABB(Vector3 p_cellIndex, AxisAlignedBoundingBox& p_aabb)
			{
				ComputeCellAABB((int)p_cellIndex.X, (int)p_cellIndex.Y, (int)p_cellIndex.Z, p_aabb);
			}

			//----------------------------------------------------------------------------------------------
			// Grid-World space transformations
			//----------------------------------------------------------------------------------------------
			inline void TransformToGridSpace(const Vector3& p_worldPosition, Vector3& p_gridPosition)
			{
				Vector3::Subtract(p_worldPosition, m_gridOrigin, p_gridPosition);
			}

			inline void TransformToGridCellIndex(const Vector3& p_worldPosition, Vector3& p_gridPosition)
			{
				p_gridPosition.X = Maths::Floor((p_worldPosition.X - m_gridOrigin.X + Maths::Epsilon) / m_gridCellSize.X);
				p_gridPosition.Y = Maths::Floor((p_worldPosition.Y - m_gridOrigin.Y + Maths::Epsilon) / m_gridCellSize.Y);
				p_gridPosition.Z = Maths::Floor((p_worldPosition.Z - m_gridOrigin.Z + Maths::Epsilon) / m_gridCellSize.Z);

				//std::cout << "Position = " << (p_worldPosition - m_gridOrigin).ToString() << std::endl;
				//std::cout << "Index    = " << p_gridPosition.ToString() << std::endl;
			}
			//----------------------------------------------------------------------------------------------

		public:
			GridMesh(void)
			{ }

			~GridMesh()
			{
			}

			boost::shared_ptr<ITriangleMesh<T, U>> CreateInstance(void) {
				return boost::shared_ptr<ITriangleMesh<T, U>>(new GridMesh<T, U>());
			}

			bool Compile(void)
			{
				// Create a list of pointers to indexed triangles
				int objectCount = (int)ITriangleMesh<T, U>::TriangleList.Size();
				List<T*> triangleList(objectCount);

				for (int idx = 0; idx < objectCount; idx++) {
					triangleList.PushBack(&ITriangleMesh<T, U>::TriangleList[idx]);
				}

				// Compute bounds of grid
				ComputeBounds(triangleList, ITriangleMesh<T, U>::m_boundingBox);

				// Grid origin is equal to the minextent of its bounding box
				m_gridOrigin = ITriangleMesh<T, U>::m_boundingBox.GetMinExtent();

				// Number of cells is equal to (rN / V) ^ 1/3
				Vector3 dimensions = ITriangleMesh<T, U>::m_boundingBox.GetSize();
				//float gridVolume = dimensions.X * dimensions.Y * dimensions.Z,
				//		coeff = 1.0, factor = Maths::Pow((coeff * triangleList.Size()) / gridVolume, 1.0f / 3.0f);
				float factor = 0.01f;

				for (int i = 0; i < 3; i++)
				{
					M[i] = 40; //(int)(factor * dimensions[i]);
					m_gridCellSize[i] = dimensions[i] / M[i];
				}

				m_gridCell.SetMinExtent(Vector3::Zero);
				m_gridCell.SetMaxExtent(m_gridCellSize);

				// Build Grid
				BuildGrid();

				return true;
			}

			bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
			{
				Ray ray(p_ray);

				return Intersect_(ray, p_fTime, p_surface);
			}

			bool Intersects(const Ray &p_ray, float p_fTime)
			{
				Ray ray(p_ray);

				return Intersect_(ray, p_fTime);
			}

			std::string ToString(void) const
			{
				//return boost::str(boost::format("\nBVHMesh %s") % m_statistics.ToString());
				return "Grid Cell";
			}

		protected:
			bool Intersect_(Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
			{
				Vector3 cellIndex, step,
					bounds, next, delta,
					startPosition,
					currentPosition;

				float in, out;

				// If the ray doesn't collide with the object's bounding box, exit early
				if (!ITriangleMesh<T, U>::m_boundingBox.Intersects(p_ray, in, out))
					return false;

				//
				// Find voxel containing ray origin
				//
				if (in < 0)
				{
					in = 0;
					startPosition = p_ray.Origin;
				}
				else
					p_ray.PointAlongRay(in + Maths::Epsilon, startPosition);

				TransformToGridCellIndex(startPosition, cellIndex);
				Vector3::Subtract(startPosition, m_gridOrigin, currentPosition);

				for (int i = 0; i < 3; i++)
				{
					//
					// Initialise step according to ray direction
					//
					step[i] = Maths::ISgn(p_ray.Direction[i]);

					//
					// Determine tMax for next crossings
					//
					bounds[i] = (cellIndex[i] + (step[i] > 0)) * m_gridCellSize[i];

					if (Maths::FAbs(p_ray.Direction[i]) > Maths::Epsilon)
						next[i] = Maths::Abs((bounds[i] - currentPosition[i]) / p_ray.Direction[i]);
					else
						next[i] = Maths::Maximum;

					//
					// Determine tDelta cell dimensions in terms of t
					//
					if (Maths::FAbs(p_ray.Direction[i]) > Maths::Epsilon)
						delta[i] = Maths::Abs(m_gridCellSize[i] / p_ray.Direction[i]);
					else
						delta[i] = Maths::Maximum;
				}

				bool bIntersect = false;

				int argmin = next.ArgMinAbsComponent(),
					hash;

				while (true)
				{
					in += delta[argmin];
					hash = ComputeHash(cellIndex);

					if (m_grid.find(hash) != m_grid.end())
					{
						GridCell<T*>* pGridCell = m_grid[hash];

						int count = 0;

						if ((count = (int)pGridCell->TriangleList.Size()) > 0)
						{
							for (int n = 0; n < count; n++)
							{
								if (pGridCell->TriangleList[n]->Intersects(p_ray, p_fTime, p_surface))
								{
									if (p_surface.Distance <= in)
									{
										p_ray.Max = Maths::Min(p_ray.Max, p_surface.Distance);
										bIntersect = true;
									}
								}
							}
						}
					}

					if (bIntersect) return true;

					argmin = next.ArgMinAbsComponent();
					cellIndex[argmin] += step[argmin];

					if (cellIndex[argmin] < 0 || cellIndex[argmin] > M[argmin])
						break;

					next[argmin] += delta[argmin];
				}

				return false;
			}

			bool Intersect_(Ray &p_ray, float p_fTime)
			{
				Vector3 cellIndex, step,
					bounds, next, delta,
					startPosition,
					currentPosition;

				float in, out;

				// If the ray doesn't collide with the object's bounding box, exit early
				if (!ITriangleMesh<T, U>::m_boundingBox.Intersects(p_ray, in, out))
					return false;

				//
				// Find voxel containing ray origin
				//
				if (in < 0)
				{
					in = 0;
					startPosition = p_ray.Origin;
				}
				else
					p_ray.PointAlongRay(in + Maths::Epsilon, startPosition);

				TransformToGridCellIndex(startPosition, cellIndex);
				Vector3::Subtract(startPosition, m_gridOrigin, currentPosition);

				for (int i = 0; i < 3; i++)
				{
					//
					// Initialise step according to ray direction
					//
					step[i] = Maths::ISgn(p_ray.Direction[i]);

					//
					// Determine tMax for next crossings
					//
					bounds[i] = (cellIndex[i] + (step[i] > 0)) * m_gridCellSize[i];

					if (Maths::FAbs(p_ray.Direction[i]) > Maths::Epsilon)
						next[i] = Maths::Abs((bounds[i] - currentPosition[i]) / p_ray.Direction[i]);
					else
						next[i] = Maths::Maximum;

					//
					// Determine tDelta cell dimensions in terms of t
					//
					if (Maths::FAbs(p_ray.Direction[i]) > Maths::Epsilon)
						delta[i] = Maths::Abs(m_gridCellSize[i] / p_ray.Direction[i]);
					else
						delta[i] = Maths::Maximum;
				}

				int argmin = next.ArgMinAbsComponent(),
					hash;

				while (true)
				{
					in += delta[argmin];
					hash = ComputeHash(cellIndex);

					if (m_grid.find(hash) != m_grid.end())
					{
						GridCell<T*>* pGridCell = m_grid[hash];

						int count = 0;

						if ((count = (int)pGridCell->TriangleList.Size()) > 0)
						{
							for (int n = 0; n < count; n++)
							{
								if (pGridCell->TriangleList[n]->Intersects(p_ray, p_fTime))
									return true;
							}
						}
					}

					argmin = next.ArgMinAbsComponent();
					cellIndex[argmin] += step[argmin];

					if (cellIndex[argmin] < 0 || cellIndex[argmin] > M[argmin])
						break;

					next[argmin] += delta[argmin];
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

			void BuildGrid(void)
			{
				Vector3 cellStart;

				Interval intervalX,
					intervalY,
					intervalZ;

				int cellCount = 0;

				// Find cell span of object
				for (int index = 0; index < ITriangleMesh<T, U>::TriangleList.Size(); index++)
				{
					IBoundingVolume* pBoundingVolume = ITriangleMesh<T, U>::TriangleList[index].GetBoundingVolume();

					pBoundingVolume->ProjectToInterval(Vector3::UnitXPos, intervalX);
					pBoundingVolume->ProjectToInterval(Vector3::UnitYPos, intervalY);
					pBoundingVolume->ProjectToInterval(Vector3::UnitZPos, intervalZ);

					//std::cout << "Actual   X : " << pBoundingVolume->GetMinExtent(0) << " : " << pBoundingVolume->GetMaxExtent(0) << std::endl;
					//std::cout << "Interval X : " << intervalX.Min << " : " << intervalX.Max << std::endl;
					//std::cout << "Actual   Y : " << pBoundingVolume->GetMinExtent(1) << " : " << pBoundingVolume->GetMaxExtent(1) << std::endl;
					//std::cout << "Interval Y : " << intervalY.Min << " : " << intervalY.Max << std::endl;
					//std::cout << "Actual   Z : " << pBoundingVolume->GetMinExtent(2) << " : " << pBoundingVolume->GetMaxExtent(2) << std::endl;
					//std::cout << "Interval Z : " << intervalZ.Min << " : " << intervalZ.Max << std::endl << std::endl;

					//std::cout << "Box : " << m_boundingBox.GetMinExtent().ToString() << " , " << m_boundingBox.GetMaxExtent().ToString() << std::endl;
					//std::cout << "Intervals : [" << intervalX.Min << " : " << intervalX.Max << "], [" << intervalY.Min << " : " << intervalY.Max << "], ["<< intervalZ.Min << " : " << intervalZ.Max << "]" << std::endl;

					for (float z = intervalZ.Min; z <= intervalZ.Max; z+= m_gridCellSize.Z)
					{
						for (float y = intervalY.Min; y <= intervalY.Max; y+= m_gridCellSize.Y)
						{
							for (float x = intervalX.Min; x <= intervalX.Max; x+= m_gridCellSize.X)
							{
								int hash = ComputeHashFromPosition(x, y, z);

								if (m_grid.find(hash) == m_grid.end())
								{
									GridCell<T*>* pGridCell = RequestCell();
									ComputeCellAABB(x, y, z, pGridCell->BoundingBox);
									pGridCell->TriangleList.PushBack(&ITriangleMesh<T, U>::TriangleList[index]);
									m_grid[hash] = pGridCell;
									cellCount++;
								}
								else
								{
									GridCell<T*>* pGridCell = m_grid[hash];
									pGridCell->TriangleList.PushBack(&ITriangleMesh<T, U>::TriangleList[index]);
								}
							}
						}
					}
				}

				std::cout << "Cell count : " << cellCount << " : " << m_grid.size() << std::endl;
			}
		};

		/*
		template<class T>
		struct HybridGrid
		{
			const int EDGE = 4;

			Vector3 CellSize;
			AxisAlignedBoundingBox BoundingBox;
			List<T> ObjectList[EDGE * EDGE * EDGE];

			void Build(List<T> &TriangleList, AxisAlignedBoundingBox p_aabb)
			{
				BoundingBox = p_aabb;
				CellSize = (BoundingBox.GetMaxExtent() - BoundingBox.GetMinExtent()) / EDGE;

				AxisAlignedBoundingBox aabb;

				int count = TriangleList.Size();
				int x, y, z;

				for (int cell = 0; cell < EDGE*EDGE*EDGE; cell++)
				{
					x = cell % EDGE;
					y = (cell / EDGE) % EDGE;
					z = cell / (EDGE*EDGE);

					GetCellAABB(x, y, z, aabb);

					for (int n = 0; n < count; n++)
					{
						if (aabb.Intersect(TriangleList[n]->GetBoundingVolume()))
							ObjectList[cell].PushBack(TriangleList[n]);
					}
				}
			}

			bool Intersect(Ray &p_ray)
			{
				return true;
			}

			bool Intersect(Ray &p_ray, DifferentialSurface &p_surface)
			{
				float in, out;
				BoundingBox.Intersect(p_ray, in, out);

				int x, y, z;

				Vector3 normalisedOrigin = p_ray.PointAlongRay(in) - BoundingBox.GetMinExtent();

				origin.X /= CellSize.X;
				origin.Y /= CellSize.Y;
				origin.Z /= CellSize.Z;

				Vector3 step;

				step.X = p_ray.Direction.X > 0 ? (1.0f - Maths::Frac(origin.X)) / p_ray.Direction.X
					: Maths::Frac(origin.X) / p_ray.Direction.X;
				step.Y = p_ray.Direction.Y > 0 ? (1.0f - Maths::Frac(origin.Y)) / p_ray.Direction.Y
					: Maths::Frac(origin.Y) / p_ray.Direction.Y;
				step.Z = p_ray.Direction.Z > 0 ? (1.0f - Maths::Frac(origin.Z)) / p_ray.Direction.Z
					: Maths::Frac(origin.Z) / p_ray.Direction.Z;



				return true;
			}

			void GetCellAABB(int x, int y, int z, AxisAlignedBoundingBox &p_aabb)
			{
				Vector3 &minExtents = BoundingBox.GetMinExtent();
				Vector3 cellStart = Vector3(minExtents.X + CellSize.X * x, minExtents.Y + CellSize.Y * y,  minExtents.Z + CellSize.Z * z),
					cellEnd = cellStart + CellSize;

				p_aabb.SetExtents(cellStart, cellEnd);
			}
		}
		*/

		// Bounding volume hierarchy node
		template<class T>
		struct HybridNode
		{
			// Node bounding box
			AxisAlignedBoundingBox BoundingBox;

			// Node Type
			TreeMeshNodeType Type;

			// Only if an internal node
			HybridNode *m_pChild[2];

			// Only if a leaf
			List<T> TriangleList;

			// Construction and destruction
			HybridNode() { }
			~HybridNode() { }
		};

		// Bounding Volume Hierarchy Mesh
		template<class T, class U>
		class HybridMesh
			: public ITriangleMesh<T, U>
		{
		protected:
			TreeMeshStatistics m_statistics;

			int m_nMaxLeafObjects,
				m_nMaxDepth;

			HybridNode<T*> m_rootNode;

		protected:
			//----------------------------------------------------------------------------------------------
			// Methods for requesting and freeing nodes
			//----------------------------------------------------------------------------------------------
			HybridNode<T*>* RequestNode(void)
			{
				return new HybridNode<T*>();
			}

			int ReleaseNode(HybridNode<T*> *p_pNode)
			{
				int nodesFreed = 0;

				if (p_pNode->Type == TreeMeshNodeType::Internal)
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
			HybridMesh(void)
				: m_nMaxLeafObjects(20)
				, m_nMaxDepth(20)
			{ }

			HybridMesh(int p_nMaxObjectsPerLeaf, int p_nMaxDepth = 20)
				: m_nMaxLeafObjects(p_nMaxObjectsPerLeaf)
				, m_nMaxDepth(p_nMaxDepth)
			{ }

			~HybridMesh()
			{
				ReleaseNode(m_rootNode.m_pChild[0]);
				ReleaseNode(m_rootNode.m_pChild[1]);
			}

			boost::shared_ptr<ITriangleMesh<T, U>> CreateInstance(void) {
				return boost::shared_ptr<ITriangleMesh<T, U>>(new HybridMesh<T, U>());
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

			bool Intersect(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
			{
				Ray ray(p_ray);

				return Intersect_Stack(&m_rootNode, ray, p_fTime, p_surface);
			}

			bool Intersect(const Ray &p_ray, float p_fTime)
			{
				Ray ray(p_ray);

				return Intersect_Stack(&m_rootNode, ray, p_fTime);
			}

			std::string ToString(void) const
			{
				return boost::str(boost::format("\nBVHMesh %s") % m_statistics.ToString());
			}

		protected:
			bool Intersect_Stack(HybridNode<T*> *p_pNode, Ray &p_ray, float p_fTime)
			{
				if (!p_pNode->BoundingBox.Intersect(p_ray))
					return false;

				std::stack<HybridNode<T*>*> traverseStack;
				traverseStack.push(p_pNode);

				HybridNode<T*> *pNode;
				int count;

				while(!traverseStack.empty())
				{
					pNode = traverseStack.top();
					traverseStack.pop();

					while(pNode->Type == TreeMeshNodeType::Internal)
					{
						if (pNode->m_pChild[1]->BoundingBox.Intersect(p_ray))
							traverseStack.push(pNode->m_pChild[1]);

						if (pNode->m_pChild[0]->BoundingBox.Intersect(p_ray))
							pNode = pNode->m_pChild[0];
						else
							break;
					}

					if ((count = (int)pNode->TriangleList.Size()) > 0)
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

			bool Intersect_Stack(HybridNode<T*> *p_pNode, Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
			{
				if (!p_pNode->BoundingBox.Intersect(p_ray))
					return false;

				std::stack<HybridNode<T*>*> traverseStack;
				traverseStack.push(p_pNode);

				bool bIntersect = false;
				HybridNode<T*> *pNode;
				int count;

				while(!traverseStack.empty())
				{
					pNode = traverseStack.top();
					traverseStack.pop();

					while(pNode->Type == TreeMeshNodeType::Internal)
					{
						if (pNode->m_pChild[1]->BoundingBox.Intersect(p_ray))
							traverseStack.push(pNode->m_pChild[1]);

						if (pNode->m_pChild[0]->BoundingBox.Intersect(p_ray))
							pNode = pNode->m_pChild[0];
						else
							break;
					}

					if ((count = (int)pNode->TriangleList.Size()) > 0)
					{
						for (int n = 0; n < count; n++)
						{
							if (pNode->TriangleList[n]->Intersect(p_ray, p_fTime, p_surface))
							{
								p_ray.Max = Maths::Min(p_ray.Max, p_surface.Distance);
								bIntersect = true;
							}
						}
					}
				}

				return bIntersect;
			}

			bool Intersect_Recursive(HybridNode<T*> *p_pNode, Ray &p_ray, float p_fTime)
			{
				float in, out;

				if (p_pNode->BoundingBox.Intersect(p_ray, in, out))
				{
					if (p_pNode->Type == TreeMeshNodeType::Internal)
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

			bool Intersect_Recursive(HybridNode<T*> *p_pNode, Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
			{
				float in, out;

				if (p_pNode->BoundingBox.Intersect(p_ray, in, out))
				{
					if (p_pNode->Type == TreeMeshNodeType::Internal)
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

			void BuildHierarchy(HybridNode<T*> *p_pNode, List<T*> &p_objectList, int p_nAxis, int p_nDepth = 0)
			{
				// Update stats
				m_statistics.m_maxTreeDepth = Maths::Min(p_nDepth, m_statistics.m_maxTreeDepth);

				// Compute the node bounds for the given object list
				ComputeBounds(p_objectList, p_pNode->BoundingBox);

				// If we have enough objects, we consider this node a leaf
				if ((int)p_objectList.Size() <= m_nMaxLeafObjects || p_nDepth == m_nMaxDepth)
				{
					p_pNode->Type = TreeMeshNodeType::Leaf;
					p_pNode->TriangleList.PushBack(p_objectList);

					m_statistics.m_leafNodeCount++;
					m_statistics.m_minTreeDepth = Maths::Min(m_statistics.m_minTreeDepth, p_nDepth);
					m_statistics.m_minLeafTriangleCount = Maths::Min(m_statistics.m_minLeafTriangleCount, (int)p_objectList.Size());
					m_statistics.m_maxLeafTriangleCount = Maths::Min(m_statistics.m_maxLeafTriangleCount, (int)p_objectList.Size());
				}
				else
				{
					List<T*> leftList, rightList;
					leftList.Clear(); rightList.Clear();
					float fPartition = FindPartitionPlane(p_objectList, p_nAxis);
					Split(p_objectList, fPartition, p_nAxis, leftList, rightList);

					if (leftList.Size() == 0 || rightList.Size() == 0)
					{
						p_pNode->Type = TreeMeshNodeType::Leaf;
						p_pNode->TriangleList.PushBack(p_objectList);

						m_statistics.m_leafNodeCount++;
						m_statistics.m_minTreeDepth = Maths::Min(m_statistics.m_minTreeDepth, p_nDepth);
						m_statistics.m_minLeafTriangleCount = Maths::Min(m_statistics.m_minLeafTriangleCount, (int)p_objectList.Size());
						m_statistics.m_maxLeafTriangleCount = Maths::Min(m_statistics.m_maxLeafTriangleCount, (int)p_objectList.Size());
					}
					else
					{
						p_pNode->Type = TreeMeshNodeType::Internal;

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
	}
}
