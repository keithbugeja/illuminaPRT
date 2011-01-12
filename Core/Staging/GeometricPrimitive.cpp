//----------------------------------------------------------------------------------------------
//	Filename:	GeometricPrimitive.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Staging/GeometricPrimitive.h"
#include "Material/MaterialGroup.h"
#include "Geometry/Intersection.h"
#include "Shape/Shape.h"

#include <iostream>

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
GeometricPrimitive::GeometricPrimitive(void)
	: m_pMaterial(NULL)
	, m_pShape(NULL)
	
{ }
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

	if (!WorldTransform.IsIdentity())
	{
		const Ray &invRay = p_ray.ApplyInverse(WorldTransform);
	
		if (m_pShape->GetBoundingVolume()->Intersects(invRay))
		{
			if (m_pShape->Intersects(invRay, p_fTime, p_intersection.Surface))
			{
				Vector3 shadingNormalWS, 
					geometryNormalWS;

				// Update geometry normal
				WorldTransform.Rotate(p_intersection.Surface.GeometryNormal, geometryNormalWS);
				p_intersection.Surface.GeometryBasisWS.InitFromW(geometryNormalWS);

				// Update shading normal
				WorldTransform.Rotate(p_intersection.Surface.ShadingNormal, shadingNormalWS);
				p_intersection.Surface.ShadingBasisWS.InitFromW(shadingNormalWS);

				// Update intersection point in world space
				p_intersection.Surface.PointWS = p_ray.PointAlongRay(p_intersection.Surface.Distance);
			
				// Update primitive world transform
				p_intersection.WorldTransform = WorldTransform;

				// Update primitive and materials
				p_intersection.SetPrimitive(this);

				if (m_pMaterial != NULL && m_pMaterial->IsComposite())
				{
					MaterialGroup *pGroup = (MaterialGroup*)m_pMaterial;
					
					if (p_intersection.Surface.GetShape()->HasGroup())
						p_intersection.SetMaterial(pGroup->GetByGroupId(p_intersection.Surface.GetShape()->GetGroupId()));
					else
						p_intersection.SetMaterial(pGroup->GetByIndex(0));
				}
				else
					p_intersection.SetMaterial(m_pMaterial);
							
				return true;
			}
		}
	}
	else
	{
		if (m_pShape->GetBoundingVolume()->Intersects(p_ray))
		{
			if (m_pShape->Intersects(p_ray, p_fTime, p_intersection.Surface))
			{
				// Update geometry normal
				p_intersection.Surface.GeometryBasisWS.InitFromW(p_intersection.Surface.GeometryNormal);

				// Update shading normal
				p_intersection.Surface.ShadingBasisWS.InitFromW(p_intersection.Surface.ShadingNormal);

				// Update intersection point in world space
				p_intersection.Surface.PointWS = p_ray.PointAlongRay(p_intersection.Surface.Distance);
			
				// Update primitive world transform
				p_intersection.WorldTransform = WorldTransform;

				// Update primitive and materials
				p_intersection.SetPrimitive(this);
				
				if (m_pMaterial != NULL && m_pMaterial->IsComposite())
				{
					MaterialGroup *pGroup = (MaterialGroup*)m_pMaterial;
					
					if (p_intersection.Surface.GetShape()->HasGroup())
						p_intersection.SetMaterial(pGroup->GetByGroupId(p_intersection.Surface.GetShape()->GetGroupId()));
					else
						p_intersection.SetMaterial(pGroup->GetByIndex(0));
				}
				else
					p_intersection.SetMaterial(m_pMaterial);
			
				return true;
			}
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
