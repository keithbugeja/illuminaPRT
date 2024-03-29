//----------------------------------------------------------------------------------------------
//	Filename:	PointLight.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Light/Light.h"

#include "Spectrum/Spectrum.h"
#include "Geometry/Vector3.h"
#include "Texture/Texture.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class InfiniteAreaLight : public ILight
		{
		public:
			InfiniteAreaLight(const InfiniteAreaLight &p_infiniteAreaLight);
			InfiniteAreaLight(const Spectrum &p_intensity, ITexture *p_pTexture = NULL);
			InfiniteAreaLight(const std::string& p_strName, const Spectrum &p_intensity, ITexture *p_pTexture = NULL);

			float Pdf(const Vector3 &p_point, const Vector3 &p_wOut);

			Spectrum Power(void);

			Spectrum Radiance(const Ray &p_ray);
			Spectrum Radiance(const Vector3 &p_lightSurfacePoint, const Vector3 &p_lightSurfaceNormal, const Vector3 &p_wIn);

			Spectrum SampleRadiance(const Vector3 &p_surfacePoint, float p_u, float p_v, Vector3 &p_wIn, float &p_pdf, VisibilityQuery &p_visibilityQuery);
			Spectrum SampleRadiance(const Vector3 &p_surfacePoint, const Vector3 &p_surfaceNormal, float p_u, float p_v, Vector3 &p_wIn, float &p_pdf, VisibilityQuery &p_visibilityQuery);
			Spectrum SampleRadiance(const Scene *p_pScene, float p_u, float p_v, float p_w, float p_x, Ray &p_ray, float &p_pdf);

			Vector3 SamplePoint(const Vector3 &p_viewPoint, float p_u, float p_v, Vector3 &p_lightSurfaceNormal, float &p_pdf);
			Vector3 SamplePoint(float p_u, float p_v, Vector3 &p_lightSurfaceNormal, float &p_pdf);

			ITexture *GetTexture(void) const;
			void SetTexture(ITexture *p_pTexture);

			Spectrum GetIntensity(void) const;
			void SetIntensity(const Spectrum &p_intensity);
		
		protected:
			Spectrum Radiance(const Vector3 &p_direction);

		protected:
			ITexture *m_pTexture;
			Spectrum m_intensity;
		};
	}
}