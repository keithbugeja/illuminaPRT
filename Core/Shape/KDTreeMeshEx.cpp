//----------------------------------------------------------------------------------------------
//	Filename:	KDTreeMeshEx.cpp
//	Author:		Kevin Napoli
//	Date:		01/07/2013
//----------------------------------------------------------------------------------------------
//  TODO:	Clean up code (use IlluminaPRT coding standards)
//----------------------------------------------------------------------------------------------

#include "Shape/KDTreeMeshEx.h"
#include "Shape/PersistentMesh.h"

#include <stack>
#include <queue>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include "Maths/Random.h"

using namespace Illumina::Core;


KDTreeMeshExNode::KDTreeMeshExNode()
	: TriangleList ( 1, 3 ) , m_pChild ( NULL ), Partition ( 0.0f ), Axis ( 0 )
{
}

/*KDTreeMeshExNode::~KDTreeMeshExNode()
{
	if(m_pChild == NULL)
	{
		Nothing to explicitly delete since List is being used
	}
}*/

KDTreeMeshEx::KDTreeMeshEx(int p_nMaxObjectsPerLeaf, int p_nMaxTreeDepth)
	: m_nMaxLeafObjects ( p_nMaxObjectsPerLeaf ), m_nMaxTreeDepth ( p_nMaxObjectsPerLeaf )
{
}


KDTreeMeshEx::KDTreeMeshEx(const std::string &p_strName, int p_nMaxObjectsPerLeaf, int p_nMaxTreeDepth)
	: ITreeMesh( p_strName ), m_nMaxLeafObjects ( p_nMaxObjectsPerLeaf ), m_nMaxTreeDepth ( p_nMaxObjectsPerLeaf )
{
}

KDTreeMeshEx::~KDTreeMeshEx(void)
{
}

boost::shared_ptr<ITriangleMesh> KDTreeMeshEx::CreateInstance(void)
{
	return boost::shared_ptr<ITriangleMesh>(new KDTreeMeshEx());
}

//----------------------------------------------------------------------------------------------
// Builds the kd-tree hierarchy
//----------------------------------------------------------------------------------------------
void KDTreeMeshEx::BuildHierarchy(KDTreeMeshExNode *p_pNode, List<IndexedTriangle*> &p_objectList, int p_nAxis, int p_nDepth)
{
	ComputeBounds(p_objectList, m_boundingBox, 1e-4f, 1e-4f);
	BuildHierarchy_S2(p_pNode, p_objectList, m_boundingBox, m_boundingBox.GetExtent().ArgMaxAbsComponent(), 0);
}

//----------------------------------------------------------------------------------------------
// Builds the kd-tree hierarchy - used most of Keith's Code
// Difference is I don't store an AABB in every node but I pass it as parameter
//----------------------------------------------------------------------------------------------
void KDTreeMeshEx::BuildHierarchy_S2(KDTreeMeshExNode *p_pNode, List<IndexedTriangle*> &p_objectList, AxisAlignedBoundingBox &p_boundingBox, int p_nAxis, int p_nDepth)
{
	// Update stats
	m_statistics.m_maxTreeDepth = Maths::Min(p_nDepth, m_statistics.m_maxTreeDepth);

	// If we have enough objects, we consider this node a leaf
	if ((int)p_objectList.Size() <= m_nMaxLeafObjects || p_nDepth == m_nMaxTreeDepth || p_boundingBox.GetRadius() <= m_fMinNodeWidth)
	{
		//std::cout << "Adding leaf node [" << p_objectList.Size() << ", " << p_nDepth << "]" << std::endl;
		//p_pNode->Type = Leaf; use p_pNode->child == NULL to check for leaf..
		
		p_pNode->TriangleList.PushBack(p_objectList);

		m_statistics.m_leafNodeCount++;
		m_statistics.m_minTreeDepth = Maths::Min(m_statistics.m_minTreeDepth, p_nDepth);
		m_statistics.m_minLeafTriangleCount = Maths::Min(m_statistics.m_minLeafTriangleCount, (int)p_objectList.Size());
		m_statistics.m_maxLeafTriangleCount = Maths::Min(m_statistics.m_maxLeafTriangleCount, (int)p_objectList.Size());
	}
	else
	{
		//std::cout << "Adding internal node [" << p_objectList.Size() << ", " << p_nDepth << "]" << std::endl;
		//p_pNode->Type = Internal;
		p_pNode->Axis = p_nAxis;
		p_pNode->Partition = FindPartitionPlane(p_objectList, p_boundingBox, p_nAxis, SpatialMedian);

		List<IndexedTriangle*> leftList, rightList;
		leftList.Clear(); rightList.Clear(); 

		p_pNode->m_pChild = m_pMem->getNext(2);

		AxisAlignedBoundingBox leftAABB ( p_boundingBox ), rightAABB ( p_boundingBox ) ;
		leftAABB.SetMaxExtent(p_nAxis, p_pNode->Partition);
		rightAABB.SetMinExtent(p_nAxis, p_pNode->Partition);

		Distribute(p_objectList, p_pNode->Partition, p_pNode->Axis, leftList, rightList);

		int nAxis ( (p_nAxis + 1) % 3 ), nDepth ( p_nDepth + 1 );
		BuildHierarchy_S2(p_pNode->m_pChild, leftList, leftAABB, nAxis, nDepth);
		BuildHierarchy_S2(p_pNode->m_pChild + 1, rightList, rightAABB, nAxis, nDepth);

		m_statistics.m_internalNodeCount++;
	}
}

//----------------------------------------------------------------------------------------------
// Compile : This method is used to transform the raw triangle mesh into a kD-Tree 
// acceleration structure.
//----------------------------------------------------------------------------------------------
bool KDTreeMeshEx::Compile(void) 
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
	ComputeBounds(triangleList, m_boundingBox);

	// Compute the minimum dimensions constraint for a node 
	// TODO: Should be parameterised!
	m_fMinNodeWidth = m_boundingBox.GetRadius() / 1000.0f;
	
	// Build kd-tree hierarchy
	m_pMem.reset();
	m_pMem = boost::shared_ptr< MemoryManager<KDTreeMeshExNode> > ( new MemoryManager<KDTreeMeshExNode>(1024*1024*1, 64) );
	BuildHierarchy(m_rootNode = m_pMem->getNext(), triangleList, 0); 
	
	// Update Stats
	m_statistics.m_triangleCount = objectCount;

	// std::cout << "Mesh contains: " << triangleList.Size() << " Triangles" << std::endl;

	return true;
}

bool KDTreeMeshEx::Intersects(const Ray &p_ray, DifferentialSurface &p_surface)
{
	Ray r ( p_ray );
	return Intersects_Stack(r, p_surface);
}



bool KDTreeMeshEx::Intersects(const Ray &p_ray)
{
	Ray r ( p_ray );
	return Intersects_Stack(r);
}

bool KDTreeMeshEx::Intersects_Stack(Ray &p_ray, DifferentialSurface &p_surface)
{
	//start alg
	//IndexedTriangle * localPrim ( NULL );
	//test b = t;
	float a, b, tTemp;
	//find entry and exit
	if ( !m_boundingBox.Intersects(p_ray, a, b) ) 
		return false;
	tTemp = b;
	bool ret ( false );

	//if(b > t) b = t; // <-- what can this lead too when splitpos > t ? no need!
	//should be found
	StackElem st[30];
	unsigned int sp ( 0 );
	//stack<StackElem> st; //while(!st.empty()) st.pop();
	KDTreeMeshExNode *farChild, *nearChild, *currNode;
	st[0].node = m_rootNode;// = StackElem(root_, a, b);
	st[0].a = a;
	st[0].b = b; ++sp;
	//st[sp++] = {root_, a, b};
	//use map for correctness.. ordered_map = hash, map = some tree
	//try unordered_map first.. both fucking slow
	//boost::unordered_map<Primitive *, bool> rayMap;
	//start loooopin'
	while(sp)
	{
		{
			const StackElem& p = st[--sp];
			currNode = p.node; a = p.a; b = p.b;
			//currNode = st[--sp].node;
			//a = st[sp].a;
			//b = st[sp].b;
		}
		while(currNode->m_pChild != NULL)//NOT LEAF
		{
			//++statPerformance_;
			float diff ( currNode->Partition - p_ray.Origin.Element[currNode->Axis] );
			if(diff == 0)
			{
				currNode = p_ray.Direction.Element[currNode->Axis] > 0 ? 
					currNode->m_pChild + 1 : currNode->m_pChild;
				continue;
			}
			else if(diff > 0)
			{
				nearChild = currNode->m_pChild;
				farChild = currNode->m_pChild + 1;
			}else
			{
				nearChild = currNode->m_pChild + 1;
				farChild = currNode->m_pChild;
			}
			
			if(p_ray.Direction.Element[currNode->Axis] != 0)
			{
				//or use direction inverse cache..
				float tSplit ( diff / p_ray.Direction.Element[currNode->Axis] ); //ATTENTION
				
				if( (tSplit > b) || (tSplit < 0) )
				{
					currNode = nearChild;
				}else //tSplit >= 0 && tSplit <= b
				{
					if(tSplit < a)//tSplit >= 0 && tSplit <= b && tSplit < a
					{
						currNode = farChild;
					}
					else //case 4 - //tSplit >= 0 && tSplit >= a && tSplit <= b
					{
						st[sp].node = farChild;
						st[sp].a = tSplit;
						st[sp++].b = b;
						if(sp == 30){ throw 30; }
						currNode = nearChild;
						b = tSplit;
					}
				}
				
			}else
			{
				//If 0 use Aabb for both and check if ray intersects..
				//return 4;
				currNode = nearChild;
				//std::cout << "UNHANDLED CASE :(\n";
			}
		}

		//is LEAF! - check leaf prim list..
		if( unsigned int count = (unsigned int)currNode->TriangleList.Size() )
		{
			//setup ray bounds
			p_ray.Min = a; //<--- this is being ignored in IndexedTriangle::Intersects method
			p_ray.Max = tTemp;

			for(unsigned int n = 0; n < count; ++n)
			{
				if(currNode->TriangleList[n]->Intersects(p_ray, p_surface))
				{
					p_ray.Max = tTemp = Maths::Min(tTemp, p_surface.Distance);
					//localPrim = currNode->TriangleList[n];
					ret = true;
				}
			}
		}
		
		//keep this order for performance
		if(tTemp <= b && tTemp >= a)
		{
			//t = tTemp;
			//primitive = localPrim;
			return ret;
		}
	}
	//stack empty, found nothing
	return false;
}

bool KDTreeMeshEx::Intersects_Stack(Ray &p_ray)
{
	//start alg
	//IndexedTriangle * localPrim ( NULL );
	//test b = t;
	float a (1000000.0f ), b ( a ), tTemp;
	//find entry and exit
	if ( !m_boundingBox.Intersects(p_ray, a, b) ) 
		return false;
	tTemp = b;
	bool ret ( false );

	//if(b > t) b = t; // <-- what can this lead too when splitpos > t ? no need!
	//should be found
	StackElem st[30];
	unsigned int sp ( 0 );
	//stack<StackElem> st; //while(!st.empty()) st.pop();
	KDTreeMeshExNode *farChild, *nearChild, *currNode;
	st[0].node = m_rootNode;// = StackElem(root_, a, b);
	st[0].a = a;
	st[0].b = b; ++sp;
	//st[sp++] = {root_, a, b};
	//use map for correctness.. ordered_map = hash, map = some tree
	//try unordered_map first.. both fucking slow
	//boost::unordered_map<Primitive *, bool> rayMap;
	//start loooopin'
	while(sp)
	{
		{
			const StackElem& p = st[--sp];
			currNode = p.node; a = p.a; b = p.b;
			//currNode = st[--sp].node;
			//a = st[sp].a;
			//b = st[sp].b;
		}
		while(currNode->m_pChild != NULL)//NOT LEAF
		{
			//++statPerformance_;
			float diff ( currNode->Partition - p_ray.Origin.Element[currNode->Axis] );
			if(diff == 0)
			{
				currNode = p_ray.Direction.Element[currNode->Axis] > 0 ? 
					currNode->m_pChild + 1 : currNode->m_pChild;
				continue;
			}
			else if(diff > 0)
			{
				nearChild = currNode->m_pChild;
				farChild = currNode->m_pChild + 1;
			}else
			{
				nearChild = currNode->m_pChild + 1;
				farChild = currNode->m_pChild;
			}
			
			if(p_ray.Direction.Element[currNode->Axis] != 0)
			{
				//or use direction inverse cache..
				float tSplit ( diff / p_ray.Direction.Element[currNode->Axis] ); //ATTENTION
				
				if( (tSplit > b) || (tSplit < 0) )
				{
					currNode = nearChild;
				}else //tSplit >= 0 && tSplit <= b
				{
					if(tSplit < a)//tSplit >= 0 && tSplit <= b && tSplit < a
					{
						currNode = farChild;
					}
					else //case 4 - //tSplit >= 0 && tSplit >= a && tSplit <= b
					{
						st[sp].node = farChild;
						st[sp].a = tSplit;
						st[sp++].b = b;
						if(sp == 30){ throw 30; }
						currNode = nearChild;
						b = tSplit;
					}
				}
				
			}else
			{
				//If 0 use Aabb for both and check if ray intersects..
				//return 4;
				currNode = nearChild;
				//std::cout << "UNHANDLED CASE :(\n";
			}
		}

		//is LEAF! - check leaf prim list..
		if( unsigned int count = (unsigned int)currNode->TriangleList.Size() )
		{
			//setup ray bounds
			p_ray.Min = a;
			p_ray.Max = tTemp;

			for(unsigned int n = 0; n < count; ++n)
			{
				if(currNode->TriangleList[n]->Intersects(p_ray))
				{
					return true;
				}
			}
		}
		
		//keep this order for performance
		if(tTemp <= b && tTemp >= a)
		{
			return false;
		}
	}
	//stack empty, found nothing
	return false;
}


//----------------------------------------------------------------------------------------------
// Returns a literal with information on the acceleration structure.
//----------------------------------------------------------------------------------------------
std::string KDTreeMeshEx::ToString(void) const
{
	return boost::str(boost::format("\nKDTreeMeshEx %s") % m_statistics.ToString());
}