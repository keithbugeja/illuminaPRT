//----------------------------------------------------------------------------------------------
//	Filename:	PersistentMesh.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Shape/PersistentMesh.h"

#include <stack>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include "Maths/Random.h"
#include "Maths/Montecarlo.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
// Definitions for Muller-Trumbore intersection
#define EPSILON 1e-5

#define CROSS(dest, v1, v2) \
	dest[0] = v1[1]*v2[2]-v1[2]*v2[1]; \
	dest[1] = v1[2]*v2[0]-v1[0]*v2[2]; \
	dest[2] = v1[0]*v2[1]-v1[1]*v2[0];

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define SUB(dest, v1, v2) \
	dest[0] = v1[0]-v2[0]; \
	dest[1] = v1[1]-v2[1]; \
	dest[2] = v1[2]-v2[2];

#define DET(c1, c2, c3) \
	(c1[0] * (c2[1]*c3[2] - c3[1]*c2[2]) - \
	 c1[1] * (c2[0]*c3[2] - c3[0]*c2[2]) + \
	 c1[2] * (c2[0]*c3[1] - c3[0]*c2[1])) 

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#define MAX_STACK_NODES	64
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		// Stack Element for stack-based traversal of Persistent Tree
		//----------------------------------------------------------------------------------------------
		struct PersistentTreeStackElement
		{
		public:
			float Min;
			float Max;
			PersistentTreeNode *pNode;

			PersistentTreeStackElement(void) { }

			PersistentTreeStackElement(PersistentTreeNode *p_pNode, float p_fMin, float p_fMax)
				: Min(p_fMin)
				, Max(p_fMax)
				, pNode(p_pNode)
			{ }

			PersistentTreeStackElement(const PersistentTreeStackElement &p_stackElement)
				: Min(p_stackElement.Min)
				, Max(p_stackElement.Max)
				, pNode(p_stackElement.pNode)
			{ }
		};
	}
}

//----------------------------------------------------------------------------------------------
void PersistentMesh::MapFiles(const std::string &p_strTrunkName)
{
	m_vertexFile.open(p_strTrunkName + ".ver");
	m_triangleFile.open(p_strTrunkName + ".tri");
	m_treeFile.open(p_strTrunkName + ".tre");

	m_nTriangleCount = m_triangleFile.size() / sizeof(PersistentIndexedTriangle);
	m_nVertexCount = m_vertexFile.size() / sizeof(Vertex);

	m_pRootNode = (PersistentTreeNode*)m_treeFile.const_data();
	m_pTriangleList = (PersistentIndexedTriangle*)m_triangleFile.const_data();
	m_pVertexList = (Vertex*)m_vertexFile.const_data();
}
//----------------------------------------------------------------------------------------------
void PersistentMesh::UnmapFiles(void)
{
	m_vertexFile.close();
	m_triangleFile.close();
	m_treeFile.close();
}

//----------------------------------------------------------------------------------------------
// Constructors and destructor
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
PersistentMesh::PersistentMesh(const std::string &p_strTrunkName)
	: IShape()
{
	MapFiles(p_strTrunkName);
}
//----------------------------------------------------------------------------------------------
PersistentMesh::PersistentMesh(const std::string &p_strName, const std::string &p_strTrunkName)
	: IShape(p_strName)
{ 
	MapFiles(p_strTrunkName);
}
//----------------------------------------------------------------------------------------------
PersistentMesh::~PersistentMesh(void)
{
	UnmapFiles();
}

//----------------------------------------------------------------------------------------------
bool PersistentMesh::IsBounded(void) const
{
	return true;
}
//----------------------------------------------------------------------------------------------
void PersistentMesh::ComputeBoundingVolume(void)
{
	Vertex *pVertex = m_pVertexList;
	m_boundingBox.SetExtents(pVertex->Position, pVertex->Position);

	for (int nIdx = 0; nIdx < m_nVertexCount; ++nIdx, ++pVertex) {
		m_boundingBox.Union(pVertex->Position);
	}
}
//----------------------------------------------------------------------------------------------
IBoundingVolume* PersistentMesh::GetBoundingVolume(void) const
{
	return (IBoundingVolume*)&m_boundingBox;
}
//----------------------------------------------------------------------------------------------
void PersistentMesh::ComputeArea(void)
{
	PersistentIndexedTriangle *pTriangle = m_pTriangleList;

	m_fArea = 0.f;
	
	for (int nIdx = 0; nIdx < m_nTriangleCount; ++nIdx, ++pTriangle)
	{
		const Vertex	&v0 = m_pVertexList[pTriangle->VertexID[0]],
						&v1 = m_pVertexList[pTriangle->VertexID[1]],
						&v2 = m_pVertexList[pTriangle->VertexID[2]];

		m_fArea += Vector3::Cross(v1.Position - v0.Position, v2.Position - v0.Position).Length() * 0.5f;
	}
}
//----------------------------------------------------------------------------------------------
float PersistentMesh::GetArea(void) const
{
	return m_fArea;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// Returns the result of an intersection between a ray and the kD-Tree. The method also
// populates a DifferentialSurface structure with all the details of the intersected
// surface.
//----------------------------------------------------------------------------------------------
bool PersistentMesh::Intersects(const Ray &p_ray, DifferentialSurface &p_surface)
{
	return IntersectP(Ray(p_ray), p_surface);
}

//----------------------------------------------------------------------------------------------
// Performs a quick intersection test, returning whether an intersection has occurred
// or not, but providing no further details as to the intersection itself.
//----------------------------------------------------------------------------------------------
bool PersistentMesh::Intersects(const Ray &p_ray)
{
	return IntersectP(Ray(p_ray));
}

//----------------------------------------------------------------------------------------------
// Returns a literal with information on the acceleration structure.
//----------------------------------------------------------------------------------------------
std::string PersistentMesh::ToString(void) const
{
	return "Persistent Mesh";
}

//----------------------------------------------------------------------------------------------
float PersistentMesh::GetPdf(const Vector3 &p_point) const { return 1.f / m_fArea; }
//----------------------------------------------------------------------------------------------
Vector3 PersistentMesh::SamplePoint(float p_u, float p_v, Vector3 &p_normal)
{
	return Vector3::Zero;
}
//----------------------------------------------------------------------------------------------
Vector3 PersistentMesh::SamplePoint(const Vector3 &p_viewPoint, float p_u, float p_v, Vector3 &p_normal)
{
	return Vector3::Zero;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
bool PersistentMesh::IntersectP(Ray &p_ray)
{
	// Compute initial parametric range of ray inside kd-tree extent
	float tmin, tmax;
	if (!m_boundingBox.Intersects(p_ray, tmin, tmax))
	{
		return false;
	}

	// Prepare to traverse kd-tree for ray
	Vector3 invDir(p_ray.DirectionInverseCache);
	Ray ray(p_ray);
#define MAX_TODO 64
	PersistentTreeStackElement todo[MAX_TODO];
	int todoPos = 0;

	// Traverse kd-tree nodes in order for ray
	bool hit = false;
	const PersistentTreeNode *node = m_pRootNode;
	while (node != NULL) {
		// Bail out if we found a hit closer than the current node
		if (ray.Max < tmin) break;
		if (node->Axis != 0x03) {
			// Process kd-tree interior node

			// Compute parametric distance along ray to split plane
			int axis = node->Axis;
			float tplane = (node->Partition - ray.Origin[axis]) * invDir[axis];

			// Get node children pointers for ray
			PersistentTreeNode *firstChild, *secondChild;
			int belowFirst = (ray.Origin[axis] <  node->Partition) ||
							 (ray.Origin[axis] == node->Partition && ray.Direction[axis] <= 0);
			
			int *children = (int*)(node + 1);

			if (belowFirst) {
				firstChild = (PersistentTreeNode*)(((char*)m_pRootNode) + children[0]);
				secondChild = (PersistentTreeNode*)(((char*)m_pRootNode) + children[1]);
			}
			else {
				firstChild = (PersistentTreeNode*)(((char*)m_pRootNode) + children[1]);
				secondChild = (PersistentTreeNode*)(((char*)m_pRootNode) + children[0]);
			}

			// Advance to next child node, possibly enqueue other child
			if (tplane > tmax || tplane <= 0)
				node = firstChild;
			else if (tplane < tmin)
				node = secondChild;
			else {
				// Enqueue _secondChild_ in todo list
				todo[todoPos].pNode = secondChild;
				todo[todoPos].Min = tplane;
				todo[todoPos].Max = tmax;
				++todoPos;
				node = firstChild;
				tmax = tplane;
			}
		}
		else {
			for (int count = node->ItemCount, *pTriangleIndex = (int*)(node + 1); count > 0; --count, ++pTriangleIndex)
			{
				PersistentIndexedTriangle *pTriangle = m_pTriangleList + *pTriangleIndex;
				if (IntersectFace(ray, pTriangle))
					return true;
			}

			// Grab next node to process from todo list
			if (todoPos > 0) {
				--todoPos;
				node = todo[todoPos].pNode;
				tmin = todo[todoPos].Min;
				tmax = todo[todoPos].Max;
			}
			else
				break;
		}
	}
	
	return false;
}

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
bool PersistentMesh::IntersectP(Ray &p_ray, DifferentialSurface &p_surface)
{
	// Compute initial parametric range of ray inside kd-tree extent
	float tmin, tmax;
	if (!m_boundingBox.Intersects(p_ray, tmin, tmax))
	{
		return false;
	}

	// Prepare to traverse kd-tree for ray
	Vector3 invDir(p_ray.DirectionInverseCache);
	Ray ray(p_ray);
#define MAX_TODO 64
	PersistentTreeStackElement todo[MAX_TODO];
	int todoPos = 0;

	// Traverse kd-tree nodes in order for ray
	bool hit = false;
	const PersistentTreeNode *node = m_pRootNode;
	while (node != NULL) {
		// Bail out if we found a hit closer than the current node
		if (ray.Max < tmin) break;
		if (node->Axis != 0x03) {
			// Process kd-tree interior node

			// Compute parametric distance along ray to split plane
			int axis = node->Axis;
			float tplane = (node->Partition - ray.Origin[axis]) * invDir[axis];

			// Get node children pointers for ray
			PersistentTreeNode *firstChild, *secondChild;
			int belowFirst = (ray.Origin[axis] <  node->Partition) ||
							 (ray.Origin[axis] == node->Partition && ray.Direction[axis] <= 0);

			int *children = (int*)(node + 1);

			if (belowFirst) {
				firstChild = (PersistentTreeNode*)(((char*)m_pRootNode) + children[0]);
				secondChild = (PersistentTreeNode*)(((char*)m_pRootNode) + children[1]);
			}
			else {
				firstChild = (PersistentTreeNode*)(((char*)m_pRootNode) + children[1]);
				secondChild = (PersistentTreeNode*)(((char*)m_pRootNode) + children[0]);
			}

			// Advance to next child node, possibly enqueue other child
			if (tplane > tmax || tplane <= 0)
				node = firstChild;
			else if (tplane < tmin)
				node = secondChild;
			else {
				// Enqueue _secondChild_ in todo list
				todo[todoPos].pNode = secondChild;
				todo[todoPos].Min = tplane;
				todo[todoPos].Max = tmax;
				++todoPos;
				node = firstChild;
				tmax = tplane;
			}
		}
		else {
			for (int count = node->ItemCount, *pTriangleIndex = (int*)(node + 1); count > 0; --count, ++pTriangleIndex)
			{
				PersistentIndexedTriangle *pTriangle = m_pTriangleList + *pTriangleIndex;
				if (IntersectFace(ray, pTriangle, p_surface))
				{
					hit = true;
					p_ray.Max = ray.Max = Maths::Min(ray.Max, p_surface.Distance);
				}
			}

			// Grab next node to process from todo list
			if (todoPos > 0) {
				--todoPos;
				node = todo[todoPos].pNode;
				tmin = todo[todoPos].Min;
				tmax = todo[todoPos].Max;
			}
			else
				break;
		}
	}
	
	return hit;
}

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
bool PersistentMesh::IntersectFace(Ray p_ray, PersistentIndexedTriangle *p_pTriangle)
{
	float te1xte2[3], 
		edge2[3], 
		interm[3];

	const float *te1 = (p_pTriangle->Edge)->Element;
	const float *te2 = (p_pTriangle->Edge + 1)->Element;

	CROSS(te1xte2, te1, te2);

	const float rcp = 1.0f / DOT(te1xte2, p_ray.Direction.Element);
	SUB(edge2, m_pVertexList[p_pTriangle->VertexID[0]].Position, p_ray.Origin.Element);

	const float toverd = DOT(te1xte2, edge2) * rcp;

	if (toverd > p_ray.Max - 1e-5 || toverd < 1e-5)
		return false;

	CROSS(interm, p_ray.Direction.Element, edge2);
	
	const float uoverd = DOT( interm, te2) * -rcp;
	if ( uoverd < 0.0f)
		return false;

	const float voverd = DOT( interm, te1) * rcp;
	if ( uoverd + voverd > 1.0f || voverd < 0.0f )
		return false;

	return true;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
bool PersistentMesh::IntersectFace(Ray p_ray, PersistentIndexedTriangle *p_pTriangle, DifferentialSurface &p_surface)
{
	float te1xte2[3], 
		edge2[3], 
		interm[3];

	const float *te1 = (p_pTriangle->Edge)->Element;
	const float *te2 = (p_pTriangle->Edge + 1)->Element;

	CROSS(te1xte2, te1, te2);

	const float rcp = 1.0f / DOT(te1xte2, p_ray.Direction.Element);
	SUB(edge2, m_pVertexList[p_pTriangle->VertexID[0]].Position, p_ray.Origin.Element);

	const float t = DOT(te1xte2, edge2) * rcp;

	if (t > p_ray.Max - 1e-5 || t < 1e-5)
		return false;

	CROSS(interm, p_ray.Direction.Element, edge2);
	
	const float beta = DOT( interm, te2) * -rcp;
	if ( beta < 0.0f)
		return false;

	const float gamma = DOT( interm, te1) * rcp;
	if ( beta + gamma > 1.0f || gamma < 0.0f )
		return false;

	const float alpha = 1.f - beta - gamma;
	
	const Vertex &v0 = m_pVertexList[p_pTriangle->VertexID[0]];
	const Vertex &v1 = m_pVertexList[p_pTriangle->VertexID[1]];
	const Vertex &v2 = m_pVertexList[p_pTriangle->VertexID[2]];

		// Populate differential surface
	p_surface.SetShape(this);
	p_surface.Distance = t;
	p_surface.Point = p_ray.PointAlongRay(t);
	
	// Set 2d parametric surface representation
	p_surface.PointUV.Set (
		v0.UV.U * alpha + v1.UV.U * beta + v2.UV.U * gamma,
		v0.UV.V * alpha + v1.UV.V * beta + v2.UV.V * gamma);

	// Set shading normal
	p_surface.ShadingNormal.Set(v0.Normal.X * alpha + v1.Normal.X * beta + v2.Normal.X * gamma,
		v0.Normal.Y * alpha + v1.Normal.Y * beta + v2.Normal.Y * gamma,
		v0.Normal.Z * alpha + v1.Normal.Z * beta + v2.Normal.Z * gamma);
	
	p_surface.ShadingNormal.Normalize();
	p_surface.GeometryNormal = p_surface.ShadingNormal;
	// p_surface.SetGroupId(p_pTriangle->GroupID);

	return true;
}