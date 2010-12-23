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
	// Todo:
	// Major bottleneck in ray inverse transform and normal rotation
	// Try speeding that up.

	const Ray &invRay = p_ray.ApplyInverse(WorldTransform);
	
	if (m_pShape->GetBoundingVolume()->Intersects(invRay))
	{
		if (m_pShape->Intersects(invRay, p_fTime, p_intersection.Surface))
		{
			Vector3 shadingNormalWS, 
				geometryNormalWS;

			// Update geometry normal
			WorldTransform.Rotate(p_intersection.Surface.GeometryNormal, geometryNormalWS);
			p_intersection.Surface.ShadingBasisWS.InitFromV(shadingNormalWS);

			// Update shading normal
			WorldTransform.Rotate(p_intersection.Surface.ShadingNormal, shadingNormalWS);
			p_intersection.Surface.GeometryBasisWS.InitFromV(geometryNormalWS);

			// Update intersection point in world space
			p_intersection.Surface.PointWS = p_ray.PointAlongRay(p_intersection.Surface.Distance);
			
			// Update primitive world transform
			p_intersection.WorldTransform = WorldTransform;

			// Update primitive and materials
			p_intersection.SetPrimitive(this);
			p_intersection.SetMaterial(m_pMaterial);
			
			return true;
		}
	}

	// Clear primitive and materials - intersection was not successful
	p_intersection.SetPrimitive(NULL);
	p_intersection.SetMaterial(NULL);

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
