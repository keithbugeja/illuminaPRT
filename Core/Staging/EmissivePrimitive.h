//----------------------------------------------------------------------------------------------
//	Filename:	EmissivePrimitive.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Staging/GeometricPrimitive.h"
#include "Light/AreaLight.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		class EmissivePrimitive 
			: public GeometricPrimitive
		{
			using GeometricPrimitive::m_pMaterial;
			using GeometricPrimitive::m_pShape;

		protected:
			AreaLight *m_pLight;

		public:
			EmissivePrimitive(void);

			AreaLight* GetLight(void) const;
			void SetLight(AreaLight *p_pLight);

			bool Intersect(const Ray &p_ray, float p_fTime, Intersection &p_intersection);
			//bool Intersect(const Ray &p_ray, float p_fTime);

			std::string ToString(void) const;
		};
	} 
}