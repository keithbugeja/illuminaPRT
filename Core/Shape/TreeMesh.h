//----------------------------------------------------------------------------------------------
//	Filename:	TreeMesh.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

namespace Illumina 
{
	namespace Core
	{
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

			TreeMeshStatistics(void) 
				: m_internalNodeCount(0)
				, m_leafNodeCount(0)
				, m_maxTreeDepth(0)
				, m_minTreeDepth(0x7FFFFFFF)
				, m_triangleCount(0)
				, m_maxLeafTriangleCount(0)
				, m_minLeafTriangleCount(0x7FFFFFFF)
				, m_intersectionCount(0)
			{ }

			std::string ToString(void) const
			{
				return boost::str(boost::format("Internal Nodes : %d \n Leaf Nodes : %d \n Minimum Depth : %d \n Maximum Depth : %d \n Triangles : %d \n Min Tri/Leaf : %d \n Max Tri/Leaf : %d \n I-Tests : %d\n")
					% m_internalNodeCount % m_leafNodeCount 
					% m_minTreeDepth % m_maxTreeDepth 
					% m_triangleCount % m_minLeafTriangleCount % m_maxLeafTriangleCount 
					% m_intersectionCount);
			}
		};

		enum TreeMeshNodeType
		{
			Internal,
			Leaf
		};
	}
}