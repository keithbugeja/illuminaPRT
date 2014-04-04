//----------------------------------------------------------------------------------------------
//	Filename:	SpotLight.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Light/Light.h"

#include "Geometry/Vector3.h"
#include "Geometry/Basis.h"
#include "Spectrum/Spectrum.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class SpotLight : public ILight
		{
		public:
			SpotLight(const SpotLight &p_spotLight);
			SpotLight(const Vector3 &p_position, const Vector3 &p_direction, const Spectrum &p_intensity, float p_fWidth, float p_fFall);
			SpotLight(const std::string& p_strName, const Vector3 &p_position, const Vector3 &p_direction, const Spectrum &p_intensity, float p_fWidth, float p_fFall);

			float Pdf(const Vector3 &p_point, const Vector3 &p_wOut);

			Spectrum Power(void);

			Spectrum Radiance(const Vector3 &p_lightSurfacePoint, const Vector3 &p_lightSurfaceNormal, const Vector3 &p_wIn);

			Spectrum SampleRadiance(const Vector3 &p_surfacePoint, float p_u, float p_v, Vector3 &p_wIn, float &p_pdf, VisibilityQuery &p_visibilityQuery);
			Spectrum SampleRadiance(const Scene *p_pScene, float p_u, float p_v, float p_w, float p_x, Ray &p_ray, float &p_pdf);

			Vector3 SamplePoint(float p_u, float p_v, Vector3 &p_lightSurfaceNormal, float &p_pdf);
		
			Vector3 GetPosition(void) const;
			void SetPosition(const Vector3 &p_position);

			Vector3 GetDirection(void) const;
			void SetDirection(const Vector3 &p_direction);

			Spectrum GetIntensity(void) const;
			void SetIntensity(const Spectrum &p_intensity);

			float Falloff(const Vector3 &p_direction) const;
		protected:
			OrthonormalBasis m_basis;
			Spectrum m_intensity;
			Vector3 m_position;
			Vector3 m_direction;
			float m_fCosTotalWidth,
				m_fCosFalloffStart;
		};
	}
}