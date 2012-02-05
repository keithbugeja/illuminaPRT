//----------------------------------------------------------------------------------------------
//	Filename:	BasicSpace.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>

#include "Scene/Primitive.h"
#include "Space/BasicSpace.h"
#include "Geometry/Intersection.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
bool BasicSpace::Initialise(void) { 
	return true; 
}
//----------------------------------------------------------------------------------------------
void BasicSpace::Shutdown(void)  
{ }
//----------------------------------------------------------------------------------------------
bool BasicSpace::Build(void) { 
	return true; 
}
//----------------------------------------------------------------------------------------------
bool BasicSpace::Update(void) { 
	return true; 
}
//----------------------------------------------------------------------------------------------
bool BasicSpace::Intersects(const Ray &p_ray, Intersection &p_intersection) const
{
	Intersection intersection;
	Ray ray(p_ray);

	bool bHit = false;

	for (int primitiveIdx = 0, count = (int)PrimitiveList.Size(); primitiveIdx < count; primitiveIdx++)
	{
		if (PrimitiveList[primitiveIdx]->Intersect(ray, intersection) && 
			ray.Max > intersection.Surface.Distance)
		{
			ray.Max = intersection.Surface.Distance;
			p_intersection = intersection;
			bHit = true;
			//return true;
		}
	}

	return bHit;
}
//----------------------------------------------------------------------------------------------
bool BasicSpace::Intersects(const Ray &p_ray, Intersection &p_intersection, IPrimitive *p_pExclude) const
{
	Intersection intersection;
	Ray ray(p_ray);

	bool bHit = false;

	for (int primitiveIdx = 0, count = (int)PrimitiveList.Size(); primitiveIdx < count; primitiveIdx++)
	{
		if (PrimitiveList[primitiveIdx] != p_pExclude &&
			PrimitiveList[primitiveIdx]->Intersect(ray, p_intersection) && 
			ray.Max > intersection.Surface.Distance)
		{
			ray.Max = intersection.Surface.Distance;
			p_intersection = intersection;
			bHit = true;
			//return true;
		}
	}

	return bHit;
}
//----------------------------------------------------------------------------------------------
bool BasicSpace::Intersects(const Ray &p_ray) const
{
	for (int primitiveIdx = 0, count = (int)PrimitiveList.Size(); primitiveIdx < count; primitiveIdx++)
	{
		if (PrimitiveList[primitiveIdx]->Intersect(p_ray))
			return true;
	}

	return false;
}
//----------------------------------------------------------------------------------------------
bool BasicSpace::Intersects(const Ray &p_ray, IPrimitive *p_pExclude) const
{
	for (int primitiveIdx = 0, count = (int)PrimitiveList.Size(); primitiveIdx < count; primitiveIdx++)
	{
		if (PrimitiveList[primitiveIdx] != p_pExclude && 
			PrimitiveList[primitiveIdx]->Intersect(p_ray))
			return true;
	}

	return false;
}
//----------------------------------------------------------------------------------------------
