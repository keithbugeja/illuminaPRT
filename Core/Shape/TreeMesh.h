//----------------------------------------------------------------------------------------------
//	Filename:	TreeMesh.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Shape/TriangleMesh.h"

namespace Illumina 
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		// TODO: Since structure members are public, change naming notation
		struct TreeMeshStatistics 
		{
			int m_internalNodeCount,
				m_leafNodeCount,
				m_maxTreeDepth,
				m_minTreeDepth,
				m_triangleCount,
				m_maxLeafTriangleCount,
				m_minLeafTriangleCount,
				m_intersectionCount;

			TreeMeshStatistics(void);

			std::string ToString(void) const;
		};

		//----------------------------------------------------------------------------------------------
		class ITreeMesh 
			: public ITriangleMesh
		{
		public:
			//----------------------------------------------------------------------------------------------
			// Enumeration for representing tree node types 
			//----------------------------------------------------------------------------------------------
			enum NodeType
			{
				Internal,
				Leaf
			};

		protected:
			//----------------------------------------------------------------------------------------------
			// Enumeration for selecting partitioning type
			//----------------------------------------------------------------------------------------------
			enum PartitionType
			{
				SpatialMedian,
				SurfaceAreaHeuristic
			};

		protected:
			//----------------------------------------------------------------------------------------------
			// Bookkeeping information on tree-based acceleration structure
			//----------------------------------------------------------------------------------------------
			TreeMeshStatistics m_statistics;
		
		protected:
			ITreeMesh(void);
			ITreeMesh(const std::string& p_strName);

			void ComputeBounds(const List<IndexedTriangle*> &p_objectList, AxisAlignedBoundingBox &p_aabb, 
				float p_fMinEpsilon = 0.0f, float p_fMaxEpsilon = 0.0f);


			int Distribute(const List<IndexedTriangle*> &p_objectList, float p_fPartition, int p_nAxis, 
				List<IndexedTriangle*> &p_outLeftList, List<IndexedTriangle*> &p_outRightList);

			int Distribute(const List<IndexedTriangle*> &p_objectList, 
				AxisAlignedBoundingBox &p_leftAABB, AxisAlignedBoundingBox &p_rightAABB, 
				List<IndexedTriangle*> &p_outLeftList, List<IndexedTriangle*> &p_outRightList);


			float FindPartitionPlane(const List<IndexedTriangle*> &p_objectList, AxisAlignedBoundingBox &p_aabb, 
				int p_nAxis, PartitionType p_partitionType);

			float FindPartitionPlaneSpatialMedian(const List<IndexedTriangle*> &p_objectList, 
				AxisAlignedBoundingBox &p_aabb, int p_nAxis);
			
			float FindPartitionPlaneSAH(const List<IndexedTriangle*> &p_objectList, 
				AxisAlignedBoundingBox &p_aabb, int p_nAxis);
		};
		//----------------------------------------------------------------------------------------------
	}
}