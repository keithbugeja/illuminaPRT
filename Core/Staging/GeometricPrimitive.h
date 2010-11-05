//----------------------------------------------------------------------------------------------
//	Filename:	GeometricPrimitive.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>

#include "Staging/Primitive.h"

namespace Illumina 
{
	namespace Core
	{
		class GeometricPrimitive : public IPrimitive
		{
		protected:
			Shape* m_pShape;

		public:
			inline Shape *GetShape(void) const { return m_pShape; }
			inline void SetShape(Shape *p_pShape) { m_pShape = p_pShape; }

			boost::shared_ptr<IBoundingVolume> GetWorldBounds(void) const;
			inline bool IsBounded(void) const { return (m_pShape != NULL && m_pShape->IsBounded()); }

			bool Intersect(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface, float &p_fTestDensity) const;
			bool Intersect(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface) const;
			bool Intersect(const Ray &p_ray, float p_fTime) const;

			std::string ToString(void) const;
		};
	} 
}