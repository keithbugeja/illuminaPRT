//----------------------------------------------------------------------------------------------
//	Filename:	KDTreeMesh.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Shape/BVHMesh.h"

#include <stack>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include "Maths/Random.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
// Helper functions for allocation and deallocation of KDTree nodes
//----------------------------------------------------------------------------------------------
BVHMeshNode* BVHMesh::RequestNode(void)
{
	return new BVHMeshNode();
}
//----------------------------------------------------------------------------------------------
int BVHMesh::ReleaseNode(BVHMeshNode *p_pNode)
{
	int nodesFreed = 0;

	if (p_pNode != NULL && p_pNode->Type == Internal)
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

//----------------------------------------------------------------------------------------------
// Constructors and destructor
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
BVHMesh::BVHMesh(int p_nMaxObjectsPerLeaf, int p_nMaxTreeDepth)
	: ITreeMesh() 
	, m_nMaxLeafObjects(p_nMaxObjectsPerLeaf)
	, m_nMaxTreeDepth(p_nMaxTreeDepth)
{ }
//----------------------------------------------------------------------------------------------
BVHMesh::BVHMesh(const std::string &p_strName, int p_nMaxObjectsPerLeaf, int p_nMaxTreeDepth)
	: ITreeMesh(p_strName) 
	, m_nMaxLeafObjects(p_nMaxObjectsPerLeaf)
	, m_nMaxTreeDepth(p_nMaxTreeDepth)
{ }
//----------------------------------------------------------------------------------------------
BVHMesh::~BVHMesh(void)
{
	ReleaseNode(m_rootNode.m_pChild[0]);
	ReleaseNode(m_rootNode.m_pChild[1]);
}

//----------------------------------------------------------------------------------------------
// Method for creating an instance of the BVHMesh
// TODO: Should be replaced through ICloneable interface inherited through Object.
//----------------------------------------------------------------------------------------------
boost::shared_ptr<ITriangleMesh> BVHMesh::CreateInstance(void) {
	return boost::shared_ptr<ITriangleMesh>(new BVHMesh());
}

//----------------------------------------------------------------------------------------------
// Compile : This method is used to transform the raw triangle mesh into a BVH
// acceleration structure.
//----------------------------------------------------------------------------------------------
bool BVHMesh::Compile(void) 
{
	// Area
	ITriangleMesh::ComputeArea();

	// Generate a list of triangle pointers 
	int objectCount = (int)ITriangleMesh::TriangleList.Size();
	List<IndexedTriangle*> triangleList(objectCount);

	for (int idx = 0; idx < objectCount; idx++) {
		triangleList.PushBack(&ITriangleMesh::TriangleList[idx]);
	}

	// Compute the bounds for triangle list
	ComputeBounds(triangleList, m_rootNode.BoundingBox);

	// Compute the minimum dimensions constraint for a node 				
	// TODO: Should be parameterised!
	m_fMinNodeWidth = m_rootNode.BoundingBox.GetRadius() / 1000.0f;
				
	// Build kd-tree hierarchy
	BuildHierarchy(&m_rootNode, triangleList, 0); 

	// Update Stats
	m_statistics.m_triangleCount = objectCount;

	// std::cout << "Mesh contains: " << triangleList.Size() << " Triangles" << std::endl;

	return true;
}

//----------------------------------------------------------------------------------------------
// Returns the result of an intersection between a ray and the acceleration structure. The method 
// also populates a DifferentialSurface structure with all the details of the intersected
// surface.
//----------------------------------------------------------------------------------------------
bool BVHMesh::Intersects(const Ray &p_ray, DifferentialSurface &p_surface)
{
	Ray ray(p_ray);

	return Intersect(&m_rootNode, ray, p_surface);
}

//----------------------------------------------------------------------------------------------
// Performs a quick intersection test, returning whether an intersection has occurred
// or not, but providing no further details as to the intersection itself.
//----------------------------------------------------------------------------------------------
bool BVHMesh::Intersects(const Ray &p_ray)
{
	Ray ray(p_ray);

	return Intersect(&m_rootNode, ray);
}

//----------------------------------------------------------------------------------------------
// Returns a literal with information on the acceleration structure.
//----------------------------------------------------------------------------------------------
std::string BVHMesh::ToString(void) const
{
	return boost::str(boost::format("\nBVHMesh %s") % m_statistics.ToString());
}

//----------------------------------------------------------------------------------------------
// Performs intersection testing using recursive traversal.
//----------------------------------------------------------------------------------------------
bool BVHMesh::Intersect(BVHMeshNode *p_pNode, Ray &p_ray)
{
	float in, out;

	if (p_pNode->BoundingBox.Intersects(p_ray, in, out))
	{
		if (p_pNode->Type == Internal)
			return Intersect(p_pNode->m_pChild[0], p_ray) || Intersect(p_pNode->m_pChild[1], p_ray);

		int count = (int)p_pNode->TriangleList.Size();

		if (count == 0) 
			return false;

		for (int n = 0; n < count; n++)
		{
			if (p_pNode->TriangleList[n]->Intersects(p_ray))
				return true;
		}
	}
	
	return false;
}
//----------------------------------------------------------------------------------------------
// Performs intersection testing using recursive traversal.
//----------------------------------------------------------------------------------------------
bool BVHMesh::Intersect(BVHMeshNode *p_pNode, Ray &p_ray, DifferentialSurface &p_surface)
{
	float in, out;

	if (p_pNode->BoundingBox.Intersects(p_ray, in, out))
	{
		if (p_pNode->Type == Internal)
			return Intersect(p_pNode->m_pChild[0], p_ray, p_surface) | Intersect(p_pNode->m_pChild[1], p_ray, p_surface);

		// Test geometry at leaf
		bool bIntersect = false;
		int count = (int)p_pNode->TriangleList.Size();

		if (count > 0)
		{
			for (int n = 0; n < count; n++)
			{
				if (p_pNode->TriangleList[n]->Intersects(p_ray, p_surface))
				{
					bIntersect = true;
					p_ray.Max = Maths::Min(p_ray.Max, p_surface.Distance - Maths::Epsilon);
				}
			}
		}

		return bIntersect;
	}

	return false;
}

//----------------------------------------------------------------------------------------------
// Builds the tree hierarchy
//----------------------------------------------------------------------------------------------
void BVHMesh::BuildHierarchy(BVHMeshNode *p_pNode, List<IndexedTriangle*> &p_objectList, int p_nAxis, int p_nDepth)
{
	ComputeBounds(p_objectList, p_pNode->BoundingBox, 0.0001f, 0.0001f);
	const Vector3 &size = p_pNode->BoundingBox.GetExtent();
	if (size.X > size.Y) p_nAxis = size.X > size.Z ? 0 : 2;
	else p_nAxis = size.Y > size.Z ? 1 : 2;

	BuildHierarchy_S2(p_pNode, p_objectList, p_nAxis, 0);
}

//----------------------------------------------------------------------------------------------
// Builds the tree hierarchy
//----------------------------------------------------------------------------------------------
void BVHMesh::BuildHierarchy_S2(BVHMeshNode *p_pNode, List<IndexedTriangle*> &p_objectList, int p_nAxis, int p_nDepth)
{
	// Update stats
	m_statistics.m_maxTreeDepth = Maths::Min(p_nDepth, m_statistics.m_maxTreeDepth);

	// If we have enough objects, we consider this node a leaf
	if ((int)p_objectList.Size() <= m_nMaxLeafObjects || p_nDepth == m_nMaxTreeDepth || p_pNode->BoundingBox.GetRadius() <= m_fMinNodeWidth)
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
		p_pNode->Partition = FindPartitionPlane(p_objectList, p_pNode->BoundingBox, p_nAxis, SurfaceAreaHeuristic);

		List<IndexedTriangle*> leftList, rightList;
		leftList.Clear(); rightList.Clear(); 

		p_pNode->m_pChild[0] = RequestNode();
		p_pNode->m_pChild[1] = RequestNode();

		AxisAlignedBoundingBox 
			&leftAABB = p_pNode->m_pChild[0]->BoundingBox,
			&rightAABB = p_pNode->m_pChild[1]->BoundingBox;

		//leftAABB = p_pNode->BoundingBox;
		//rightAABB = p_pNode->BoundingBox;

		//leftAABB.SetMaxExtent(p_nAxis, p_pNode->Partition);
		//rightAABB.SetMinExtent(p_nAxis, p_pNode->Partition);

		Distribute(p_objectList, p_pNode->Partition, p_pNode->Axis, leftList, rightList);

		ComputeBounds(leftList, leftAABB);
		ComputeBounds(rightList, rightAABB);

		int nAxis = (p_nAxis + 1) % 3,
			nDepth = p_nDepth + 1;

		BuildHierarchy_S2(p_pNode->m_pChild[0], leftList, nAxis, nDepth);
		BuildHierarchy_S2(p_pNode->m_pChild[1], rightList, nAxis, nDepth);

		m_statistics.m_internalNodeCount++;
	}
}