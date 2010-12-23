//----------------------------------------------------------------------------------------------
//	Filename:	SphereUV.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Shape/Sphere.h"
#include "Exception/Exception.h"
#include "Maths/Random.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
Sphere::Sphere(const Vector3 &p_centre, float p_fRadius)
	: Centre(p_centre)
	, Radius(p_fRadius)
{}
//----------------------------------------------------------------------------------------------
Sphere::Sphere(const Sphere &p_sphere)
	: Centre(p_sphere.Centre)
	, Radius(p_sphere.Radius)
{}
//----------------------------------------------------------------------------------------------
bool Sphere::IsBounded(void) const {
	return true;
}
//----------------------------------------------------------------------------------------------
void Sphere::ComputeBoundingVolume(void)
{
	Vector3 extent(Radius);

	m_boundingBox.SetExtents(Centre - extent, Centre + extent);
}
//----------------------------------------------------------------------------------------------
IBoundingVolume* Sphere::GetBoundingVolume(void) const {
	return (IBoundingVolume*)&m_boundingBox;
}
//----------------------------------------------------------------------------------------------
bool Sphere::Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
{
	const Vector3 &temp = p_ray.Origin - Centre;
	
	double a = Vector3::Dot(p_ray.Direction, p_ray.Direction);
	double b = 2 * Vector3::Dot(p_ray.Direction, temp);
	double c = Vector3::Dot(temp, temp) - Radius * Radius;

	double discriminant = b*b - 4*a*c;
	
	if (discriminant > 0)
	{
		discriminant = Maths::Sqrt((float)discriminant);
		double t = (-b - discriminant) / (2 * a);
		
		if (t < p_ray.Min)
			t = (-b + discriminant) / (2 * a);

		if (t < p_ray.Min || t > p_ray.Max)
			return false;


		p_surface.SetShape((IShape*)this);
		p_surface.Distance = (float)t;
		p_surface.Point = p_ray.PointAlongRay((float)t);
		Vector3::Subtract(p_surface.Point, Centre, p_surface.GeometryNormal);

		// TODO: Make texture wrapping axis to revolve about +Y
		p_surface.PointUV = OrthonormalBasis::ToSpherical(p_surface.Point, Centre);
		if (p_surface.PointUV.X < 0.0f) p_surface.PointUV.X += Maths::PiTwo;
		p_surface.PointUV.Set((Maths::PiTwo - p_surface.PointUV.X) / Maths::PiTwo, p_surface.PointUV.Y / Maths::PiHalf);

		return true;
	}

	return false;
}
//----------------------------------------------------------------------------------------------
bool Sphere::Intersects(const Ray &p_ray, float p_fTime)
{
	const Vector3 &temp = p_ray.Origin - Centre;
	
	double a = Vector3::Dot(p_ray.Direction, p_ray.Direction);
	double b = 2 * Vector3::Dot(p_ray.Direction, temp);
	double c = Vector3::Dot(temp, temp) - Radius * Radius;

	double discriminant = b*b - 4*a*c;
	
	if (discriminant > 0)
	{
		discriminant = Maths::Sqrt((float)discriminant);
		double t = (-b - discriminant) / (2 * a);

		if (t < p_ray.Min)
			t = (-b + discriminant) / (2 * a);
		
		if (t < p_ray.Min || t > p_ray.Max)
			return false;

		return true;
	}

	return false;
}
//----------------------------------------------------------------------------------------------
float Sphere::GetArea(void) const
{
	return Maths::Pi * 4 * Radius * Radius;
}
//----------------------------------------------------------------------------------------------
float Sphere::GetPdf(const Vector3 &p_point) const
{
	return 1.0f / GetArea();
}
//----------------------------------------------------------------------------------------------
Vector3 Sphere::SamplePoint(const Vector3 &p_viewPoint, float p_u, float p_v, Vector3 &p_normal) const
{
	Vector3 wIn = p_viewPoint - Centre;
	float distance = wIn.Length();

	if (distance < Radius)
		return Vector3::Zero;

	float sin_alpha_max = Radius / distance;
	float cos_alpha_max = (1.0f - sin_alpha_max * sin_alpha_max);
	float q = 1.0f / (Maths::PiTwo * (1.0f - cos_alpha_max));

	float cos_alpha = 1.0 + p_u * (cos_alpha_max - 1.0f);
	float sin_alpha = Maths::Sqrt(1.0 - cos_alpha * cos_alpha);

	float phi = Maths::PiTwo * p_v;
	float cos_phi = Maths::Cos(phi);
	float sin_phi = Maths::Sin(phi);

	Vector3 surfacePoint(cos_phi * sin_alpha, sin_alpha * cos_alpha, cos_alpha);
	OrthonormalBasis basis; 
	wIn.Normalize(); basis.InitFromV(wIn);
	surfacePoint = basis.Project(surfacePoint);

	p_normal = wIn;

	return Centre + surfacePoint * (Radius + 0.01f);
}
//----------------------------------------------------------------------------------------------
Vector3 Sphere::SamplePoint(float p_u, float p_v, Vector3 &p_normal) const
{
	throw new Exception("Use point sampling function that specifies a view point!");
}
//----------------------------------------------------------------------------------------------
