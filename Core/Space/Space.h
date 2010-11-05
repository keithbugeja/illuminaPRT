//----------------------------------------------------------------------------------------------
//	Filename:	Primitive.h
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
		class ISpace
		{
		public:
			List<IPrimitive*> PrimitiveList;

			virtual void Initialise(void) = 0;
			virtual void Update(void) = 0;
			virtual void Build(void) = 0;

			virtual bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface& p_surface, float& p_fTestDensity) { return Intersects(p_ray, p_fTime, p_surface); }
			virtual bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface) const = 0;
			virtual bool Intersects(const Ray &p_ray, float p_fTime) const = 0;

			virtual std::string ToString(void) const { return "ISpace"; }
		};
	} 
}