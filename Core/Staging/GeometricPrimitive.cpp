//----------------------------------------------------------------------------------------------
//	Filename:	GeometricPrimitive.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Staging/GeometricPrimitive.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
boost::shared_ptr<IBoundingVolume> GeometricPrimitive::GetWorldBounds(void) const 
{
	return m_pShape->GetBoundingVolume()->Apply(WorldTransform);
}
//----------------------------------------------------------------------------------------------
bool GeometricPrimitive::Intersect(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface, float &p_fTestDensity) const
{
	Ray &invRay = p_ray.ApplyInverse(WorldTransform);
	
	if (m_pShape->GetBoundingVolume()->Intersects(invRay))
	{
		if (m_pShape->Intersects(invRay, p_fTime, p_surface, p_fTestDensity))
		{
			Vector3 NormalWS;
			WorldTransform.Rotate(p_surface.Normal, NormalWS);
			//WorldTransform.Apply(p_surface.Normal, NormalWS);
			p_surface.BasisWS.InitFromU(NormalWS);
			p_surface.PointWS = p_ray.PointAlongRay(p_surface.Distance);
			return true;
		}
	}

	return false;
}
//----------------------------------------------------------------------------------------------
bool GeometricPrimitive::Intersect(const Ray &p_ray, float p_fTime, DifferentialSurface& p_surface) const
{
	Ray &invRay = p_ray.ApplyInverse(WorldTransform);
	
	if (m_pShape->GetBoundingVolume()->Intersects(invRay))
	{
		if (m_pShape->Intersects(invRay, p_fTime, p_surface))
		{
			Vector3 NormalWS;
			WorldTransform.Apply(p_surface.Normal, NormalWS);
			p_surface.BasisWS.InitFromU(NormalWS);
			p_surface.PointWS = p_ray.PointAlongRay(p_surface.Distance);

			return true;
		}
	}

	return false;
}
//----------------------------------------------------------------------------------------------
bool GeometricPrimitive::Intersect(const Ray &p_ray, float p_fTime) const
{
	Ray &invRay = p_ray.ApplyInverse(WorldTransform);
	return (m_pShape->GetBoundingVolume()->Intersects(p_ray) && 
			m_pShape->Intersects(invRay, p_fTime));
}
//----------------------------------------------------------------------------------------------
std::string GeometricPrimitive::ToString(void) const { 
	return "GeometricPrimitive"; 
}
//----------------------------------------------------------------------------------------------
