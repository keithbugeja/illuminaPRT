//----------------------------------------------------------------------------------------------
//	Filename:	AreaLight.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "Light/Light.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class IAreaLight : 
			public ILight
		{
		protected:
			IShape *m_pShape;
			Transformation *m_pWorldTransform;

		protected:
			IAreaLight(void);
			IAreaLight(const std::string &p_strId);
			IAreaLight(Transformation *p_pWorldTransform, IShape* p_pShape);
			IAreaLight(const std::string &p_strId, Transformation *p_pWorldTransform, IShape* p_pShape);

		public:
			float Pdf(const Vector3 &p_point, const Vector3 &p_wOut) = 0;

			Spectrum Power(void) = 0;
			Spectrum Radiance(const Vector3 &p_point, const Vector3 &p_normal, const Vector3 &p_wIn) = 0;
			Spectrum SampleRadiance(const Vector3 &p_point, Vector3 &p_wIn, VisibilityQuery &p_visibilityQuery) = 0;
			Spectrum SampleRadiance(const Vector3 &p_point, double p_u, double p_v, Vector3& p_wIn, VisibilityQuery &p_visibilityQuery) = 0;

			IShape* GetShape(void) const;
			void SetShape(IShape* p_pShape);

			Transformation* GetWorldTransform(void) const;
			void SetWorldTransform(Transformation *p_pWorldTransform);
		};
	}
}