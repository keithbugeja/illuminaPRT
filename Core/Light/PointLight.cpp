//----------------------------------------------------------------------------------------------
//	Filename:	PointLight.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Light/PointLight.h"
#include "Scene/Visibility.h"
#include "Maths/Montecarlo.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
PointLight::PointLight(const Vector3 &p_position, const Spectrum &p_intensity)
	: m_position(p_position) 
	, m_intensity(p_intensity)
{ }
//----------------------------------------------------------------------------------------------
PointLight::PointLight(const std::string& p_strName, const Vector3 &p_position, const Spectrum &p_intensity)
	: ILight(p_strName) 
	, m_position(p_position) 
	, m_intensity(p_intensity)
{ }
//----------------------------------------------------------------------------------------------
PointLight::PointLight(const PointLight &p_pointLight)
	: m_position(p_pointLight.m_position)
	, m_intensity(p_pointLight.m_intensity)
{ }
//----------------------------------------------------------------------------------------------
float PointLight::Pdf(const Vector3 &p_point, const Vector3 &p_wOut) {
	return 0.0f;
}
//----------------------------------------------------------------------------------------------
Spectrum PointLight::Power(void) {
	return m_intensity * 4.0 * Maths::Pi;
}
//----------------------------------------------------------------------------------------------
Spectrum PointLight::Radiance(const Vector3 &p_lightSurfacePoint, const Vector3 &p_lightSurfaceNormal, const Vector3 &p_wIn)
{
	return Vector3::Dot(p_lightSurfacePoint, p_wIn) > 0 ? m_intensity : 0.0f;
}
//----------------------------------------------------------------------------------------------
/*
	Returns the Radiance L(i, x), where
	i is the incident vector denoting incoming radiance at x,
	and x is the location of a differential area on a surface.
			 
	The visibility query object, denoted by p_visibilityQuery, is set to the segment
	(Sp, x), where Sp is the location of the point light.
	The direction of incident light, denoted by p_direction, is set to (x - Sp).
*/
//----------------------------------------------------------------------------------------------
Spectrum PointLight::SampleRadiance(const Vector3 &p_surfacePoint, float p_u, float p_v, Vector3 &p_wIn, float &p_pdf, VisibilityQuery &p_visibilityQuery)
{
	// Update visibility query information
	p_visibilityQuery.SetSegment(m_position, 1e-4f, p_surfacePoint, 1e-4f); 

	Vector3::Subtract(p_surfacePoint, m_position, p_wIn);
	double distanceSquared = p_wIn.LengthSquared();
	p_wIn.Normalize();
	p_pdf = 1.0f;

	// Radiance prop to Energy / (Area of sphere * distance squared)
	// L = Phi / (4*Pi * |Sp - x| ^ 2)
	return m_intensity / distanceSquared;
}
//----------------------------------------------------------------------------------------------
Spectrum PointLight::SampleRadiance(const Scene *p_pScene, float p_u, float p_v, float p_w, float p_x, Ray &p_ray, float &p_pdf)
{
	Vector3 normal;

	p_ray.Direction = Montecarlo::UniformSampleSphere(p_w, p_x);
	p_ray.Origin = SamplePoint(p_u, p_v, normal, p_pdf) + p_ray.Direction * 1e-3f; normal = -normal;

	p_ray.Min = 1e-3f;
	p_ray.Max = Maths::Maximum;

	if (Vector3::Dot(p_ray.Direction, normal) < 0) 
		p_ray.Direction *= -1.0f;

	Vector3::Inverse(p_ray.Direction, p_ray.DirectionInverseCache);

	p_pdf = Montecarlo::UniformSpherePdf();
	
	return m_intensity;

	// throw new Exception("Cannot sample point light radiance without a reference point!");
}
//----------------------------------------------------------------------------------------------
Vector3 PointLight::SamplePoint(float p_u, float p_v, Vector3 &p_lightSurfaceNormal, float &p_pdf)
{
	p_pdf = 0.5f / Maths::PiTwo;
	p_lightSurfaceNormal = Montecarlo::UniformSampleSphere(p_u, p_v);
	return m_position;
}
//----------------------------------------------------------------------------------------------
Vector3 PointLight::GetPosition(void) const { 
	return m_position; 
}
//----------------------------------------------------------------------------------------------
void PointLight::SetPosition(const Vector3 &p_position) { 
	m_position = p_position; 
}
//----------------------------------------------------------------------------------------------
Spectrum PointLight::GetIntensity(void) const { 
	return m_intensity; 
}
//----------------------------------------------------------------------------------------------
void PointLight::SetIntensity(const Spectrum &p_intensity) { 
	m_intensity = p_intensity; 
}
//----------------------------------------------------------------------------------------------
