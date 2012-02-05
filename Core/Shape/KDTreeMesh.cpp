//----------------------------------------------------------------------------------------------
//	Filename:	KDTreeMesh.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Shape/KDTreeMesh.h"

#include <stack>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include "Maths/Random.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		// Stack Element for stack-based traversal of KD-Tree
		//----------------------------------------------------------------------------------------------
		struct KDTreeStackElement
		{
		public:
			float Min;
			float Max;
			KDTreeMeshNode *pNode;

			KDTreeStackElement(KDTreeMeshNode *p_pNode, float p_fMin, float p_fMax)
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
	}
}

//----------------------------------------------------------------------------------------------
// Helper functions for allocation and deallocation of KDTree nodes
//----------------------------------------------------------------------------------------------
KDTreeMeshNode* KDTreeMesh::RequestNode(void)
{
	return new KDTreeMeshNode();
}
//----------------------------------------------------------------------------------------------
int KDTreeMesh::ReleaseNode(KDTreeMeshNode *p_pNode)
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
KDTreeMesh::KDTreeMesh(int p_nMaxObjectsPerLeaf, int p_nMaxTreeDepth)
	: ITreeMesh() 
	, m_nMaxLeafObjects(p_nMaxObjectsPerLeaf)
	, m_nMaxTreeDepth(p_nMaxTreeDepth)
{ }
//----------------------------------------------------------------------------------------------
KDTreeMesh::KDTreeMesh(const std::string &p_strName, int p_nMaxObjectsPerLeaf, int p_nMaxTreeDepth)
	: ITreeMesh(p_strName) 
	, m_nMaxLeafObjects(p_nMaxObjectsPerLeaf)
	, m_nMaxTreeDepth(p_nMaxTreeDepth)
{ }
//----------------------------------------------------------------------------------------------
KDTreeMesh::~KDTreeMesh(void)
{
	ReleaseNode(m_rootNode.m_pChild[0]);
	ReleaseNode(m_rootNode.m_pChild[1]);
}

//----------------------------------------------------------------------------------------------
// Method for creating an instance of the KDTreeMesh
// TODO: Should be replaced through ICloneable interface inherited through Object.
//----------------------------------------------------------------------------------------------
boost::shared_ptr<ITriangleMesh> KDTreeMesh::CreateInstance(void) {
	return boost::shared_ptr<ITriangleMesh>(new KDTreeMesh());
}

//----------------------------------------------------------------------------------------------
// Compile : This method is used to transform the raw triangle mesh into a kD-Tree 
// acceleration structure.
//----------------------------------------------------------------------------------------------
bool KDTreeMesh::Compile(void) 
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
// Returns the result of an intersection between a ray and the kD-Tree. The method also
// populates a DifferentialSurface structure with all the details of the intersected
// surface.
//----------------------------------------------------------------------------------------------
bool KDTreeMesh::Intersects(const Ray &p_ray, DifferentialSurface &p_surface)
{
	Ray ray(p_ray);

	//return Intersect_Recursive(&m_rootNode, ray, p_surface);
	return Intersect_Stack(&m_rootNode, ray, p_surface);
}

//----------------------------------------------------------------------------------------------
// Performs a quick intersection test, returning whether an intersection has occurred
// or not, but providing no further details as to the intersection itself.
//----------------------------------------------------------------------------------------------
bool KDTreeMesh::Intersects(const Ray &p_ray)
{
	Ray ray(p_ray);

	//return Intersect_Recursive(&m_rootNode, ray);
	return Intersect_Stack(&m_rootNode, ray);
}

//----------------------------------------------------------------------------------------------
// Returns a literal with information on the acceleration structure.
//----------------------------------------------------------------------------------------------
std::string KDTreeMesh::ToString(void) const
{
	return boost::str(boost::format("\nKDTreeMesh %s") % m_statistics.ToString());
}

//----------------------------------------------------------------------------------------------
// Performs intersection testing using a stack-based tree traversal method.
//----------------------------------------------------------------------------------------------
bool KDTreeMesh::Intersect_Stack(KDTreeMeshNode *p_pNode, Ray &p_ray)
{
	AxisAlignedBoundingBox *pAABB = 
		&p_pNode->BoundingBox;

	float tIn, tOut, tSplit,
		intercept, direction;

	int count, halfspace;

	if (!pAABB->Intersects(p_ray, tIn, tOut))
		return false;

	tIn = Maths::Max(0, tIn);

	KDTreeStackElement rootElement(p_pNode, tIn, tOut);
	std::stack<KDTreeStackElement> traverseStack;
	traverseStack.push(rootElement);

	KDTreeMeshNode *pNode;

	while(!traverseStack.empty())
	{
		// Get next element
		const KDTreeStackElement &nodeElement = 
			traverseStack.top();

		pNode = nodeElement.pNode;
		tOut = nodeElement.Max;
		tIn = nodeElement.Min;

		traverseStack.pop();

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
					KDTreeStackElement nodeElementHS(pNode->m_pChild[halfspace^1], tSplit, tOut);
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
				if (pNode->TriangleList[n]->Intersects(p_ray))
					return true;
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------------------------------
// Performs intersection testing using stack-based traversal.
//----------------------------------------------------------------------------------------------
bool KDTreeMesh::Intersect_Stack(KDTreeMeshNode *p_pNode, Ray &p_ray, DifferentialSurface &p_surface)
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

	KDTreeStackElement rootElement(p_pNode, tIn, tOut);
	std::stack<KDTreeStackElement> traverseStack;
	traverseStack.push(rootElement);

	KDTreeMeshNode *pNode;

	while(!traverseStack.empty())
	{
		// Get next element
		const KDTreeStackElement &nodeElement = 
			traverseStack.top();

		pNode = nodeElement.pNode;
		tOut = nodeElement.Max;
		tIn = nodeElement.Min;

		traverseStack.pop();

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
					KDTreeStackElement nodeElementHS(pNode->m_pChild[halfspace^1], tSplit, tOut);
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
				if (pNode->TriangleList[n]->Intersects(p_ray, p_surface))
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
bool KDTreeMesh::Intersect_Recursive(KDTreeMeshNode *p_pNode, Ray &p_ray)
{
	float in, out;

	if (p_pNode->BoundingBox.Intersects(p_ray, in, out))
	{
		if (p_pNode->Type == Internal)
			return Intersect_Recursive(p_pNode->m_pChild[0], p_ray) || Intersect_Recursive(p_pNode->m_pChild[1], p_ray);

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
bool KDTreeMesh::Intersect_Recursive(KDTreeMeshNode *p_pNode, Ray &p_ray, DifferentialSurface &p_surface)
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
					return Intersect_Recursive(p_pNode->m_pChild[halfspace], p_ray, p_surface);
				else
					return Intersect_Recursive(p_pNode->m_pChild[halfspace], p_ray, p_surface) || 
					Intersect_Recursive(p_pNode->m_pChild[halfspace^1], p_ray, p_surface);
			}
			else
			{
				return Intersect_Recursive(p_pNode->m_pChild[halfspace], p_ray, p_surface);
			}
		}

		// Test geometry at leaf
		bool bIntersect = false;
		int count = (int)p_pNode->TriangleList.Size();

		if (count > 0)
		{
			for (int n = 0; n < count; n++)
			{
				if (p_pNode->TriangleList[n]->Intersects(p_ray, p_surface))
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
// Builds the kd-tree hierarchy
//----------------------------------------------------------------------------------------------
void KDTreeMesh::BuildHierarchy(KDTreeMeshNode *p_pNode, List<IndexedTriangle*> &p_objectList, int p_nAxis, int p_nDepth)
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
void KDTreeMesh::BuildHierarchy_S2(KDTreeMeshNode *p_pNode, List<IndexedTriangle*> &p_objectList, int p_nAxis, int p_nDepth)
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