//----------------------------------------------------------------------------------------------
//	Filename:	TriangleMesh.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//  TODO: Use surface-area based CDF to determine which face should be sampled.
//----------------------------------------------------------------------------------------------
#include "Shape/TriangleMesh.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
ITriangleMesh::ITriangleMesh(void) 
	: IShape() 
{ }
//----------------------------------------------------------------------------------------------
ITriangleMesh::ITriangleMesh(const std::string &p_strName) 
	: IShape(p_strName) 
{ }
//----------------------------------------------------------------------------------------------
bool ITriangleMesh::IsBounded(void) const {
	return false;
}
//----------------------------------------------------------------------------------------------
void ITriangleMesh::ComputeBoundingVolume(void) 
{
	m_boundingBox.Invalidate();

	for (int nIdx = 0, count = (int)TriangleList.Size(); nIdx < count; nIdx++)
	{
		IndexedTriangle& triangle = TriangleList.At(nIdx);
		triangle.ComputeBoundingVolume();	
		m_boundingBox.Union(*(triangle.GetBoundingVolume()));
	}
}
//----------------------------------------------------------------------------------------------
IBoundingVolume* ITriangleMesh::GetBoundingVolume(void) const {
	return (IBoundingVolume*)&m_boundingBox;
}
//----------------------------------------------------------------------------------------------
size_t ITriangleMesh::AddVertex(const Vertex &p_vertex)
{
	VertexList.PushBack(p_vertex);
	return VertexList.Size() - 1;
}
//----------------------------------------------------------------------------------------------
void ITriangleMesh::AddVertexList(const Vertex *p_pVertex, int p_nCount)
{
	for (int idx = 0; idx < p_nCount; idx++, p_pVertex++)
		AddVertex(*p_pVertex);
}
//----------------------------------------------------------------------------------------------
void ITriangleMesh::AddVertexList(const List<Vertex> &p_vertexList)
{
	for (int idx = 0; idx < p_vertexList.Size(); idx++)
		AddVertex(p_vertexList[idx]);
}
//----------------------------------------------------------------------------------------------
void ITriangleMesh::AddTriangle(const Vertex &p_v1, const Vertex &p_v2, const Vertex &p_v3, int p_nGroupId)
{
	int  i1 = (int)AddVertex(p_v1),
		 i2 = (int)AddVertex(p_v2),
		 i3 = (int)AddVertex(p_v3);

	AddIndexedTriangle(i1, i2, i3, p_nGroupId);
}
//----------------------------------------------------------------------------------------------
void ITriangleMesh::AddTriangleList(const Vertex *p_pVertexList, int p_nTriangleCount, int p_nGroupId)
{
	for (int idx = 0; idx < p_nTriangleCount; idx++, p_pVertexList+=3)
		AddTriangle(p_pVertexList[0], p_pVertexList[1], p_pVertexList[2], p_nGroupId);
}
//----------------------------------------------------------------------------------------------
void ITriangleMesh::AddTriangleList(const List<Vertex> &p_vertexList, int p_nGroupId)
{
	for (int idx = 0; idx < p_vertexList.Size(); idx+=3)
		AddTriangle(p_vertexList[0], p_vertexList[1], p_vertexList[2], p_nGroupId);
}
//----------------------------------------------------------------------------------------------
void ITriangleMesh::AddIndexedTriangle(int p_v1, int p_v2, int p_v3, int p_nGroupId)
{
	TriangleList.PushBack(IndexedTriangle(this, p_v1, p_v2, p_v3, p_nGroupId));
}
//----------------------------------------------------------------------------------------------
void ITriangleMesh::AddIndexedTriangleList(const int *p_pIndexList, int p_nTriangleCount, int p_nGroupId)
{
	for (int idx = 0; idx < p_nTriangleCount; idx++, p_pIndexList+=3)
		AddIndexedTriangle(p_pIndexList[0], p_pIndexList[1], p_pIndexList[2], p_nGroupId);
}
//----------------------------------------------------------------------------------------------
void ITriangleMesh::AddIndexedTriangleList(const List<int> &p_indexList, int p_nGroupId)
{
	for (int idx = 0; idx < p_indexList.Size(); idx+=3)
		AddIndexedTriangle(p_indexList[0], p_indexList[1], p_indexList[2], p_nGroupId);
}
//----------------------------------------------------------------------------------------------
bool ITriangleMesh::UpdateNormals(void)
{
	// Check for normal semantic before applying normal update
	if ((Vertex::GetDescriptor() & VertexFormat::Normal) > 0)
	{
		Vector3 vec3Edge1, vec3Edge2, vec3Normal;
		
		int nVertexCount = VertexList.Size();

		// Clear all vertex normals
		for (int nVertexIndex = 0; nVertexIndex < nVertexCount; nVertexIndex++)
			VertexList[nVertexIndex].Normal = Vector3::Zero;

		// Find normals
		for (int nFaceIndex = 0, nFaceCount = TriangleList.Size(); nFaceIndex < nFaceCount; nFaceIndex++)
		{
			IndexedTriangle &face = TriangleList.At(nFaceIndex);
			
			vec3Edge1 = VertexList[face.GetVertexIndex(1)].Position - VertexList[face.GetVertexIndex(0)].Position;
			vec3Edge2 = VertexList[face.GetVertexIndex(2)].Position - VertexList[face.GetVertexIndex(0)].Position;
			Vector3::Cross(vec3Edge1, vec3Edge2, vec3Normal);

			VertexList[face.GetVertexIndex(0)].Normal += vec3Normal;
			VertexList[face.GetVertexIndex(1)].Normal += vec3Normal;
			VertexList[face.GetVertexIndex(2)].Normal += vec3Normal;
		}

		// Normalize all vertex normals
		for (int nVertexIndex = 0; nVertexIndex < nVertexCount; nVertexIndex++)
		{
			VertexList[nVertexIndex].Normal = Vector3::Normalize(VertexList[nVertexIndex].Normal);
		}

		return true;
	}

	return false;
}
//----------------------------------------------------------------------------------------------
float ITriangleMesh::GetArea(void) const {
	return m_fArea;
}
//----------------------------------------------------------------------------------------------
float ITriangleMesh::GetPdf(const Vector3 &p_point) const {
	return 1.0f / m_fArea;
}
//----------------------------------------------------------------------------------------------
Vector3 ITriangleMesh::SamplePoint(float p_u, float p_v, Vector3 &p_normal) 
{
	// TODO: Use CDF (pdf ~ area) to determine which face to use.
		
	//int triangleToSample = (int)(TriangleList.Size() * m_random.NextFloat());
	//int triangleToSample = 0; //Maths::Max(0, Maths::Min(TriangleList.Size() - 1, TriangleList.Size() * (1 - p_u)));
	//int triangleToSample = 1;
	int triangleToSample = Maths::Min(TriangleList.Size() * ((p_u + p_v) * 0.5f), TriangleList.Size() - 1);
	return TriangleList[triangleToSample].SamplePoint(p_u, p_v, p_normal);
}
//----------------------------------------------------------------------------------------------
Vector3 ITriangleMesh::SamplePoint(const Vector3 &p_viewPoint, float p_u, float p_v, Vector3 &p_normal) 
{
	Vector3 samplePoint;

	samplePoint = SamplePoint(p_u, p_v, p_normal);

	float cosTheta = Vector3::Dot((samplePoint - p_viewPoint), p_normal);

	if (cosTheta > 0)
		p_normal = -p_normal;

	return samplePoint;
}
//----------------------------------------------------------------------------------------------
void ITriangleMesh::ComputeArea(void)
{
	m_fArea = 0;

	for (size_t triIdx = 0; triIdx < TriangleList.Size(); triIdx++)
		m_fArea += TriangleList[triIdx].GetArea();
}
//----------------------------------------------------------------------------------------------