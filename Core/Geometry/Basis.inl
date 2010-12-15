//----------------------------------------------------------------------------------------------
//	Filename:	Basis.inl
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
inline Vector2 OrthonormalBasis::ToSpherical(const Vector3 &p_point) {
	return Vector2(Maths::Atan(p_point.Y, p_point.X), Maths::Acos(p_point.Z));
}
//----------------------------------------------------------------------------------------------
inline Vector2 OrthonormalBasis::ToSpherical(const Vector3 &p_point, const Vector3 &p_centre)
{
	Vector3 direction = Vector3::Normalize(p_point - p_centre);
	return OrthonormalBasis::ToSpherical(direction);
}
//----------------------------------------------------------------------------------------------
inline Vector3 OrthonormalBasis::FromSpherical(const Vector2 &p_omega)
{
	float sinTheta = Maths::Sin(p_omega.X);
	return Vector3(Maths::Cos(p_omega.Y) * sinTheta,
		Maths::Sin(p_omega.Y) * sinTheta,
		Maths::Cos(p_omega.X));
}
//----------------------------------------------------------------------------------------------
inline Vector3 OrthonormalBasis::FromSpherical(const Vector2 &p_omega, const Vector3 &p_centre, float p_fRadius) {
	return p_centre + p_fRadius * FromSpherical(p_omega);
}
//----------------------------------------------------------------------------------------------
