//----------------------------------------------------------------------------------------------
//	Filename:	IndexedTriangle.inl
//	Author:		Keith Bugeja
//	Date:		22/03/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

namespace Illumina {
namespace Core {

//----------------------------------------------------------------------------------------------
template<class TVertex> 
IndexedTriangle<TVertex>::IndexedTriangle(const ITriangleMesh<IndexedTriangle, TVertex> *p_pMesh, int p_nV1, int p_nV2, int p_nV3)
{
	m_pMesh = (ITriangleMesh<IndexedTriangle, TVertex>*)p_pMesh;
	
	m_nVertexID[0] = p_nV1;
	m_nVertexID[1] = p_nV2;
	m_nVertexID[2] = p_nV3;
}
//----------------------------------------------------------------------------------------------
template<class TVertex> 
IndexedTriangle<TVertex>::IndexedTriangle(const IndexedTriangle &p_triangle)
{
	m_pMesh = p_triangle.m_pMesh;

	m_nVertexID[0] = p_triangle.m_nVertexID[0];
	m_nVertexID[1] = p_triangle.m_nVertexID[1];
	m_nVertexID[2] = p_triangle.m_nVertexID[2];
}
//----------------------------------------------------------------------------------------------
template<class TVertex> 
bool IndexedTriangle<TVertex>::IsBounded(void) const {
	return true;
}
//----------------------------------------------------------------------------------------------
template<class TVertex> 
IBoundingVolume* IndexedTriangle<TVertex>::GetBoundingVolume(void) const {
	return (IBoundingVolume*)&m_boundingBox;
}
//----------------------------------------------------------------------------------------------
template<class TVertex> 
void IndexedTriangle<TVertex>::ComputeBoundingVolume(void)
{
	Vector3 vertex[3];
	
	vertex[0] = m_pMesh->VertexList[m_nVertexID[0]].Position;
	vertex[1] = m_pMesh->VertexList[m_nVertexID[1]].Position;
	vertex[2] = m_pMesh->VertexList[m_nVertexID[2]].Position;

	m_boundingBox.ComputeFromPoints(vertex, 3);
}
//----------------------------------------------------------------------------------------------
template<class TVertex>
bool IndexedTriangle<TVertex>::Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
{
	const TVertex &v0 = m_pMesh->VertexList[m_nVertexID[0]],
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

	// Populate differential surface
	p_surface.SetShape(this);
	p_surface.Distance = t;
	p_surface.Point = p_ray.PointAlongRay(t);
	
	p_surface.PointUV.Set (
		v0.UV.U * alpha + v1.UV.U * beta + v2.UV.U * gamma,
		v0.UV.V * alpha + v1.UV.V * beta + v2.UV.V * gamma);

	//p_surface.Basis.InitFromU(Vector3::Cross(BA, CA));
	Vector3::Cross(BA, CA, p_surface.Normal);
	p_surface.Colour = RGBPixel::Green;
	return true;
}
//----------------------------------------------------------------------------------------------
// Specialisation for Vertex
template<>
bool IndexedTriangle<Vertex>::Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
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
	
	p_surface.PointUV.Set (
		v0.UV.U * alpha + v1.UV.U * beta + v2.UV.U * gamma,
		v0.UV.V * alpha + v1.UV.V * beta + v2.UV.V * gamma);

	Vector3::Cross(BA, CA, p_surface.Normal);
	p_surface.Normal.Normalize();
	return true;
}
//----------------------------------------------------------------------------------------------
template<class TVertex>
bool IndexedTriangle<TVertex>::Intersects(const Ray &p_ray, float p_fTime)
{
	const TVertex &v0 = m_pMesh->VertexList[m_nVertexID[0]],
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
template<class TVertex>
IndexedTriangle<TVertex>& IndexedTriangle<TVertex>::operator=(const IndexedTriangle<TVertex>& p_indexedTriangle)
{
	m_nVertexID[0] = p_indexedTriangle.m_nVertexID[0];
	m_nVertexID[1] = p_indexedTriangle.m_nVertexID[1];
	m_nVertexID[2] = p_indexedTriangle.m_nVertexID[2];

	m_pMesh = p_indexedTriangle.m_pMesh;
	m_boundingBox = p_indexedTriangle.m_boundingBox;

	return *this;
}
//----------------------------------------------------------------------------------------------

}
}