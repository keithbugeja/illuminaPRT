//----------------------------------------------------------------------------------------------
//	Filename:	DiffuseAreaLight.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "Spectrum/Spectrum.h"
#include "Light/AreaLight.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class DiffuseAreaLight : 
			public IAreaLight
		{
			using IAreaLight::m_pShape;
			using IAreaLight::m_pWorldTransform;

		protected:
			Spectrum m_emit;
			float m_fArea;

		public:
			DiffuseAreaLight(const std::string &p_strName, Transformation *p_pWorldTransform, IShape* p_pShape, const Spectrum &p_emit);
			DiffuseAreaLight(Transformation *p_pWorldTransform, IShape* p_pShape, const Spectrum &p_emit);

			void SetShape(IShape *p_pShape);

			Spectrum Power(void);
			Spectrum Radiance(const Vector3 &p_lightSurfacePoint, const Vector3 &p_lightSurfaceNormal, const Vector3 &p_wIn);
			Spectrum SampleRadiance(const Vector3 &p_surfacePoint, double p_u, double p_v, Vector3 &p_wIn, float &p_pdf, VisibilityQuery &p_visibilityQuery);

			Vector3 SamplePoint(const Vector3 &p_viewPoint, double p_u, double p_v, Vector3 &p_lightSurfaceNormal, float &p_pdf);
			Vector3 SamplePoint(double p_u, double p_v, Vector3 &p_lightSurfaceNormal, float &p_pdf);

			float Pdf(const Vector3 &p_point, const Vector3 &p_wOut);
		};
	}
}