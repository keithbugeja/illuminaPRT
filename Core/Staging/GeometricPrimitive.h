//----------------------------------------------------------------------------------------------
//	Filename:	GeometricPrimitive.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Geometry/Transform.h"
#include "Staging/Primitive.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		class GeometricPrimitive 
			: public IPrimitive
		{
		protected:
			IShape *m_pShape;

		public:
			Transformation WorldTransform;

		public:
			inline IShape *GetShape(void) const { return m_pShape; }
			inline void SetShape(IShape *p_pShape) { m_pShape = p_pShape; }

			boost::shared_ptr<IBoundingVolume> GetWorldBounds(void) const;
			bool IsBounded(void) const;

			bool Intersect(const Ray &p_ray, float p_fTime, Intersection &p_intersection);
			bool Intersect(const Ray &p_ray, float p_fTime);

			std::string ToString(void) const;
		};
	} 
}