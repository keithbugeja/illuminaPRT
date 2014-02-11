//----------------------------------------------------------------------------------------------
//	Filename:	DirectionalLight.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Light/DirectionalLight.h"
#include "Scene/Visibility.h"
#include "Maths/Montecarlo.h"

/*
Spectrum DistantLight::Power(const Scene *scene) const {
	Point worldCenter;
	float worldRadius;
	scene->WorldBound().BoundingSphere(&worldCenter, &worldRadius);
	return L * M_PI * worldRadius * worldRadius;
}

Spectrum DistantLight::Sample_L(const Scene *scene,
		const LightSample &ls, float u1, float u2, float time,
		Ray *ray, Normal *Ns, float *pdf) const {
	// Choose point on disk oriented toward infinite light direction
	Point worldCenter;
	float worldRadius;
	scene->WorldBound().BoundingSphere(&worldCenter, &worldRadius);
	Vector v1, v2;
	CoordinateSystem(lightDir, &v1, &v2);
	float d1, d2;
	ConcentricSampleDisk(ls.uPos[0], ls.uPos[1], &d1, &d2);
	Point Pdisk = worldCenter + worldRadius * (d1 * v1 + d2 * v2);

	// Set ray origin and direction for infinite light ray
	*ray = Ray(Pdisk + worldRadius * lightDir, -lightDir, 0.f, INFINITY,
			   time);
	*Ns = (Normal)ray->d;

	*pdf = 1.f / (M_PI * worldRadius * worldRadius);
	return L;
}
*/

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
DirectionalLight::DirectionalLight(const Vector3 &p_direction, const Spectrum &p_intensity)
	: m_direction(p_direction) 
	, m_intensity(p_intensity)
{ }
//----------------------------------------------------------------------------------------------
DirectionalLight::DirectionalLight(const std::string& p_strName, const Vector3 &p_direction, const Spectrum &p_intensity)
	: ILight(p_strName) 
	, m_direction(p_direction) 
	, m_intensity(p_intensity)
{ }
//----------------------------------------------------------------------------------------------
DirectionalLight::DirectionalLight(const DirectionalLight &p_directionLight)
	: m_direction(p_directionLight.m_direction)
	, m_intensity(p_directionLight.m_intensity)
{ }
//----------------------------------------------------------------------------------------------
float DirectionalLight::Pdf(const Vector3 &p_point, const Vector3 &p_wOut) {
	return 0.0f;
}
//----------------------------------------------------------------------------------------------
Spectrum DirectionalLight::Power(void) {
	return m_intensity * 4.0 * Maths::Pi;
}
//----------------------------------------------------------------------------------------------
Spectrum DirectionalLight::Radiance(const Vector3 &p_lightSurfacePoint, const Vector3 &p_lightSurfaceNormal, const Vector3 &p_wIn)
{
	return Vector3::Dot(p_lightSurfacePoint, p_wIn) > 0 ? m_intensity : 0.0f;
}
//----------------------------------------------------------------------------------------------
Spectrum DirectionalLight::SampleRadiance(const Vector3 &p_surfacePoint, float p_u, float p_v, Vector3 &p_wIn, float &p_pdf, VisibilityQuery &p_visibilityQuery)
{
	// Update visibility query information
	p_visibilityQuery.SetSegment(p_surfacePoint, m_direction, Maths::Maximum, 1e-4f); 

	p_wIn = m_direction;
	p_pdf = 1.0f;

	return m_intensity;
}
//----------------------------------------------------------------------------------------------
Spectrum DirectionalLight::SampleRadiance(const Scene *p_pScene, float p_u, float p_v, float p_w, float p_x, Ray &p_ray, float &p_pdf)
{
	IBoundingVolume *pBoundingVolume = p_pScene->GetSpace()->GetBoundingVolume();
	Vector3 centre = pBoundingVolume->GetCentre();

	//Vector3 normal;
	//p_ray.Direction = Montecarlo::UniformSampleSphere(p_w, p_x);
	//p_ray.Origin = SamplePoint(p_u, p_v, normal, p_pdf) + p_ray.Direction * 1e-3f; normal = -normal;
	
	p_ray.Set(m_direction, Montecarlo::UniformSampleSphere(p_u, p_v));
	p_pdf = Montecarlo::UniformSpherePdf();
	
	return m_intensity;

	// throw new Exception("Cannot sample point light radiance without a reference point!");
}
/*
Spectrum DistantLight::Sample_L(const Scene *scene,
		const LightSample &ls, float u1, float u2, float time,
		Ray *ray, Normal *Ns, float *pdf) const {
	// Choose point on disk oriented toward infinite light direction
	Point worldCenter;
	float worldRadius;
	scene->WorldBound().BoundingSphere(&worldCenter, &worldRadius);
	Vector v1, v2;
	CoordinateSystem(lightDir, &v1, &v2);
	float d1, d2;
	ConcentricSampleDisk(ls.uPos[0], ls.uPos[1], &d1, &d2);
	Point Pdisk = worldCenter + worldRadius * (d1 * v1 + d2 * v2);

	// Set ray origin and direction for infinite light ray
	*ray = Ray(Pdisk + worldRadius * lightDir, -lightDir, 0.f, INFINITY,
			   time);
	*Ns = (Normal)ray->d;

	*pdf = 1.f / (M_PI * worldRadius * worldRadius);
	return L;
}
*/
//----------------------------------------------------------------------------------------------
Vector3 DirectionalLight::SamplePoint(float p_u, float p_v, Vector3 &p_lightSurfaceNormal, float &p_pdf)
{
	p_pdf = 0.5f / Maths::PiTwo;
	p_lightSurfaceNormal = Montecarlo::UniformSampleSphere(p_u, p_v);
	return m_direction;
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
