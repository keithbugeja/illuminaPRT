//----------------------------------------------------------------------------------------------
//	Filename:	Quad.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Shape/Quad.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
Quad::Quad(const std::string &p_strName, 
	const Vector3 &p_vertex1, const Vector3 &p_vertex2, 
	const Vector3 &p_vertex3, const Vector3 &p_vertex4)
	: IShape(p_strName) 
{
	m_tesselatedQuad[0].Vertex[0] = p_vertex1;
	m_tesselatedQuad[0].Vertex[2] = p_vertex2;
	m_tesselatedQuad[0].Vertex[1] = p_vertex3;

	m_tesselatedQuad[0].UV[0].Set(0, 0);
	m_tesselatedQuad[0].UV[2].Set(0, 1);
	m_tesselatedQuad[0].UV[1].Set(1, 1);

	m_tesselatedQuad[1].Vertex[0] = p_vertex1;
	m_tesselatedQuad[1].Vertex[2] = p_vertex3;
	m_tesselatedQuad[1].Vertex[1] = p_vertex4;

	m_tesselatedQuad[1].UV[0].Set(0, 0);
	m_tesselatedQuad[1].UV[2].Set(1, 1);
	m_tesselatedQuad[1].UV[1].Set(0, 1);	
}
//----------------------------------------------------------------------------------------------
Quad::Quad(const std::string &p_strName, 
	const Vector3 &p_vertex1, const Vector3 &p_vertex2, 
	const Vector3 &p_vertex3, const Vector3 &p_vertex4, 
	const Vector2 &p_uv1, const Vector2 &p_uv2, 
	const Vector2 &p_uv3, const Vector2 &p_uv4)
	: IShape(p_strName)
{
	m_tesselatedQuad[0].Vertex[0] = p_vertex1;
	m_tesselatedQuad[0].Vertex[2] = p_vertex2;
	m_tesselatedQuad[0].Vertex[1] = p_vertex3;

	m_tesselatedQuad[0].UV[0] = p_uv1;
	m_tesselatedQuad[0].UV[2] = p_uv2;
	m_tesselatedQuad[0].UV[1] = p_uv3;

	m_tesselatedQuad[1].Vertex[0] = p_vertex1;
	m_tesselatedQuad[1].Vertex[2] = p_vertex3;
	m_tesselatedQuad[1].Vertex[1] = p_vertex4;

	m_tesselatedQuad[1].UV[0] = p_uv1;
	m_tesselatedQuad[1].UV[2] = p_uv3;
	m_tesselatedQuad[1].UV[1] = p_uv4;	
}
//----------------------------------------------------------------------------------------------
Quad::Quad(const Vector3 &p_vertex1, const Vector3 &p_vertex2, 
	const Vector3 &p_vertex3, const Vector3 &p_vertex4)
	: IShape() 
{
	m_tesselatedQuad[0].Vertex[0] = p_vertex1;
	m_tesselatedQuad[0].Vertex[2] = p_vertex2;
	m_tesselatedQuad[0].Vertex[1] = p_vertex3;

	m_tesselatedQuad[0].UV[0].Set(0, 0);
	m_tesselatedQuad[0].UV[2].Set(0, 1);
	m_tesselatedQuad[0].UV[1].Set(1, 1);

	m_tesselatedQuad[1].Vertex[0] = p_vertex1;
	m_tesselatedQuad[1].Vertex[2] = p_vertex3;
	m_tesselatedQuad[1].Vertex[1] = p_vertex4;

	m_tesselatedQuad[1].UV[0].Set(0, 0);
	m_tesselatedQuad[1].UV[2].Set(1, 1);
	m_tesselatedQuad[1].UV[1].Set(0, 1);	
}
//----------------------------------------------------------------------------------------------
Quad::Quad(const Vector3 &p_vertex1, const Vector3 &p_vertex2, 
	const Vector3 &p_vertex3, const Vector3 &p_vertex4, 
	const Vector2 &p_uv1, const Vector2 &p_uv2, 
	const Vector2 &p_uv3, const Vector2 &p_uv4)
	: IShape()
{
	m_tesselatedQuad[0].Vertex[0] = p_vertex1;
	m_tesselatedQuad[0].Vertex[2] = p_vertex2;
	m_tesselatedQuad[0].Vertex[1] = p_vertex3;

	m_tesselatedQuad[0].UV[0] = p_uv1;
	m_tesselatedQuad[0].UV[2] = p_uv2;
	m_tesselatedQuad[0].UV[1] = p_uv3;

	m_tesselatedQuad[1].Vertex[0] = p_vertex1;
	m_tesselatedQuad[1].Vertex[2] = p_vertex3;
	m_tesselatedQuad[1].Vertex[1] = p_vertex4;

	m_tesselatedQuad[1].UV[0] = p_uv1;
	m_tesselatedQuad[1].UV[2] = p_uv3;
	m_tesselatedQuad[1].UV[1] = p_uv4;	
}
//----------------------------------------------------------------------------------------------
Quad::Quad(const Quad &p_quad)
	: IShape()
{
	m_tesselatedQuad[0] = p_quad.m_tesselatedQuad[0];
	m_tesselatedQuad[1] = p_quad.m_tesselatedQuad[1];
	m_boundingBox = p_quad.m_boundingBox;
}
//----------------------------------------------------------------------------------------------
void Quad::ComputeBoundingVolume(void) 
{
	m_tesselatedQuad[0].ComputeBoundingVolume();
	m_tesselatedQuad[1].ComputeBoundingVolume();

	m_boundingBox.ComputeFromVolume(*(m_tesselatedQuad[0].GetBoundingVolume()));
	m_boundingBox.Union(*(m_tesselatedQuad[1].GetBoundingVolume()));
}
//----------------------------------------------------------------------------------------------
IBoundingVolume* Quad::GetBoundingVolume(void) const {
	return (IBoundingVolume*)&m_boundingBox;
}
//----------------------------------------------------------------------------------------------
bool Quad::IsBounded(void) const {
	return true;
}
//----------------------------------------------------------------------------------------------
bool Quad::Intersects(const Ray &p_ray, DifferentialSurface &p_surface)
{
	return m_tesselatedQuad[0].Intersects(p_ray, p_surface) || m_tesselatedQuad[1].Intersects(p_ray, p_surface);
}
//----------------------------------------------------------------------------------------------
bool Quad::Intersects(const Ray &p_ray)
{
	return m_tesselatedQuad[0].Intersects(p_ray) || m_tesselatedQuad[1].Intersects(p_ray);
}
//----------------------------------------------------------------------------------------------
float Quad::GetArea(void) const
{
	return m_tesselatedQuad[0].GetArea() + m_tesselatedQuad[1].GetArea();
}
//----------------------------------------------------------------------------------------------
float Quad::GetPdf(const Vector3 &p_point) const
{
	return 1.0f / GetArea();
}
//----------------------------------------------------------------------------------------------
Vector3 Quad::SamplePoint(float p_u, float p_v, Vector3 &p_normal)
{
	p_normal = Vector3::Cross(m_tesselatedQuad[0].Vertex[1] - m_tesselatedQuad[0].Vertex[0], m_tesselatedQuad[0].Vertex[2] - m_tesselatedQuad[0].Vertex[0]);

	return m_tesselatedQuad[0].Vertex[0] + (m_tesselatedQuad[0].Vertex[1] - m_tesselatedQuad[0].Vertex[0]) * p_u + 
		(m_tesselatedQuad[1].Vertex[1] - m_tesselatedQuad[0].Vertex[1]) * p_v + 
		(m_tesselatedQuad[0].Vertex[0] - m_tesselatedQuad[0].Vertex[1] + m_tesselatedQuad[0].Vertex[2] - m_tesselatedQuad[1].Vertex[1]) * p_u * p_v;
}
//----------------------------------------------------------------------------------------------
