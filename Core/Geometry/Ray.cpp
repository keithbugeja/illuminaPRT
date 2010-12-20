//----------------------------------------------------------------------------------------------
//	Filename:	Ray.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Geometry/Ray.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
Ray::Ray(void) 
{ }
//----------------------------------------------------------------------------------------------
Ray::Ray(const Vector3 &p_origin, const Vector3 &p_direction)
	: Origin(p_origin)
	, Direction(p_direction)
	, Min(0.0f)
	, Max(Maths::Maximum) 
{ }
//----------------------------------------------------------------------------------------------
Ray::Ray(const Vector3 &p_origin, const Vector3 &p_direction, float p_fMin, float p_fMax)
	: Origin(p_origin)
	, Direction(p_direction)
	, Min(p_fMin)
	, Max(p_fMax) 
{ }
//----------------------------------------------------------------------------------------------
Ray::Ray(const Ray &p_ray) {
	*this = p_ray;
}
//----------------------------------------------------------------------------------------------
void Ray::Apply(const Transformation &p_transformation, const Ray &p_ray, Ray &p_out)
{
	p_transformation.Rotate(p_ray.Direction, p_out.Direction);
	p_transformation.Scale(p_out.Direction, p_out.Direction);
	p_transformation.Apply(p_ray.Origin, p_out.Origin);

	p_out.Min = p_ray.Min;
	p_out.Max = p_ray.Max;
}
//----------------------------------------------------------------------------------------------
void Ray::ApplyInverse(const Transformation &p_transformation, const Ray &p_ray, Ray &p_out)
{
	p_transformation.RotateInverse(p_ray.Direction, p_out.Direction);
	p_transformation.ScaleInverse(p_out.Direction, p_out.Direction);
	p_transformation.ApplyInverse(p_ray.Origin, p_out.Origin);

	p_out.Min = p_ray.Min;
	p_out.Max = p_ray.Max;
}
//----------------------------------------------------------------------------------------------
std::string Ray::ToString(void) const
{
	std::string strOut = boost::str(boost::format("[O:<%1%> D:<%2%> <%d-%d>]") 
		% Origin.ToString() 
		% Direction.ToString() 
		% Min % Max);
	return strOut;
}
//----------------------------------------------------------------------------------------------