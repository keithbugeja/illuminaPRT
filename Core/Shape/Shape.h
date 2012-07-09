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

#include "System/IlluminaPRT.h"
#include "System/FactoryManager.h"

#include "Object/Object.h"

#include "Geometry/Ray.h"
#include "Geometry/Basis.h"
#include "Geometry/BoundingVolume.h"

#include "Shape/DifferentialSurface.h"

namespace Illumina 
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		// IShape : Abstract base class for class for all shape primitives. 
		//----------------------------------------------------------------------------------------------
		class IShape 
			: public Object
		{
		public:
			IShape(void) : Object() { }
			IShape(const std::string& p_strName) : Object(p_strName) { }

			virtual bool HasGroup(void) const { return false; }
			virtual int GetGroupId(void) const { return -1; }

			virtual bool IsBounded(void) const = 0;
			virtual void ComputeBoundingVolume(void) = 0;
			virtual IBoundingVolume* GetBoundingVolume(void) const = 0;

			virtual bool IsCompilationRequired(void) const { return false; }
			virtual bool Compile(void) { return false; }

			virtual bool IsUpdateRequired(void) const { return false; }
			virtual bool Update(void) { return false; }

			virtual bool IsRebuildRequired(void) const { return false; }
			virtual bool Rebuild(void) { return false; }

			virtual bool Intersects(const Ray &p_ray, DifferentialSurface& p_surface) = 0;
			virtual bool Intersects(const Ray &p_ray) = 0;

			virtual float GetArea(void) const = 0;
			virtual float GetPdf(const Vector3 &p_point) const = 0;

			virtual Vector3 SamplePoint(float p_u, float p_v, Vector3 &p_normal) = 0;
			virtual Vector3 SamplePoint(const Vector3 &p_viewPoint, float p_u, float p_v, Vector3 &p_normal) { return SamplePoint(p_u, p_v, p_normal); }

			virtual std::string ToString(void) const { return "IShape"; };
		};

		//----------------------------------------------------------------------------------------------
		// ShapeManager : All Shape factories must register with object.
		//----------------------------------------------------------------------------------------------
		typedef FactoryManager<IShape> ShapeManager;
	} 
}