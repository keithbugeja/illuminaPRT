//----------------------------------------------------------------------------------------------
//	Filename:	Ray.inl
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
inline Ray Ray::operator-(void) const
{
	return Ray(Origin, -Direction, Min, Max);
}
//----------------------------------------------------------------------------------------------
inline void Ray::Set(const Vector3 &p_origin, const Vector3 &p_direction, float p_fMin, float p_fMax)
{
	Min = p_fMin;
	Max = p_fMax;

	Origin = p_origin;
	Direction = p_direction;

	Vector3::Inverse(Direction, DirectionInverseCache);
}
//----------------------------------------------------------------------------------------------
inline void Ray::SetDirection(const Vector3 &p_direction)
{
	Direction = p_direction;
	Vector3::Inverse(Direction, DirectionInverseCache);
}
//----------------------------------------------------------------------------------------------
inline Vector3 Ray::PointAlongRay(float p_fDistance) const {
	return Origin + p_fDistance * Direction;
}
//----------------------------------------------------------------------------------------------
inline void Ray::PointAlongRay(float p_fDistance, Vector3 &p_out) {
	Vector3::Add(Origin, p_fDistance * Direction, p_out);
}
//----------------------------------------------------------------------------------------------
inline Ray Ray::Apply(const Transformation &p_transformation) const
{
	Ray result;
	Ray::Apply(p_transformation, *this, result);
	return result;
}
//----------------------------------------------------------------------------------------------
inline Ray Ray::ApplyInverse(const Transformation &p_transformation) const 
{
	Ray result;
	Ray::ApplyInverse(p_transformation, *this, result);
	return result;
}
//----------------------------------------------------------------------------------------------
inline Ray& Ray::operator=(const Ray &p_ray)
{
	Origin = p_ray.Origin;
	Direction = p_ray.Direction;
	DirectionInverseCache = p_ray.DirectionInverseCache;
	Min = p_ray.Min;
	Max = p_ray.Max;

	return *this;
}
//----------------------------------------------------------------------------------------------
