//----------------------------------------------------------------------------------------------
//	Filename:	Shape.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Implementation for Rays in R3
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>

#include "Geometry/Ray.h"
#include "Geometry/Basis.h"
#include "Geometry/BoundingVolume.h"

#include "Shape/DifferentialSurface.h"

namespace Illumina 
{
	namespace Core
	{
		/* Base class for all shape primitives */
		class IShape
		{
		public:
			virtual bool IsBounded(void) const = 0;
			virtual void ComputeBoundingVolume(void) = 0;
			virtual IBoundingVolume* GetBoundingVolume(void) const = 0;

			virtual bool IsCompilationRequired(void) const { return false; }
			virtual bool Compile(void) { return false; }

			virtual bool IsUpdateRequired(void) const { return false; }
			virtual bool Update(void) { return false; }

			virtual bool IsRebuildRequired(void) const { return false; }
			virtual bool Rebuild(void) { return false; }

			virtual bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface& p_surface, float& p_fTestDensity) { return Intersects(p_ray, p_fTime, p_surface); }
			virtual bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface& p_surface) = 0;
			virtual bool Intersects(const Ray &p_ray, float p_fTime) = 0;

			virtual std::string ToString(void) const { return "[IShape]"; };
		};
	} 
}