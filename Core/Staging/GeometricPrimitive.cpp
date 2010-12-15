//----------------------------------------------------------------------------------------------
//	Filename:	GeometricPrimitive.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Staging/GeometricPrimitive.h"
#include "Geometry/Intersection.h"
#include "Shape/Shape.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
boost::shared_ptr<IBoundingVolume> GeometricPrimitive::GetWorldBounds(void) const 
{
	return m_pShape->GetBoundingVolume()->Apply(WorldTransform);
}
//----------------------------------------------------------------------------------------------
bool GeometricPrimitive::IsBounded(void) const 
{ 
	return (m_pShape != NULL && m_pShape->IsBounded()); 
}
//----------------------------------------------------------------------------------------------
bool GeometricPrimitive::Intersect(const Ray &p_ray, float p_fTime, Intersection &p_intersection)
{
	const Ray &invRay = p_ray.ApplyInverse(WorldTransform);
	
	if (m_pShape->GetBoundingVolume()->Intersects(invRay))
	{
		if (m_pShape->Intersects(invRay, p_fTime, p_intersection.Surface))
		{
			Vector3 NormalWS;
			
			WorldTransform.Rotate(p_intersection.Surface.Normal, NormalWS);
			p_intersection.Surface.BasisWS.InitFromU(NormalWS);
			p_intersection.Surface.PointWS = p_ray.PointAlongRay(p_intersection.Surface.Distance);
			
			return true;
		}
	}

	return false;
}
//----------------------------------------------------------------------------------------------
bool GeometricPrimitive::Intersect(const Ray &p_ray, float p_fTime)
{
	const Ray &invRay = p_ray.ApplyInverse(WorldTransform);
	return (m_pShape->GetBoundingVolume()->Intersects(p_ray) && 
			m_pShape->Intersects(invRay, p_fTime));
}
//----------------------------------------------------------------------------------------------
std::string GeometricPrimitive::ToString(void) const { 
	return "[GeometricPrimitive]"; 
}
//----------------------------------------------------------------------------------------------
