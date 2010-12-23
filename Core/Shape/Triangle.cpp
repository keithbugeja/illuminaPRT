//----------------------------------------------------------------------------------------------
//	Filename:	TriangleUV.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Shape/Triangle.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
Triangle::Triangle(const Vector3 &p_vertex1, const Vector3 &p_vertex2, const Vector3 &p_vertex3)
{
	Vertex[0] = p_vertex1;
	Vertex[1] = p_vertex2;
	Vertex[2] = p_vertex3;

	UV[0].Set(0,0);
	UV[1].Set(1,1);
	UV[2].Set(1,0);
}
//----------------------------------------------------------------------------------------------
Triangle::Triangle(const Vector3 &p_vertex1, const Vector3 &p_vertex2, const Vector3 &p_vertex3,
				   const Vector2 &p_uv1, const Vector2 &p_uv2, const Vector2 &p_uv3)
{
	Vertex[0] = p_vertex1;
	Vertex[1] = p_vertex2;
	Vertex[2] = p_vertex3;

	UV[0] = p_uv1;
	UV[0] = p_uv2;
	UV[0] = p_uv3;
}
//----------------------------------------------------------------------------------------------
Triangle::Triangle(const Triangle &p_triangle)
{
	Vertex[0] = p_triangle.Vertex[0];
	Vertex[1] = p_triangle.Vertex[1];
	Vertex[2] = p_triangle.Vertex[2];

	UV[0] = p_triangle.UV[0];
	UV[1] = p_triangle.UV[1];
	UV[2] = p_triangle.UV[2];
}
//----------------------------------------------------------------------------------------------
void Triangle::ComputeBoundingVolume(void) {
	m_boundingBox.ComputeFromPoints(Vertex, 3);
}
//----------------------------------------------------------------------------------------------
IBoundingVolume* Triangle::GetBoundingVolume(void) const {
	return (IBoundingVolume*)&m_boundingBox;
}
//----------------------------------------------------------------------------------------------
bool Triangle::IsBounded(void) const {
	return true;
}
//----------------------------------------------------------------------------------------------
bool Triangle::Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
{
	const Vector3 &OA = p_ray.Origin - Vertex[0],
		&BA = Vertex[1] - Vertex[0],
		&CA = Vertex[2] - Vertex[0],
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

	// Populate 
	p_surface.SetShape(this);
	p_surface.Distance = t;
	p_surface.Point = p_ray.PointAlongRay(t);
	p_surface.PointUV.Set(
		UV[0].U * alpha + UV[1].U * beta + UV[2].U * gamma, 
		UV[0].V * alpha + UV[1].V * beta + UV[2].V * gamma);

	Vector3::Cross(BA, CA, p_surface.GeometryNormal);

	return true;
}
//----------------------------------------------------------------------------------------------
bool Triangle::Intersects(const Ray &p_ray, float p_fTime)
{
	const Vector3 &OA = p_ray.Origin - Vertex[0],
		&BA = Vertex[1] - Vertex[0],
		&CA = Vertex[2] - Vertex[0],
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

	return true;
}
//----------------------------------------------------------------------------------------------
