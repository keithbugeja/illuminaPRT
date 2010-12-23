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
void ITriangleMesh<T, U>::AddTriangle(const U &p_v1, const U &p_v2, const U &p_v3)
{
	int  i1 = (int)AddVertex(p_v1),
		 i2 = (int)AddVertex(p_v2),
		 i3 = (int)AddVertex(p_v3);

	AddIndexedTriangle(i1, i2, i3);
}
//----------------------------------------------------------------------------------------------
template<class T, class U>
void ITriangleMesh<T, U>::AddTriangleList(const U *p_pVertexList, int p_nTriangleCount)
{
	for (int idx = 0; idx < p_nTriangleCount; idx++, p_pVertexList+=3)
		AddTriangle(p_pVertexList[0], p_pVertexList[1], p_pVertexList[2]);
}
//----------------------------------------------------------------------------------------------
template<class T, class U>
void ITriangleMesh<T, U>::AddTriangleList(const List<U> &p_vertexList)
{
	for (int idx = 0; idx < p_vertexList.Size(); idx+=3)
		AddTriangle(p_vertexList[0], p_vertexList[1], p_vertexList[2]);
}
//----------------------------------------------------------------------------------------------
template<class T, class U>
void ITriangleMesh<T, U>::AddIndexedTriangle(int p_v1, int p_v2, int p_v3)
{
	TriangleList.PushBack(T(this, p_v1, p_v2, p_v3));
}
//----------------------------------------------------------------------------------------------
template<class T, class U>
void ITriangleMesh<T, U>::AddIndexedTriangleList(const int *p_pIndexList, int p_nTriangleCount)
{
	for (int idx = 0; idx < p_nTriangleCount; idx++, p_pIndexList+=3)
		AddIndexedTriangle(p_pIndexList[0], p_pIndexList[1], p_pIndexList[2]);
}
//----------------------------------------------------------------------------------------------
template<class T, class U>
void ITriangleMesh<T, U>::AddIndexedTriangleList(const List<int> &p_indexList)
{
	for (int idx = 0; idx < p_indexList.Size(); idx+=3)
		AddIndexedTriangle(p_indexList[0], p_indexList[1], p_indexList[2]);
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
Vector3 ITriangleMesh<T, U>::SamplePoint(float p_u, float p_v, Vector3 &p_normal) const 
{
	float sampleBuffer;
	JitterSampler sampler;
	sampler.Get1DSamples(&sampleBuffer, 1);

	int triangleToSample = (int)(TriangleList.Size() * sampleBuffer);
	return TriangleList[triangleToSample].SamplePoint(p_u, p_v, p_normal);
}
//----------------------------------------------------------------------------------------------
template<class T, class U>
Vector3 ITriangleMesh<T, U>::SamplePoint(const Vector3 &p_viewPoint, float p_u, float p_v, Vector3 &p_normal) const {
	return SamplePoint(p_u, p_v, p_normal);
}
//----------------------------------------------------------------------------------------------
template<class T, class U>
void ITriangleMesh<T, U>::ComputeArea(void)
{
	m_fArea = 0;

	for (int triIdx = 0; triIdx < TriangleList.Size(); triIdx++)
		m_fArea += TriangleList[triIdx].GetArea();
}
