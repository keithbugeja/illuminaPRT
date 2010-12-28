//----------------------------------------------------------------------------------------------
//	Filename:	SphereUV.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iostream>
#include "Shape/Sphere.h"
#include "Exception/Exception.h"
#include "Maths/Montecarlo.h"
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
		p_surface.ShadingNormal = p_surface.GeometryNormal;

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
	return Maths::InvPiTwo * 0.5f;
}
//----------------------------------------------------------------------------------------------
Vector3 Sphere::SamplePoint(const Vector3 &p_viewPoint, float p_u, float p_v, Vector3 &p_normal)
{
	Vector3 viewToCentre = Centre - p_viewPoint;
	float distanceSquared = viewToCentre.LengthSquared(),
		radiusSquared = Radius * Radius;

	OrthonormalBasis basis; 
	basis.InitFromW(viewToCentre);

	if (distanceSquared - radiusSquared < 1e-4f)
		return SamplePoint(p_u, p_v, p_normal);

	float sinThetaMax2 = radiusSquared / distanceSquared;
	float cosThetaMax = Maths::Sqrt(Maths::Max(0.0f, 1.0f - sinThetaMax2));

	DifferentialSurface surface;	
	Ray ray(p_viewPoint, Montecarlo::UniformSampleCone(p_u, p_v, cosThetaMax, basis), 1e-3f);
	
	if (!Intersects(ray, 0, surface))
		surface.Distance = Vector3::Dot(viewToCentre, Vector3::Normalize(ray.Direction));

	Vector3 surfacePoint = ray.PointAlongRay(surface.Distance);
	p_normal = Vector3::Normalize(surfacePoint - Centre);

	return surfacePoint;
}
//----------------------------------------------------------------------------------------------
Vector3 Sphere::SamplePoint(float p_u, float p_v, Vector3 &p_normal)
{
	p_normal = Montecarlo::UniformSampleSphere(p_u, p_v);
	return p_normal * Radius + Centre + 1;
}
//----------------------------------------------------------------------------------------------
