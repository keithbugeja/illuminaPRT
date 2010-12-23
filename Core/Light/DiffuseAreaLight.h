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
			public AreaLight
		{
			using AreaLight::m_pShape;
			using AreaLight::m_pWorldTransform;

		protected:
			Spectrum m_emit;
			float m_fArea;

		public:
			DiffuseAreaLight(Transformation *p_worldTransform, IShape* p_pShape, const Spectrum& p_emit);

			void SetShape(IShape *p_pShape);

			Spectrum Power(void);
			Spectrum Radiance(const Vector3 &p_point, Vector3 &p_wOut, VisibilityQuery &p_visibilityQuery);
			Spectrum Radiance(const Vector3 &p_point, double p_u, double p_v, Vector3& p_wOut, VisibilityQuery &p_visibilityQuery);
		};
	}
}