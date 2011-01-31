//----------------------------------------------------------------------------------------------
//	Filename:	PointLight.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Light/PointLight.h"
#include "Scene/Visibility.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
float PointLight::Pdf(const Vector3 &p_point, const Vector3 &p_wOut) {
	return 1.0f;
}
//----------------------------------------------------------------------------------------------
Spectrum PointLight::Power(void) {
	return Spectrum(1.0f);
}
//----------------------------------------------------------------------------------------------
Spectrum PointLight::Radiance(const Vector3 &p_point, const Vector3 &p_normal, const Vector3 &p_wIn)
{
	return Vector3::Dot(p_normal, p_wIn) > 0 ? m_intensity : 0.0f;
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
Spectrum PointLight::SampleRadiance(const Vector3 &p_point, Vector3 &p_wIn, VisibilityQuery &p_visibilityQuery)
{
	// Update visibility query information
	p_visibilityQuery.SetSegment(m_position, 1e-4f, p_point, 1e-4f); 

	Vector3::Subtract(p_point, m_position, p_wIn);
	double distanceSquared = p_wIn.LengthSquared();
	p_wIn.Normalize();

	// Radiance prop to Energy / (Area of sphere * distance squared)
	// L = Phi / (4*Pi * |Sp - x| ^ 2)
	return m_intensity / (Maths::InvPi * distanceSquared);
}
//----------------------------------------------------------------------------------------------
Spectrum PointLight::SampleRadiance(const Vector3 &p_point, double p_u, double p_v, Vector3& p_wIn, VisibilityQuery &p_visibilityQuery)
{
	return SampleRadiance(p_point, p_wIn, p_visibilityQuery);
}
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
