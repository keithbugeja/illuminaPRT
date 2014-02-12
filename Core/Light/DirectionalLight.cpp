//----------------------------------------------------------------------------------------------
//	Filename:	DirectionalLight.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Scene/Scene.h"
#include "Scene/Visibility.h"
#include "Maths/Montecarlo.h"
#include "Light/DirectionalLight.h"
#include "Geometry/BoundingVolume.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
DirectionalLight::DirectionalLight(float p_fDistance, const Vector3 &p_direction, const Spectrum &p_intensity)
	: m_fDistance(p_fDistance)
	, m_direction(p_direction) 
	, m_intensity(p_intensity)
{ }
//----------------------------------------------------------------------------------------------
DirectionalLight::DirectionalLight(const std::string &p_strName, float p_fDistance, const Vector3 &p_direction, const Spectrum &p_intensity)
	: ILight(p_strName) 
	, m_fDistance(p_fDistance)
	, m_direction(p_direction) 
	, m_intensity(p_intensity)
{ }
//----------------------------------------------------------------------------------------------
DirectionalLight::DirectionalLight(const DirectionalLight &p_directionLight)
	: m_fDistance(p_directionLight.m_fDistance)
	, m_direction(p_directionLight.m_direction)
	, m_intensity(p_directionLight.m_intensity)
{ }
//----------------------------------------------------------------------------------------------
float DirectionalLight::Pdf(const Vector3 &p_point, const Vector3 &p_wOut) {
	return 0.0f;
}
//----------------------------------------------------------------------------------------------
Spectrum DirectionalLight::Power(void) 
{
	// Need scene to compute boundary-based power in the system
	return m_intensity;
}
//----------------------------------------------------------------------------------------------
Spectrum DirectionalLight::Radiance(const Vector3 &p_lightSurfacePoint, const Vector3 &p_lightSurfaceNormal, const Vector3 &p_wIn)
{
	//return m_intensity;
	return Vector3::Dot(p_lightSurfacePoint, p_wIn) > 0 ? m_intensity : 0.0f;
}
//----------------------------------------------------------------------------------------------
Spectrum DirectionalLight::SampleRadiance(const Vector3 &p_surfacePoint, float p_u, float p_v, Vector3 &p_wIn, float &p_pdf, VisibilityQuery &p_visibilityQuery)
{
	// Update visibility query information
	p_visibilityQuery.SetSegment(p_surfacePoint - (m_fDistance * m_direction), 1e-3f, p_surfacePoint, 1e-3f);

	p_wIn = m_direction;
	p_pdf = 1.0f;

	return m_intensity;
}
//----------------------------------------------------------------------------------------------
Spectrum DirectionalLight::SampleRadiance(const Scene *p_pScene, float p_u, float p_v, float p_w, float p_x, Ray &p_ray, float &p_pdf)
{
	IBoundingVolume *pBoundingVolume = p_pScene->GetSpace()->GetBoundingVolume();
	Vector3 worldCentre = pBoundingVolume->GetCentre();
	float worldRadius = pBoundingVolume->GetRadius();

	OrthonormalBasis basis;
	basis.InitFromW(m_direction);
	Vector2 d = Montecarlo::ConcentricSampleDisk(p_u, p_v);
	Vector3 p = worldCentre + worldRadius * (d.X * basis.U + d.Y * basis.V);
	
	p_ray.Set(p + worldRadius * m_direction, -m_direction);
	p_pdf = 1.f / (Maths::Pi * worldRadius * worldRadius);

	// std::cout << p_ray.ToString() << std::endl;

	return m_intensity;
}
//----------------------------------------------------------------------------------------------
Vector3 DirectionalLight::SamplePoint(float p_u, float p_v, Vector3 &p_lightSurfaceNormal, float &p_pdf)
{
	return Vector3::Zero;
}
//----------------------------------------------------------------------------------------------
float DirectionalLight::GetDistance(void) const {
	return m_fDistance;
}
//----------------------------------------------------------------------------------------------
void DirectionalLight::SetDistance(float p_fDistance) {
	m_fDistance = p_fDistance;
}
//----------------------------------------------------------------------------------------------
Vector3 DirectionalLight::GetDirection(void) const { 
	return m_direction; 
}
//----------------------------------------------------------------------------------------------
void DirectionalLight::SetDirection(const Vector3 &p_direction) { 
	m_direction = p_direction; 
}
//----------------------------------------------------------------------------------------------
Spectrum DirectionalLight::GetIntensity(void) const { 
	return m_intensity; 
}
//----------------------------------------------------------------------------------------------
void DirectionalLight::SetIntensity(const Spectrum &p_intensity) { 
	m_intensity = p_intensity; 
}
//----------------------------------------------------------------------------------------------
