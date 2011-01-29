//----------------------------------------------------------------------------------------------
//	Filename:	BasicSpace.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>

#include "Staging/Primitive.h"

#include "Space/BasicSpace.h"
#include "Geometry/Intersection.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
BasicSpace::BasicSpace(const std::string &p_strId)
	: ISpace(p_strId)
{ }
//----------------------------------------------------------------------------------------------
BasicSpace::BasicSpace(void)
{ }
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
bool BasicSpace::Intersects(const Ray &p_ray, float p_fTime, Intersection &p_intersection) const
{
	Ray ray(p_ray);
	bool bHit = false;

	for (int primitiveIdx = 0, count = (int)PrimitiveList.Size(); primitiveIdx < count; primitiveIdx++)
	{
		if (PrimitiveList[primitiveIdx]->Intersect(ray, p_fTime, p_intersection))
		{
			ray.Max = Maths::Min(ray.Max, p_intersection.Surface.Distance);
			bHit = true;
		}
	}

	return bHit;
}
//----------------------------------------------------------------------------------------------
bool BasicSpace::Intersects(const Ray &p_ray, float p_fTime, Intersection &p_intersection, IPrimitive *p_pExclude) const
{
	Ray ray(p_ray);
	bool bHit = false;

	for (int primitiveIdx = 0, count = (int)PrimitiveList.Size(); primitiveIdx < count; primitiveIdx++)
	{
		if (PrimitiveList[primitiveIdx] != p_pExclude)
		{
			if (PrimitiveList[primitiveIdx]->Intersect(ray, p_fTime, p_intersection))
			{
				ray.Max = Maths::Min(ray.Max, p_intersection.Surface.Distance);
				bHit = true;
			}
		}
	}

	return bHit;
}
//----------------------------------------------------------------------------------------------
bool BasicSpace::Intersects(const Ray &p_ray, float p_fTime) const
{
	for (int primitiveIdx = 0, count = (int)PrimitiveList.Size(); primitiveIdx < count; primitiveIdx++)
	{
		if (PrimitiveList[primitiveIdx]->Intersect(p_ray, p_fTime))
			return true;
	}

	return false;
}
//----------------------------------------------------------------------------------------------
bool BasicSpace::Intersects(const Ray &p_ray, float p_fTime, IPrimitive *p_pExclude) const
{
	for (int primitiveIdx = 0, count = (int)PrimitiveList.Size(); primitiveIdx < count; primitiveIdx++)
	{
		if (PrimitiveList[primitiveIdx] != p_pExclude)
		{
			if (PrimitiveList[primitiveIdx]->Intersect(p_ray, p_fTime))
				return true;
		}
	}

	return false;
}
//----------------------------------------------------------------------------------------------
