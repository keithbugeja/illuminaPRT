//----------------------------------------------------------------------------------------------
//	Filename:	IndexedTriangle.inl
//	Author:		Keith Bugeja
//	Date:		22/03/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core 
	{
//----------------------------------------------------------------------------------------------
template<class TVertex> 
IndexedTriangle<TVertex>::IndexedTriangle(const ITriangleMesh<IndexedTriangle, TVertex> *p_pMesh, 
	int p_nV1, int p_nV2, int p_nV3, int p_nGroupId)
{
	m_pMesh = (ITriangleMesh<IndexedTriangle, TVertex>*)p_pMesh;
	
	m_nVertexID[0] = p_nV1;
	m_nVertexID[1] = p_nV2;
	m_nVertexID[2] = p_nV3;

	m_nGroupID = p_nGroupId;
}
//----------------------------------------------------------------------------------------------
template<class TVertex> 
IndexedTriangle<TVertex>::IndexedTriangle(const IndexedTriangle &p_triangle)
{
	m_pMesh = p_triangle.m_pMesh;

	m_nVertexID[0] = p_triangle.m_nVertexID[0];
	m_nVertexID[1] = p_triangle.m_nVertexID[1];
	m_nVertexID[2] = p_triangle.m_nVertexID[2];

	m_nGroupID = p_triangle.m_nGroupID;
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
bool IndexedTriangle<TVertex>::HasGroup(void) const { 
	return m_nGroupID >= 0;
}
//----------------------------------------------------------------------------------------------
template<class TVertex>
int IndexedTriangle<TVertex>::GetGroupId(void) const {
	return m_nGroupID;
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
	
	// Set 2d parametric surface representation
	p_surface.PointUV.Set (
		v0.UV.U * alpha + v1.UV.U * beta + v2.UV.U * gamma,
		v0.UV.V * alpha + v1.UV.V * beta + v2.UV.V * gamma);

	// Set shading normal
	p_surface.ShadingNormal.Set(v0.Normal.X * alpha + v1.Normal.X * beta + v2.Normal.X * gamma,
		v0.Normal.Y * alpha + v1.Normal.Y * beta + v2.Normal.Y * gamma,
		v0.Normal.Z * alpha + v1.Normal.Z * beta + v2.Normal.Z * gamma);
	
	p_surface.ShadingNormal.Normalize();

	// Set geometry normal
	Vector3::Cross(BA, CA, p_surface.GeometryNormal);
	p_surface.GeometryNormal.Normalize();

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
float IndexedTriangle<TVertex>::GetArea(void) const
{
	const TVertex &v0 = m_pMesh->VertexList[m_nVertexID[0]],
		&v1 = m_pMesh->VertexList[m_nVertexID[1]],
		&v2 = m_pMesh->VertexList[m_nVertexID[2]];

	return Vector3::Cross(v1.Position - v0.Position, v2.Position - v0.Position).Length() * 0.5;
}
//----------------------------------------------------------------------------------------------
template<class TVertex>
float IndexedTriangle<TVertex>::GetPdf(const Vector3 &p_point) const
{
	return 1.0f / GetArea();
}
//----------------------------------------------------------------------------------------------
template<class TVertex>
Vector3 IndexedTriangle<TVertex>::SamplePoint(float p_u, float p_v, Vector3 &p_normal)
{
	float b1, b2;
	Montecarlo::UniformSampleTriangle(p_u, p_v, &b1, &b2);
	
	// Get triangle vertices in _p1_, _p2_, and _p3_
	const Vector3 &p1 = m_pMesh->VertexList[m_nVertexID[0]].Position;
	const Vector3 &p2 = m_pMesh->VertexList[m_nVertexID[1]].Position;
	const Vector3 &p3 = m_pMesh->VertexList[m_nVertexID[2]].Position;

	Vector3 p = b1 * p1 + b2 * p2 + (1.f - b1 - b2) * p3;
	Vector3 n = Vector3::Cross(p2-p1, p3-p1);
	p_normal = Vector3::Normalize(n);
	return p;

	//const TVertex &v0 = m_pMesh->VertexList[m_nVertexID[0]],
	//	&v1 = m_pMesh->VertexList[m_nVertexID[1]],
	//	&v2 = m_pMesh->VertexList[m_nVertexID[2]];

	//float temp = Maths::Sqrt(1.0f - p_u);
	//float beta = 1.0f - temp;
	//float gamma = temp * p_v;

	//p_normal = Vector3::Cross(v1.Position - v0.Position, v2.Position - v0.Position);
	//return (1.0f - beta - gamma) * v0.Position + beta * v1.Position + gamma * v2.Position;
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