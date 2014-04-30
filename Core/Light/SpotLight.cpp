//----------------------------------------------------------------------------------------------
//	Filename:	SpotLight.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Light/SpotLight.h"
#include "Scene/Visibility.h"
#include "Maths/Montecarlo.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
SpotLight::SpotLight(const Vector3 &p_position, const Vector3 &p_direction, const Spectrum &p_intensity, float p_fWidth, float p_fFall)
	: m_position(p_position) 
	, m_direction(p_direction)
	, m_intensity(p_intensity)
	, m_fCosTotalWidth(Maths::Cos(Maths::DegToRad(p_fWidth)))
	, m_fCosFalloffStart(Maths::Cos(Maths::DegToRad(p_fFall)))
{ 
	m_basis.InitFromW(p_direction);
}
//----------------------------------------------------------------------------------------------
SpotLight::SpotLight(const std::string& p_strName, const Vector3 &p_position, const Vector3 &p_direction, const Spectrum &p_intensity, float p_fWidth, float p_fFall)
	: ILight(p_strName) 
	, m_position(p_position) 
	, m_direction(p_direction)
	, m_intensity(p_intensity)
	, m_fCosTotalWidth(Maths::Cos(Maths::DegToRad(p_fWidth)))
	, m_fCosFalloffStart(Maths::Cos(Maths::DegToRad(p_fFall)))
{ 
	m_basis.InitFromW(p_direction);
}
//----------------------------------------------------------------------------------------------
SpotLight::SpotLight(const SpotLight &p_spotLight)
	: m_position(p_spotLight.m_position)
	, m_direction(p_spotLight.m_direction) 
	, m_basis(p_spotLight.m_basis)
	, m_intensity(p_spotLight.m_intensity)
	, m_fCosTotalWidth(p_spotLight.m_fCosTotalWidth)
	, m_fCosFalloffStart(p_spotLight.m_fCosFalloffStart)
{ }
//----------------------------------------------------------------------------------------------
float SpotLight::Falloff(const Vector3 &p_direction) const
{
	float cosTheta = Vector3::Dot(p_direction, m_direction);
	if (cosTheta < m_fCosTotalWidth) return 0.f;
	if (cosTheta > m_fCosFalloffStart) return 1.0f;
	float delta = (cosTheta - m_fCosTotalWidth) / (m_fCosFalloffStart - m_fCosTotalWidth);
	return delta * delta * delta * delta;
}
//----------------------------------------------------------------------------------------------
float SpotLight::Pdf(const Vector3 &p_point, const Vector3 &p_wOut) {
	return 0.0f;
}
//----------------------------------------------------------------------------------------------
Spectrum SpotLight::Power(void) {
	return m_intensity * 2.0 * Maths::Pi * (1.f - 0.5 * (m_fCosFalloffStart + m_fCosTotalWidth));
}
//----------------------------------------------------------------------------------------------
Spectrum SpotLight::Radiance(const Vector3 &p_surfacePoint, const Vector3 &p_surfaceNormal, const Vector3 &p_wIn)
{
	return Vector3::Dot(p_surfaceNormal, p_wIn) > 0 ? m_intensity : 0.0f;
}
//----------------------------------------------------------------------------------------------
Spectrum SpotLight::SampleRadiance(const Vector3 &p_surfacePoint, float p_u, float p_v, Vector3 &p_wIn, float &p_pdf, VisibilityQuery &p_visibilityQuery)
{
	// Update visibility query information
	p_visibilityQuery.SetSegment(m_position, 1e-4f, p_surfacePoint, 1e-4f); 

	Vector3::Subtract(p_surfacePoint, m_position, p_wIn);
	float distanceSquared = p_wIn.LengthSquared();
	p_wIn.Normalize();
	p_pdf = 1.0f;

	return m_intensity * Falloff(p_wIn) / distanceSquared;
}
//----------------------------------------------------------------------------------------------
Spectrum SpotLight::SampleRadiance(const Scene *p_pScene, float p_u, float p_v, float p_w, float p_x, Ray &p_ray, float &p_pdf)
{
	p_ray.Set(m_position, m_basis.Project(Montecarlo::UniformSampleCone(p_u, p_v, m_fCosTotalWidth)));
	p_pdf = Montecarlo::UniformConePdf(m_fCosTotalWidth);

	return m_intensity * Falloff(p_ray.Direction);
}
//----------------------------------------------------------------------------------------------
Vector3 SpotLight::SamplePoint(float p_u, float p_v, Vector3 &p_lightSurfaceNormal, float &p_pdf)
{
	p_pdf = 0.5f / Maths::PiTwo;
	p_lightSurfaceNormal = Montecarlo::UniformSampleSphere(p_u, p_v);
	return m_position;
}
//----------------------------------------------------------------------------------------------
Vector3 SpotLight::GetPosition(void) const { 
	return m_position; 
}
//----------------------------------------------------------------------------------------------
void SpotLight::SetPosition(const Vector3 &p_position) { 
	m_position = p_position; 
}
//----------------------------------------------------------------------------------------------
Vector3 SpotLight::GetDirection(void) const { 
	return m_direction; 
}
//----------------------------------------------------------------------------------------------
void SpotLight::SetDirection(const Vector3 &p_direction) 
{ 
	m_direction = p_direction; 
	m_basis.InitFromW(p_direction);
}
//----------------------------------------------------------------------------------------------
Spectrum SpotLight::GetIntensity(void) const { 
	return m_intensity; 
}
//----------------------------------------------------------------------------------------------
void SpotLight::SetIntensity(const Spectrum &p_intensity) { 
	m_intensity = p_intensity; 
}
//----------------------------------------------------------------------------------------------
