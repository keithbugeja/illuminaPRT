//----------------------------------------------------------------------------------------------
//	Filename:	GeometricPrimitive.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Geometry/Intersection.h"
#include "Staging/GeometricPrimitive.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
boost::shared_ptr<IBoundingVolume> GeometricPrimitive::GetWorldBounds(void) const 
{
	return m_pShape->GetBoundingVolume()->Apply(WorldTransform);
}
//----------------------------------------------------------------------------------------------
bool GeometricPrimitive::Intersect(const Ray &p_ray, float p_fTime, Intersection &p_intersection)
{
	const Ray &invRay = p_ray.ApplyInverse(WorldTransform);
	
	if (m_pShape->GetBoundingVolume()->Intersects(invRay))
	{
		DifferentialSurface &surface = p_intersection.GetDifferentialSurface();

		if (m_pShape->Intersects(invRay, p_fTime, surface))
		{
			Vector3 NormalWS;
			WorldTransform.Rotate(surface.Normal, NormalWS);
			surface.BasisWS.InitFromU(NormalWS);
			surface.PointWS = p_ray.PointAlongRay(surface.Distance);
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
	return "GeometricPrimitive"; 
}
//----------------------------------------------------------------------------------------------
