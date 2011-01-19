//----------------------------------------------------------------------------------------------
//	Filename:	KDTreeMesh.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Shape/TreeMesh.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
TreeMeshStatistics::TreeMeshStatistics(void) 
	: m_internalNodeCount(0)
	, m_leafNodeCount(0)
	, m_maxTreeDepth(0)
	, m_minTreeDepth(0x7FFFFFFF)
	, m_triangleCount(0)
	, m_maxLeafTriangleCount(0)
	, m_minLeafTriangleCount(0x7FFFFFFF)
	, m_intersectionCount(0)
{ }
//----------------------------------------------------------------------------------------------
std::string TreeMeshStatistics::ToString(void) const
{
	return boost::str(boost::format("Internal Nodes : %d \n Leaf Nodes : %d \n Minimum Depth : %d \n Maximum Depth : %d \n Triangles : %d \n Min Tri/Leaf : %d \n Max Tri/Leaf : %d \n I-Tests : %d\n")
		% m_internalNodeCount % m_leafNodeCount 
		% m_minTreeDepth % m_maxTreeDepth 
		% m_triangleCount % m_minLeafTriangleCount % m_maxLeafTriangleCount 
		% m_intersectionCount);
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
ITreeMesh::ITreeMesh(void)
	: ITriangleMesh()
{ }
//----------------------------------------------------------------------------------------------
ITreeMesh::ITreeMesh(const std::string& p_strName)
	: ITriangleMesh(p_strName)
{ }

//----------------------------------------------------------------------------------------------
// Computes the axis-aligned bounding box for the specified list of triangles.
//----------------------------------------------------------------------------------------------
void ITreeMesh::ComputeBounds(const List<IndexedTriangle*> &p_objectList, 
	AxisAlignedBoundingBox &p_aabb, float p_fMinEpsilon, float p_fMaxEpsilon)
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
//----------------------------------------------------------------------------------------------
int ITreeMesh::Distribute(const List<IndexedTriangle*> &p_objectList, float p_fPartition, 
	int p_nAxis, List<IndexedTriangle*> &p_outLeftList, List<IndexedTriangle*> &p_outRightList)
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
//----------------------------------------------------------------------------------------------
int ITreeMesh::Distribute(const List<IndexedTriangle*> &p_objectList, 
	AxisAlignedBoundingBox &p_leftAABB, AxisAlignedBoundingBox &p_rightAABB, 
	List<IndexedTriangle*> &p_outLeftList, List<IndexedTriangle*> &p_outRightList)
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
// Returns a partition
//----------------------------------------------------------------------------------------------
float ITreeMesh::FindPartitionPlane(const List<IndexedTriangle*> &p_objectList, 
	AxisAlignedBoundingBox &p_aabb, int p_nAxis, PartitionType p_partition) 
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
// Returns a partition based on the spatian median of the object list.
//----------------------------------------------------------------------------------------------
float ITreeMesh::FindPartitionPlaneSpatialMedian(const List<IndexedTriangle*> &p_objectList, 
	AxisAlignedBoundingBox &p_aabb, int p_nAxis) 
{
	return p_aabb.GetCentre()[p_nAxis];
}

//----------------------------------------------------------------------------------------------
// Returns a partition baed on the (modified) SAH applied to the object list.
//----------------------------------------------------------------------------------------------
float ITreeMesh::FindPartitionPlaneSAH(const List<IndexedTriangle*> &p_objectList, 
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