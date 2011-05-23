//----------------------------------------------------------------------------------------------
//	Filename:	PointLight.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Light/InfiniteAreaLight.h"
#include "Maths/Montecarlo.h"
#include "Scene/Visibility.h"
#include "Texture/Texture.h"
#include "Geometry/Basis.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
InfiniteAreaLight::InfiniteAreaLight(const Spectrum &p_intensity, ITexture *p_pTexture)
	: m_pTexture(p_pTexture) 
	, m_intensity(p_intensity)
{ }
//----------------------------------------------------------------------------------------------
InfiniteAreaLight::InfiniteAreaLight(const std::string& p_strName, const Spectrum &p_intensity, ITexture *p_pTexture)
	: ILight(p_strName) 
	, m_pTexture(p_pTexture) 
	, m_intensity(p_intensity)
{ }
//----------------------------------------------------------------------------------------------
InfiniteAreaLight::InfiniteAreaLight(const InfiniteAreaLight &p_infiniteAreaLight)
	: m_pTexture(p_infiniteAreaLight.m_pTexture)
	, m_intensity(p_infiniteAreaLight.m_intensity)
{ }
//----------------------------------------------------------------------------------------------
float InfiniteAreaLight::Pdf(const Vector3 &p_point, const Vector3 &p_wOut) {
	return 0.25f / Maths::Pi;
}
//----------------------------------------------------------------------------------------------
Spectrum InfiniteAreaLight::Power(void) {
	return Spectrum(1.0f);
}
//----------------------------------------------------------------------------------------------
Spectrum InfiniteAreaLight::Radiance(const Vector3 &p_direction)
{
	Spectrum radiance(1);
	
	if (m_pTexture != NULL)
	{
		Vector2 uvCoords; 

		uvCoords.U = 1.0f - (Maths::Pi + Maths::Atan(p_direction.Z, p_direction.X)) / Maths::PiTwo;
		uvCoords.V = 1.0f - 0.5f * (p_direction.Y + 1);

		RGBPixel value = m_pTexture->GetValue(uvCoords);
		radiance.Set(value.R, value.G, value.B);
	}

	return radiance * m_intensity;
}
//----------------------------------------------------------------------------------------------
Spectrum InfiniteAreaLight::Radiance(const Ray &p_ray)
{
	return Radiance(p_ray.Direction);
}
//----------------------------------------------------------------------------------------------
Spectrum InfiniteAreaLight::Radiance(const Vector3 &p_lightSurfacePoint, const Vector3 &p_lightSurfaceNormal, const Vector3 &p_wIn)
{
	return Vector3::Dot(p_lightSurfaceNormal, p_wIn) > 0 ? Radiance(p_wIn) : 0.0f;
}
//----------------------------------------------------------------------------------------------
Spectrum InfiniteAreaLight::SampleRadiance(const Vector3 &p_surfacePoint, float p_u, float p_v, Vector3 &p_wIn, float &p_pdf, VisibilityQuery &p_visibilityQuery)
{
	throw new Exception("Method not supported!");
}
//----------------------------------------------------------------------------------------------
Spectrum InfiniteAreaLight::SampleRadiance(const Vector3 &p_surfacePoint, const Vector3 &p_surfaceNormal, float p_u, float p_v, Vector3 &p_wIn, float &p_pdf, VisibilityQuery &p_visibilityQuery)
{
	OrthonormalBasis basis; basis.InitFromW(p_surfaceNormal);
	p_wIn = Montecarlo::UniformSampleCone(p_u, p_v, 1e-3f, basis);
	p_visibilityQuery.SetSegment(p_surfacePoint, p_wIn, Maths::Maximum, 1e-4f);
	p_wIn = -p_wIn;
	
	p_pdf = 0.5f / Maths::PiTwo;
	
	return Radiance(p_wIn);
}
//----------------------------------------------------------------------------------------------
Spectrum InfiniteAreaLight::SampleRadiance(const Scene *p_pScene, float p_u, float p_v, float p_w, float p_x, Ray &p_ray, float &p_pdf)
{
	p_pdf = 0.5f / Maths::PiTwo;

	p_ray.Direction = -Montecarlo::UniformSampleSphere(p_u, p_v);
	p_ray.Origin = p_ray.Direction * -1e+38f;
	p_ray.Min = 1e-03;
	p_ray.Max = Maths::Maximum;

	return Radiance(p_ray.Direction);
}
//----------------------------------------------------------------------------------------------
Vector3 InfiniteAreaLight::SamplePoint(const Vector3 &p_viewPoint, float p_u, float p_v, Vector3 &p_lightSurfaceNormal, float &p_pdf)
{
	return SamplePoint(p_u, p_v, p_lightSurfaceNormal, p_pdf);
}
//----------------------------------------------------------------------------------------------
Vector3 InfiniteAreaLight::SamplePoint(float p_u, float p_v, Vector3 &p_lightSurfaceNormal, float &p_pdf)
{
	p_pdf = 0.5f / Maths::PiTwo;

	p_lightSurfaceNormal = -Montecarlo::UniformSampleSphere(p_u, p_v);

	return p_lightSurfaceNormal * -1e+38; // Arbitrarily large
}
//----------------------------------------------------------------------------------------------
ITexture* InfiniteAreaLight::GetTexture(void) const { 
	return m_pTexture; 
}
//----------------------------------------------------------------------------------------------
void InfiniteAreaLight::SetTexture(ITexture *p_pTexture) { 
	m_pTexture = p_pTexture; 
}
//----------------------------------------------------------------------------------------------
Spectrum InfiniteAreaLight::GetIntensity(void) const { 
	return m_intensity; 
}
//----------------------------------------------------------------------------------------------
void InfiniteAreaLight::SetIntensity(const Spectrum &p_intensity) { 
	m_intensity = p_intensity; 
}
//----------------------------------------------------------------------------------------------