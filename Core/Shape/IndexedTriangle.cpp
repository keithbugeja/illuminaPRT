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
IndexedTriangle::IndexedTriangle(ITriangleMesh *p_pMesh, 
	int p_nV1, int p_nV2, int p_nV3, int p_nGroupId)
{
	m_pMesh = p_pMesh;
	
	m_nVertexID[0] = p_nV1;
	m_nVertexID[1] = p_nV2;
	m_nVertexID[2] = p_nV3;

	m_nGroupID = p_nGroupId;
}
//----------------------------------------------------------------------------------------------
IndexedTriangle::IndexedTriangle(const IndexedTriangle &p_triangle)
{
	m_pMesh = p_triangle.m_pMesh;

	m_nVertexID[0] = p_triangle.m_nVertexID[0];
	m_nVertexID[1] = p_triangle.m_nVertexID[1];
	m_nVertexID[2] = p_triangle.m_nVertexID[2];

	m_nGroupID = p_triangle.m_nGroupID;
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
bool IndexedTriangle::Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
{
	const Vertex &v0 = m_pMesh->VertexList[m_nVertexID[0]],
		&v1 = m_pMesh->VertexList[m_nVertexID[1]],
		&v2 = m_pMesh->VertexList[m_nVertexID[2]];

	const Vector3 &OA = p_ray.Origin - v0.Position,
		&BA = v1.Position - v0.Position,
		&CA = v2.Position - v0.Position,
		&D = -p_ray.Direction;

	// Use Cramer's Rule to solve for beta, gamma and t:
	// Since Xi = det(Ai)/det(A), find det(A):
	Matrix3x3 A(BA, CA, D, false);
	float detA = A.Determinant();

	// Find beta
	A.SetColumn(0, OA);
	float beta = A.Determinant() / detA;
	if (beta < 0.0f || beta > 1.0f) 
		return false;

	// Find gamma
	A.SetColumn(0, BA);
	A.SetColumn(1, OA);
	float gamma = A.Determinant() / detA;
	if (gamma < 0.0f || gamma > 1.0f) 
		return false;

	// alpha = 1 - (beta + gamma)
	float alpha = 1 - beta - gamma;
	if (alpha < 0.0f || alpha > 1.0f) 
		return false;

	// Find t
	A.SetColumn(1, CA);
	A.SetColumn(2, OA);
	float t = A.Determinant() / detA;
	if (t < p_ray.Min || t > p_ray.Max)
		return false;

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

	//// Set geometry normal
	//p_surface.GeometryNormal = Vector3::Normalize(Vector3::Cross(CA, BA));

	//if (p_surface.GeometryNormal.Dot(p_surface.ShadingNormal) < 0)
	//	p_surface.GeometryNormal = -p_surface.GeometryNormal;

	return true;
}
//----------------------------------------------------------------------------------------------
bool IndexedTriangle::Intersects(const Ray &p_ray, float p_fTime)
{
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

	// Find beta
	A.SetColumn(0, OA);
	float beta = A.Determinant() / detA;
	if (beta < 0.0f || beta > 1.0f) 
		return false;

	// Find gamma
	A.SetColumn(0, BA);
	A.SetColumn(1, OA);
	float gamma = A.Determinant() / detA;
	if (gamma < 0.0f || gamma > 1.0f) 
		return false;

	// alpha = 1 - (beta + gamma)
	float alpha = 1 - beta - gamma;
	if (alpha < 0.0f || alpha > 1.0f) 
		return false;

	// Find t
	A.SetColumn(1, CA);
	A.SetColumn(2, OA);
	float t = A.Determinant() / detA;
	if (t < p_ray.Min || t > p_ray.Max)
		return false;

	return true;
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

	m_pMesh = p_indexedTriangle.m_pMesh;
	m_boundingBox = p_indexedTriangle.m_boundingBox;

	return *this;
}
//----------------------------------------------------------------------------------------------