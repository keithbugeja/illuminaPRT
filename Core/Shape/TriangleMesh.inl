//----------------------------------------------------------------------------------------------
//	Filename:	MeshTriangle.inl
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
template<class T, class U> 
bool ITriangleMesh<T, U>::IsBounded(void) const {
	return false;
}
//----------------------------------------------------------------------------------------------
template<class T, class U> 
void ITriangleMesh<T, U>::ComputeBoundingVolume(void) 
{
	m_boundingBox.Invalidate();

	for (int nIdx = 0, count = (int)TriangleList.Size(); nIdx < count; nIdx++)
	{
		T& triangle = TriangleList.At(nIdx);
		triangle.ComputeBoundingVolume();	
		m_boundingBox.Union(*(triangle.GetBoundingVolume()));
	}

	std::cout << "Volume : " << m_boundingBox.ToString() << std::endl;
}
//----------------------------------------------------------------------------------------------
template<class T, class U> 
IBoundingVolume* ITriangleMesh<T, U>::GetBoundingVolume(void) const {
	return (IBoundingVolume*)&m_boundingBox;
}
//----------------------------------------------------------------------------------------------
template<class T, class U> 
size_t ITriangleMesh<T, U>::AddVertex(const U &p_vertex)
{
	VertexList.PushBack(p_vertex);
	return VertexList.Size() - 1;
}
//----------------------------------------------------------------------------------------------
template<class T, class U> 
void ITriangleMesh<T, U>::AddVertexList(const U *p_pVertex, int p_nCount)
{
	for (int idx = 0; idx < p_nCount; idx++, p_pVertex++)
		AddVertex(*p_pVertex);
}
//----------------------------------------------------------------------------------------------
template<class T, class U> 
void ITriangleMesh<T, U>::AddVertexList(const List<U> &p_vertexList)
{
	for (int idx = 0; idx < p_vertexList.Size(); idx++)
		AddVertex(p_vertexList[idx]);
}
//----------------------------------------------------------------------------------------------
template<class T, class U> 
void ITriangleMesh<T, U>::AddTriangle(const U &p_v1, const U &p_v2, const U &p_v3, int p_nGroupId)
{
	int  i1 = (int)AddVertex(p_v1),
		 i2 = (int)AddVertex(p_v2),
		 i3 = (int)AddVertex(p_v3);

	AddIndexedTriangle(i1, i2, i3, p_nGroupId);
}
//----------------------------------------------------------------------------------------------
template<class T, class U>
void ITriangleMesh<T, U>::AddTriangleList(const U *p_pVertexList, int p_nTriangleCount, int p_nGroupId)
{
	for (int idx = 0; idx < p_nTriangleCount; idx++, p_pVertexList+=3)
		AddTriangle(p_pVertexList[0], p_pVertexList[1], p_pVertexList[2], p_nGroupId);
}
//----------------------------------------------------------------------------------------------
template<class T, class U>
void ITriangleMesh<T, U>::AddTriangleList(const List<U> &p_vertexList, int p_nGroupId)
{
	for (int idx = 0; idx < p_vertexList.Size(); idx+=3)
		AddTriangle(p_vertexList[0], p_vertexList[1], p_vertexList[2], p_nGroupId);
}
//----------------------------------------------------------------------------------------------
template<class T, class U>
void ITriangleMesh<T, U>::AddIndexedTriangle(int p_v1, int p_v2, int p_v3, int p_nGroupId)
{
	TriangleList.PushBack(T(this, p_v1, p_v2, p_v3, p_nGroupId));
}
//----------------------------------------------------------------------------------------------
template<class T, class U>
void ITriangleMesh<T, U>::AddIndexedTriangleList(const int *p_pIndexList, int p_nTriangleCount, int p_nGroupId)
{
	for (int idx = 0; idx < p_nTriangleCount; idx++, p_pIndexList+=3)
		AddIndexedTriangle(p_pIndexList[0], p_pIndexList[1], p_pIndexList[2], p_nGroupId);
}
//----------------------------------------------------------------------------------------------
template<class T, class U>
void ITriangleMesh<T, U>::AddIndexedTriangleList(const List<int> &p_indexList, int p_nGroupId)
{
	for (int idx = 0; idx < p_indexList.Size(); idx+=3)
		AddIndexedTriangle(p_indexList[0], p_indexList[1], p_indexList[2], p_nGroupId);
}
//----------------------------------------------------------------------------------------------
template<class T, class U>
bool ITriangleMesh<T, U>::UpdateNormals(void)
{
	// Try fixing vertices instead
	if ((U::GetDescriptor() & VertexFormat::Normal) > 0)
	{
		Vector3 vec3Edge1, vec3Edge2, vec3Normal;

		int nVertexCount = VertexList.Size();

		// Find normals
		for (int nFaceIndex = 0, nFaceCount = TriangleList.Size(); nFaceIndex < nFaceCount; nFaceIndex++)
		{
			T& face = TriangleList.At(nFaceIndex);
			
			vec3Edge1 = VertexList[face.GetVertexIndex(1)].Position - VertexList[face.GetVertexIndex(0)].Position;
			vec3Edge2 = VertexList[face.GetVertexIndex(2)].Position - VertexList[face.GetVertexIndex(0)].Position;
			Vector3::Cross(vec3Edge1, vec3Edge2, vec3Normal);

			for (int vertexIndex = 0; vertexIndex < 3; vertexIndex++)
			{
				if (VertexList[face.GetVertexIndex(vertexIndex)].Normal.Dot(vec3Normal) < 0)
					VertexList[face.GetVertexIndex(vertexIndex)].Normal = -VertexList[face.GetVertexIndex(vertexIndex)].Normal;
			}
		}

		// Normalize all vertex normals
		for (int nVertexIndex = 0; nVertexIndex < nVertexCount; nVertexIndex++)
		{
			VertexList[nVertexIndex].Normal = Vector3::Normalize(VertexList[nVertexIndex].Normal);
		}

		return true;
	}
/*
	// Check for normal semantic before applying normal update
	if ((U::GetDescriptor() & VertexFormat::Normal) > 0)
	{
		Vector3 vec3Edge1, vec3Edge2, vec3Normal;
		
		int nVertexCount = VertexList.Size();

		// Clear all vertex normals
		for (int nVertexIndex = 0; nVertexIndex < nVertexCount; nVertexIndex++)
			VertexList[nVertexIndex].Normal = Vector3::Zero;

		// Find normals
		for (int nFaceIndex = 0, nFaceCount = TriangleList.Size(); nFaceIndex < nFaceCount; nFaceIndex++)
		{
			T& face = TriangleList.At(nFaceIndex);
			
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
*/
	return false;
}
//----------------------------------------------------------------------------------------------
template<class T, class U>
float ITriangleMesh<T, U>::GetArea(void) const {
	return m_fArea;
}
//----------------------------------------------------------------------------------------------
template<class T, class U>
float ITriangleMesh<T, U>::GetPdf(const Vector3 &p_point) const {
	return 1.0f / m_fArea;
}
//----------------------------------------------------------------------------------------------
template<class T, class U>
Vector3 ITriangleMesh<T, U>::SamplePoint(float p_u, float p_v, Vector3 &p_normal) 
{
	int triangleToSample = (int)(TriangleList.Size() * ((p_u + p_v) * 0.5f));
	return TriangleList[triangleToSample].SamplePoint(p_u, p_v, p_normal);
}
//----------------------------------------------------------------------------------------------
template<class T, class U>
Vector3 ITriangleMesh<T, U>::SamplePoint(const Vector3 &p_viewPoint, float p_u, float p_v, Vector3 &p_normal) 
{
	Vector3 samplePoint = SamplePoint(p_u, p_v, p_normal);

	DifferentialSurface surface;	
	Ray ray(p_viewPoint, Vector3::Normalize(samplePoint - p_viewPoint), 1e-4f);
	
	if (Intersects(ray, 0, surface))
	{
		Vector3 surfacePoint = ray.PointAlongRay(surface.Distance);
		p_normal = surface.GeometryNormal; 

		return surfacePoint;
	}

	return samplePoint;
	//surface.Distance = Vector3::Dot(viewToCentre, Vector3::Normalize(ray.Direction));
}
//----------------------------------------------------------------------------------------------
template<class T, class U>
void ITriangleMesh<T, U>::ComputeArea(void)
{
	m_fArea = 0;

	for (int triIdx = 0; triIdx < TriangleList.Size(); triIdx++)
		m_fArea += TriangleList[triIdx].GetArea();
}
//----------------------------------------------------------------------------------------------
