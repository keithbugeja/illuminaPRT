//----------------------------------------------------------------------------------------------
//	Filename:	PointLight.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Light/Light.h"

#include "Geometry/Vector3.h"
#include "Spectrum/Spectrum.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class PointLight : public ILight
		{
		public:
			Spectrum Power(void);
			Spectrum Radiance(const Vector3 &p_point, Vector3 &p_wOut, VisibilityQuery &p_visibilityQuery);
			Spectrum Radiance(const Vector3 &p_point, double p_u, double p_v, Vector3& p_wOut, VisibilityQuery &p_visibilityQuery);

			PointLight(const Vector3 &p_position, const Spectrum &p_intensity);
			PointLight(const PointLight &p_pointLight);
		
			Vector3 GetPosition(void) const;
			void SetPosition(const Vector3 &p_position);

			Spectrum GetIntensity(void) const;
			void SetIntensity(const Spectrum &p_intensity);

		protected:
			Vector3 m_position;
			Spectrum m_intensity;
		};
	}
}