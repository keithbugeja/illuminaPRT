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
		class AreaLight : 
			public ILight
		{
		protected:
			IShape *m_pShape;
			Transformation *m_pWorldTransform;

		public:
			AreaLight(void);

			Spectrum Power(void) = 0;
			Spectrum Radiance(const Vector3 &p_point, Vector3 &p_wOut, VisibilityQuery &p_visibilityQuery) = 0;
			Spectrum Radiance(const Vector3 &p_point, double p_u, double p_v, Vector3& p_wOut, VisibilityQuery &p_visibilityQuery) = 0;

			IShape* GetShape(void) const;
			void SetShape(IShape* p_pShape);

			Transformation* GetWorldTransform(void) const;
			void SetWorldTransform(Transformation *p_pWorldTransform);
		};
	}
}