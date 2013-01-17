//----------------------------------------------------------------------------------------------
//	Filename:	Triangle.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Shape/Triangle.h"
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
Triangle::Triangle(const std::string &p_strName, const Vector3 &p_vertex1, 
				   const Vector3 &p_vertex2, const Vector3 &p_vertex3)
	: IShape(p_strName)
{
	Vertex[0] = p_vertex1;
	Vertex[1] = p_vertex2;
	Vertex[2] = p_vertex3;

	UV[0].Set(0,0);
	UV[1].Set(1,1);
	UV[2].Set(1,0);
}
//----------------------------------------------------------------------------------------------
Triangle::Triangle(const std::string &p_strName, const Vector3 &p_vertex1, const Vector3 &p_vertex2, 
				   const Vector3 &p_vertex3, const Vector2 &p_uv1, const Vector2 &p_uv2, const Vector2 &p_uv3)
	: IShape(p_strName)
{
	Vertex[0] = p_vertex1;
	Vertex[1] = p_vertex2;
	Vertex[2] = p_vertex3;

	UV[0] = p_uv1;
	UV[1] = p_uv2;
	UV[2] = p_uv3;
}
//----------------------------------------------------------------------------------------------
Triangle::Triangle(const Vector3 &p_vertex1, const Vector3 &p_vertex2, const Vector3 &p_vertex3)
	: IShape()
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
	: IShape()
{
	Vertex[0] = p_vertex1;
	Vertex[1] = p_vertex2;
	Vertex[2] = p_vertex3;

	UV[0] = p_uv1;
	UV[1] = p_uv2;
	UV[2] = p_uv3;
}
//----------------------------------------------------------------------------------------------
Triangle::Triangle(const Triangle &p_triangle)
	: IShape()
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
bool Triangle::Intersects(const Ray &p_ray, DifferentialSurface &p_surface)
{
	//----------------------------------------------------------------------------------------------
	// Shirley's
	//----------------------------------------------------------------------------------------------
	float te1xte2[3], 
		edge2[3], 
		interm[3];

	Vector3 edge[2];
	
	edge[0] = Vertex[1] - Vertex[0];
	edge[1] = Vertex[2] - Vertex[0];

	const float *te1 = edge[0].Element;
	const float *te2 = edge[1].Element;

	CROSS(te1xte2, te1, te2);

	const float rcp = 1.0f / DOT(te1xte2, p_ray.Direction.Element);
	SUB(edge2, Vertex[0], p_ray.Origin.Element);

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
	
	//----------------------------------------------------------------------------------------------
	// Populate differential surface
	p_surface.SetShape(this);
	p_surface.Distance = t;
	p_surface.Point = p_ray.PointAlongRay(t);
	
	// Set 2d parametric surface representation
	p_surface.PointUV.Set (
		UV[0].U * alpha + UV[1].U * beta + UV[2].U * gamma,
		UV[0].V * alpha + UV[1].V * beta + UV[2].V * gamma);

	// Set shading normal
	Vector3::Cross(edge[0], edge[1], p_surface.GeometryNormal);	
	p_surface.GeometryNormal.Normalize();
	p_surface.ShadingNormal = p_surface.GeometryNormal;

	return true;
}
//----------------------------------------------------------------------------------------------
bool Triangle::Intersects(const Ray &p_ray)
{
	//----------------------------------------------------------------------------------------------
	// Shirley's
	//----------------------------------------------------------------------------------------------
	float te1xte2[3], 
		edge2[3], 
		interm[3];

	Vector3 edge[2];
	
	edge[0] = Vertex[1] - Vertex[0];
	edge[1] = Vertex[2] - Vertex[0];

	const float *te1 = edge[0].Element;
	const float *te2 = edge[1].Element;

	CROSS(te1xte2, te1, te2);

	const float rcp = 1.0f / DOT(te1xte2, p_ray.Direction.Element);
	SUB(edge2, Vertex[0], p_ray.Origin.Element);

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
float Triangle::GetArea(void) const
{
	return Vector3::Cross(Vertex[1] - Vertex[0], Vertex[2] - Vertex[0]).Length() * 0.5;
}
//----------------------------------------------------------------------------------------------
float Triangle::GetPdf(const Vector3 &p_point) const
{
	return 1.0f / GetArea();
}
//----------------------------------------------------------------------------------------------
Vector3 Triangle::SamplePoint(float p_u, float p_v, Vector3 &p_normal)
{
	float b1, b2;
	Montecarlo::UniformSampleTriangle(p_u, p_v, &b1, &b2);
	
	Vector3 p = b1 * Vertex[0] + b2 * Vertex[1] + (1.f - b1 - b2) * Vertex[2];
	Vector3 n = Vector3::Cross(Vertex[2]-Vertex[0], Vertex[1]-Vertex[0]);
	p_normal = Vector3::Normalize(n);
	return p;
}
//----------------------------------------------------------------------------------------------
