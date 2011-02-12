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

	/* return Spectrum( 
		Maths::Pow(m_intensity[0] * radiance[0], 3),
		Maths::Pow(m_intensity[1] * radiance[1], 3),
		Maths::Pow(m_intensity[2] * radiance[2], 3));
	
	return (m_intensity * radiance); // * radiance * radiance;
	*/
}
//----------------------------------------------------------------------------------------------
Spectrum InfiniteAreaLight::Radiance(const Ray &p_ray)
{
	return Radiance(p_ray.Direction);
}
//----------------------------------------------------------------------------------------------
Spectrum InfiniteAreaLight::Radiance(const Vector3 &p_point, const Vector3 &p_normal, const Vector3 &p_wIn)
{
	return Vector3::Dot(p_normal, p_wIn) > 0 ? Radiance(p_wIn) : 0.0f;
}
//----------------------------------------------------------------------------------------------
Spectrum InfiniteAreaLight::SampleRadiance(const Vector3 &p_point, Vector3 &p_wIn, VisibilityQuery &p_visibilityQuery)
{
	throw new Exception("Method not supported!");
}
//----------------------------------------------------------------------------------------------
Spectrum InfiniteAreaLight::SampleRadiance(const Vector3 &p_point, double p_u, double p_v, Vector3& p_wIn, VisibilityQuery &p_visibilityQuery)
{
	throw new Exception("Method not supported!");
}
//----------------------------------------------------------------------------------------------
Spectrum InfiniteAreaLight::SampleRadiance(const Vector3 &p_point, const Vector3 &p_normal, double p_u, double p_v, Vector3& p_wIn, VisibilityQuery &p_visibilityQuery)
{
	OrthonormalBasis basis; basis.InitFromW(p_normal);
	p_wIn = Montecarlo::UniformSampleCone(p_u, p_v, 1e-3f, basis);
	p_visibilityQuery.SetSegment(p_point, p_wIn, Maths::Maximum, 1e-4f);
	p_wIn = -p_wIn;
	return Radiance(p_wIn);
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
