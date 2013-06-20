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
void BasicSpace::ComputeBoundingBox(void) 
{
	if (m_bRebuildRequired)
	{
		m_aabb.Invalidate();

		for (int nIdx = 0, count = (int)PrimitiveList.Size(); nIdx < count; nIdx++)
		{
			IPrimitive *primitive = PrimitiveList.At(nIdx);

			if (primitive->IsBounded())
				m_aabb.Union(*(primitive->GetWorldBounds()));
		}
	}
}

//----------------------------------------------------------------------------------------------
bool BasicSpace::Initialise(void) 
{ 
	m_bRebuildRequired = true;

	return true; 
}
//----------------------------------------------------------------------------------------------
void BasicSpace::Shutdown(void)  
{ }
//----------------------------------------------------------------------------------------------
bool BasicSpace::Build(void) 
{ 
	ComputeBoundingBox();

	return true; 
}
//----------------------------------------------------------------------------------------------
bool BasicSpace::Update(void) { 
	return true; 
}
//----------------------------------------------------------------------------------------------
IBoundingVolume *BasicSpace::GetBoundingVolume(void) {
	return &m_aabb;
}
//----------------------------------------------------------------------------------------------
bool BasicSpace::Intersects(const Ray &p_ray, Intersection &p_intersection) const
{
	bool bIntersect = false;
	Ray ray(p_ray);

	for (int index = 0, count = (int)PrimitiveList.Size(); index < count; index++)
	{
		if (PrimitiveList[index]->Intersects(ray, p_intersection))
		{
			bIntersect = true;
			ray.Max = Maths::Min(ray.Max, p_intersection.Surface.Distance);
		}
	}

	return bIntersect;
}
//----------------------------------------------------------------------------------------------
bool BasicSpace::Intersects(const Ray &p_ray, Intersection &p_intersection, IPrimitive *p_pExclude) const
{
	bool bIntersect = false;
	Ray ray(p_ray);

	for (int index = 0, count = (int)PrimitiveList.Size(); index < count; index++)
	{
		if (PrimitiveList[index] != p_pExclude && 
			PrimitiveList[index]->Intersects(ray, p_intersection))
		{
			bIntersect = true;
			ray.Max = Maths::Min(ray.Max, p_intersection.Surface.Distance);
		}
	}

	return bIntersect;
}
//----------------------------------------------------------------------------------------------
bool BasicSpace::Intersects(const Ray &p_ray) const
{
	for (int primitiveIdx = 0, count = (int)PrimitiveList.Size(); primitiveIdx < count; primitiveIdx++)
	{
		if (PrimitiveList[primitiveIdx]->Intersects(p_ray))
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
			PrimitiveList[primitiveIdx]->Intersects(p_ray))
			return true;
	}

	return false;
}
//----------------------------------------------------------------------------------------------
