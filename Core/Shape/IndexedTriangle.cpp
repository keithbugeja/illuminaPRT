//----------------------------------------------------------------------------------------------
//	Filename:	IndexedTriangle.inl
//	Author:		Keith Bugeja
//	Date:		22/03/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Shape/IndexedTriangle.h"
#include "Shape/TriangleMesh.h"
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
IndexedTriangle::IndexedTriangle(ITriangleMesh *p_pMesh, 
	int p_nV1, int p_nV2, int p_nV3, int p_nGroupId)
{
	m_pMesh = p_pMesh;
	
	m_nVertexID[0] = p_nV1;
	m_nVertexID[1] = p_nV2;
	m_nVertexID[2] = p_nV3;

	m_nGroupID = p_nGroupId;

	m_edge[0] = m_pMesh->VertexList[m_nVertexID[1]].Position - m_pMesh->VertexList[m_nVertexID[0]].Position;
	m_edge[1] = m_pMesh->VertexList[m_nVertexID[2]].Position - m_pMesh->VertexList[m_nVertexID[0]].Position;
}
//----------------------------------------------------------------------------------------------
IndexedTriangle::IndexedTriangle(const IndexedTriangle &p_triangle)
{
	m_pMesh = p_triangle.m_pMesh;

	m_nVertexID[0] = p_triangle.m_nVertexID[0];
	m_nVertexID[1] = p_triangle.m_nVertexID[1];
	m_nVertexID[2] = p_triangle.m_nVertexID[2];

	m_nGroupID = p_triangle.m_nGroupID;

	m_edge[0] = m_pMesh->VertexList[m_nVertexID[1]].Position - m_pMesh->VertexList[m_nVertexID[0]].Position;
	m_edge[1] = m_pMesh->VertexList[m_nVertexID[2]].Position - m_pMesh->VertexList[m_nVertexID[0]].Position;
}
//----------------------------------------------------------------------------------------------
bool IndexedTriangle::IsBounded(void) const {
	return true;
}
//----------------------------------------------------------------------------------------------
IBoundingVolume* IndexedTriangle::GetBoundingVolume(void) const {
	return (IBoundingVolume*)&m_boundingBox;
}
//----------------------------------------------------------------------------------------------
void IndexedTriangle::ComputeBoundingVolume(void)
{
	Vector3 vertex[3];
	
	vertex[0] = m_pMesh->VertexList[m_nVertexID[0]].Position;
	vertex[1] = m_pMesh->VertexList[m_nVertexID[1]].Position;
	vertex[2] = m_pMesh->VertexList[m_nVertexID[2]].Position;

	m_boundingBox.ComputeFromPoints(vertex, 3);
}
//----------------------------------------------------------------------------------------------
bool IndexedTriangle::HasGroup(void) const { 
	return m_nGroupID >= 0;
}
//----------------------------------------------------------------------------------------------
int IndexedTriangle::GetGroupId(void) const {
	return m_nGroupID;
}
//----------------------------------------------------------------------------------------------
bool IndexedTriangle::Intersects(const Ray &p_ray, DifferentialSurface &p_surface)
{
	//----------------------------------------------------------------------------------------------
	// Note that p_surface is not modified unless there has been an intersection
	//----------------------------------------------------------------------------------------------
	
	//----------------------------------------------------------------------------------------------
	// Shirley's 
	//----------------------------------------------------------------------------------------------
	float te1xte2[3], 
		edge2[3], 
		interm[3];

	const float *te1 = GetEdgeP(0);
	const float *te2 = GetEdgeP(1);

	CROSS(te1xte2, te1, te2);

	const float rcp = 1.0f / DOT(te1xte2, p_ray.Direction.Element);
	SUB(edge2, m_pMesh->VertexList[m_nVertexID[0]].Position, p_ray.Origin.Element);

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
	
	const Vertex &v0 = m_pMesh->VertexList[m_nVertexID[0]];
	const Vertex &v1 = m_pMesh->VertexList[m_nVertexID[1]];
	const Vertex &v2 = m_pMesh->VertexList[m_nVertexID[2]];
	
	//----------------------------------------------------------------------------------------------
	// Moeller-Trumbore
	//----------------------------------------------------------------------------------------------
	/**/ /*
	double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	double det, inv_det;
	float alpha, beta, gamma, t;

	const Vertex &v0 = m_pMesh->VertexList[m_nVertexID[0]],
		&v1 = m_pMesh->VertexList[m_nVertexID[1]],
		&v2 = m_pMesh->VertexList[m_nVertexID[2]];

	SUB(edge1, v1.Position.Element, v0.Position.Element);
	SUB(edge2, v2.Position.Element, v0.Position.Element);

	CROSS(pvec, p_ray.Direction.Element, edge2);

	det = DOT(edge1, pvec);
	
	if (det > -EPSILON && det < EPSILON)
		return false;

	inv_det = 1.0 / det;

	SUB(tvec, p_ray.Origin.Element, v0.Position.Element);

	beta = DOT(tvec, pvec) * inv_det;
	if (beta < 0.0 || beta > 1.0)
		return false;

	CROSS(qvec, tvec, edge1);

	gamma = DOT(p_ray.Direction.Element, qvec) * inv_det;
	if (gamma < 0.0 || gamma > 1.0 || (alpha = 1.0 - beta - gamma) < 0)
		return false;

	t = DOT(edge2, qvec) * inv_det;
	if (t < p_ray.Min || t > p_ray.Max)
		return false;
	/**/

	//----------------------------------------------------------------------------------------------
	// Reference
	//----------------------------------------------------------------------------------------------
	/**//*
	const Vertex &v0 = m_pMesh->VertexList[m_nVertexID[0]];
	const Vertex &v1 = m_pMesh->VertexList[m_nVertexID[1]];
	const Vertex &v2 = m_pMesh->VertexList[m_nVertexID[2]];

	const Vector3 &OA = p_ray.Origin - v0.Position;
	const Vector3 &BA = v1.Position - v0.Position;
	const Vector3 &CA = v2.Position - v0.Position;
	const Vector3 &D = -p_ray.Direction;

	// Use Cramer's Rule to solve for beta, gamma and t:
	// Since Xi = det(Ai)/det(A), find det(A):
	Matrix3x3 A(BA, CA, D, false);
	float detA = A.Determinant();
	
	if (detA > -EPSILON && detA < EPSILON)
		return false;

	float invDetA = 1.0 / detA;

	// Find beta
	A.SetColumn(0, OA);
	float beta = A.Determinant() * invDetA;
	if (beta < 0.0f || beta > 1.0f) 
		return false;

	// Find gamma
	A.SetColumn(0, BA);
	A.SetColumn(1, OA);
	float gamma = A.Determinant() * invDetA;
	if (gamma < 0.0f || gamma > 1.0f) 
		return false;

	// alpha = 1 - (beta + gamma)
	float alpha = 1 - beta - gamma;
	if (alpha < 0.0f || alpha > 1.0f) 
		return false;

	// Find t
	A.SetColumn(1, CA);
	A.SetColumn(2, OA);
	float t = A.Determinant() * invDetA;
	if (t < p_ray.Min || t > p_ray.Max)
		return false;
	/**/
	//----------------------------------------------------------------------------------------------

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

	/* 
	if (p_surface.ShadingNormal.X != p_surface.ShadingNormal.X)
		std::cerr << "Warning : Indeterminate normal computation!" << std::endl;
	*/

	return true;
}
//----------------------------------------------------------------------------------------------
bool IndexedTriangle::Intersects(const Ray &p_ray)
{	
	//----------------------------------------------------------------------------------------------
	// Shirley's
	//----------------------------------------------------------------------------------------------
	float te1xte2[3], 
		edge2[3], 
		interm[3];

	const float *te1 = GetEdgeP(0);
	const float *te2 = GetEdgeP(1);

	CROSS(te1xte2, te1, te2);

	const float rcp = 1.0f / DOT(te1xte2, p_ray.Direction.Element);
	SUB(edge2, m_pMesh->VertexList[m_nVertexID[0]].Position, p_ray.Origin.Element);

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

	//----------------------------------------------------------------------------------------------
	// Muller-Trumbore
	//----------------------------------------------------------------------------------------------
	/**//*
	double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	double det, inv_det;
	float u, v, a, t;

	const Vertex &v0 = m_pMesh->VertexList[m_nVertexID[0]],
		&v1 = m_pMesh->VertexList[m_nVertexID[1]],
		&v2 = m_pMesh->VertexList[m_nVertexID[2]];

	SUB(edge1, v1.Position.Element, v0.Position.Element);
	SUB(edge2, v2.Position.Element, v0.Position.Element);

	CROSS(pvec, p_ray.Direction.Element, edge2);

	det = DOT(edge1, pvec);
	
	if (det > -EPSILON && det < EPSILON)
		return false;

	inv_det = 1.0 / det;

	SUB(tvec, p_ray.Origin.Element, v0.Position.Element);

	u = DOT(tvec, pvec) * inv_det;
	if (u < 0.0 || u > 1.0)
		return false;

	CROSS(qvec, tvec, edge1);

	v = DOT(p_ray.Direction.Element, qvec) * inv_det;
	if (v < 0.0 || v > 1.0 || (1 - u - v < 0))
		return false;

	t = DOT(edge2, qvec) * inv_det;
	if (t < p_ray.Min || t > p_ray.Max)
		return false;

	return true;
	/**/

	//----------------------------------------------------------------------------------------------
	// Reference
	//----------------------------------------------------------------------------------------------
	/**/ /*
	const Vertex &v0 = m_pMesh->VertexList[m_nVertexID[0]],
		&v1 = m_pMesh->VertexList[m_nVertexID[1]],
		&v2 = m_pMesh->VertexList[m_nVertexID[2]];

	const Vector3 &OA = p_ray.Origin - v0.Position,
		&BA = v1.Position - v0.Position,
		&CA = v2.Position - v0.Position,
		&D = -p_ray.Direction;

	//td + o = a + B(b-a) + G(c-a)

	// Use Cramer's Rule to solve for beta, gamma and t:
	// Since Xi = det(Ai)/det(A), find det(A):
	Matrix3x3 A(BA, CA, D, false);
	float detA = A.Determinant();
	float invDetA = 1.0 / detA;

	// Find beta
	A.SetColumn(0, OA);
	//float beta = A.Determinant() / detA;
	float beta = A.Determinant() * invDetA;
	if (beta < 0.0f || beta > 1.0f) 
		return false;

	// Find gamma
	A.SetColumn(0, BA);
	A.SetColumn(1, OA);
	//float gamma = A.Determinant() / detA;
	float gamma = A.Determinant() * invDetA;
	if (gamma < 0.0f || gamma > 1.0f) 
		return false;

	// alpha = 1 - (beta + gamma)
	float alpha = 1 - beta - gamma;
	if (alpha < 0.0f || alpha > 1.0f) 
		return false;
	
	// Find t
	A.SetColumn(1, CA);
	A.SetColumn(2, OA);
	//float t = A.Determinant() / detA;
	float t = A.Determinant() * invDetA;
	if (t < p_ray.Min || t > p_ray.Max)
		return false;

	return true;
	/**/
}
//----------------------------------------------------------------------------------------------
float IndexedTriangle::GetArea(void) const
{
	const Vertex &v0 = m_pMesh->VertexList[m_nVertexID[0]],
		&v1 = m_pMesh->VertexList[m_nVertexID[1]],
		&v2 = m_pMesh->VertexList[m_nVertexID[2]];

	return Vector3::Cross(v1.Position - v0.Position, v2.Position - v0.Position).Length() * 0.5;
}
//----------------------------------------------------------------------------------------------
float IndexedTriangle::GetPdf(const Vector3 &p_point) const
{
	return 1.0f / GetArea();
}
//----------------------------------------------------------------------------------------------
Vector3 IndexedTriangle::SamplePoint(float p_u, float p_v, Vector3 &p_normal)
{
	float b1, b2;
	Montecarlo::UniformSampleTriangle(p_u, p_v, &b1, &b2);
	
	// Get triangle vertices in _p1_, _p2_, and _p3_
	const Vector3 &p1 = m_pMesh->VertexList[m_nVertexID[0]].Position;
	const Vector3 &p2 = m_pMesh->VertexList[m_nVertexID[1]].Position;
	const Vector3 &p3 = m_pMesh->VertexList[m_nVertexID[2]].Position;

	Vector3 p = b1 * p1 + b2 * p2 + (1.f - b1 - b2) * p3;
	Vector3 n = Vector3::Cross(p3-p1, p2-p1);
	p_normal = Vector3::Normalize(n);
	return p;
}
//----------------------------------------------------------------------------------------------
IndexedTriangle& IndexedTriangle::operator=(const IndexedTriangle &p_indexedTriangle)
{
	m_nVertexID[0] = p_indexedTriangle.m_nVertexID[0];
	m_nVertexID[1] = p_indexedTriangle.m_nVertexID[1];
	m_nVertexID[2] = p_indexedTriangle.m_nVertexID[2];

	m_edge[0] = p_indexedTriangle.m_edge[0];
	m_edge[1] = p_indexedTriangle.m_edge[1];

	m_pMesh = p_indexedTriangle.m_pMesh;
	m_boundingBox = p_indexedTriangle.m_boundingBox;

	return *this;
}
//----------------------------------------------------------------------------------------------