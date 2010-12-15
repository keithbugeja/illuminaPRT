//----------------------------------------------------------------------------------------------
//	Filename:	Plane.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
inline Vector3 Plane::GetReflectionVector(const Vector3 &p_vector) const {
	return p_vector - (Normal * 2 * (Normal.Dot(p_vector) - Distance));
}
//----------------------------------------------------------------------------------------------
inline void Plane::GetDisplacementVector(const Vector3 &p_vector, Vector3 &p_out) const {
	p_out = Normal * (Normal.Dot(p_vector) - Distance);
}
//----------------------------------------------------------------------------------------------
inline Vector3 Plane::GetDisplacementVector(const Vector3 &p_vector) const {
	return Normal * (Normal.Dot(p_vector) - Distance);
}
//----------------------------------------------------------------------------------------------
inline void Plane::GetReflectionVector(const Vector3 &p_vector, Vector3 &p_out) const
{
	p_out = Normal * (2.0f * Vector3::Dot(Normal, p_vector) - Distance);
	Vector3::Subtract(p_vector, p_out, p_out);
}
//----------------------------------------------------------------------------------------------
inline float Plane::GetDisplacement(const Vector3& p_vector) const
{
	// s = (V - dN) . N
	//   = V . N - dN . N
	//   = V . N - dN^2

	// |N| = 1
	// N^2 = 1^2 = 1

	// s = V . N - d(1)
	//   = V . N - d

	return Normal.Dot(p_vector) - Distance;
}
//----------------------------------------------------------------------------------------------
inline float Plane::GetDistance(const Vector3 &p_vector) const
{
	return Maths::Abs(Normal.Dot(p_vector) - Distance);
}
//----------------------------------------------------------------------------------------------
inline void Plane::Normalize(void) {
	Normal.Normalize();
}
//----------------------------------------------------------------------------------------------
