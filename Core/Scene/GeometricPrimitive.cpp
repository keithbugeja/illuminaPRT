//----------------------------------------------------------------------------------------------
//	Filename:	GeometricPrimitive.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Scene/GeometricPrimitive.h"
#include "Geometry/Intersection.h"
#include "Material/MaterialGroup.h"
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
Vector3 GeometricPrimitive::SamplePoint(float p_u, float p_v, Vector3 &p_normal)
{
	BOOST_ASSERT(m_pShape != NULL);

	if (!WorldTransform.IsIdentity())
	{
		Vector3 normal,
			point = m_pShape->SamplePoint(p_u, p_v, normal);
		
		WorldTransform.Rotate(normal, p_normal);
		return WorldTransform.Apply(point);
	}
	else
	{
		return m_pShape->SamplePoint(p_u, p_v, p_normal);
	}
}
//----------------------------------------------------------------------------------------------
Vector3 GeometricPrimitive::SamplePoint(const Vector3 &p_viewPoint, float p_u, float p_v, Vector3 &p_normal)
{
	BOOST_ASSERT(m_pShape != NULL);

	if (!WorldTransform.IsIdentity())
	{
		Vector3 normal,
			viewPoint = WorldTransform.ApplyInverse(p_viewPoint),
			point = m_pShape->SamplePoint(viewPoint, p_u, p_v, normal);
		
		WorldTransform.Rotate(normal, p_normal);
		return WorldTransform.Apply(point);
	}
	else
	{
		return m_pShape->SamplePoint(p_viewPoint, p_u, p_v, p_normal);
	}
}
//----------------------------------------------------------------------------------------------
bool GeometricPrimitive::Intersect(const Ray &p_ray, Intersection &p_intersection)
{
	// Todo:
	// Major bottleneck in ray inverse transform and normal rotation
	// Try speeding that up.

	if (!WorldTransform.IsIdentity())
	{
		const Ray &invRay = p_ray.ApplyInverse(WorldTransform);
	
		if (m_pShape->GetBoundingVolume()->Intersects(invRay))
		{
			if (m_pShape->Intersects(invRay, p_intersection.Surface))
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
			
				// Update ray information both in world and surface space
				p_intersection.Surface.RayOriginWS = p_ray.Origin;
				p_intersection.Surface.RayDirectionWS = p_ray.Direction;

				p_intersection.Surface.RayOrigin = invRay.Origin;
				p_intersection.Surface.RayDirection = invRay.Direction;

				// Update primitive world transform
				p_intersection.WorldTransform = WorldTransform;

				// Update primitive, materials and lights
				p_intersection.SetPrimitive(this);
				p_intersection.SetLight(NULL);

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
			if (m_pShape->Intersects(p_ray, p_intersection.Surface))
			{
				// Update geometry normal
				p_intersection.Surface.GeometryBasisWS.InitFromW(p_intersection.Surface.GeometryNormal);

				// Update shading normal
				p_intersection.Surface.ShadingBasisWS.InitFromW(p_intersection.Surface.ShadingNormal);

				// Update intersection point in world space
				p_intersection.Surface.PointWS = p_ray.PointAlongRay(p_intersection.Surface.Distance);

				// Update ray information both in world and surface space
				p_intersection.Surface.RayOrigin = 
					p_intersection.Surface.RayOriginWS = p_ray.Origin;

				p_intersection.Surface.RayDirection = 
					p_intersection.Surface.RayDirectionWS = p_ray.Direction;

				// Update primitive world transform
				p_intersection.WorldTransform.Reset(); //= WorldTransform;

				// Update primitive, materials and lights
				p_intersection.SetPrimitive(this);
				p_intersection.SetLight(NULL);

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
bool GeometricPrimitive::Intersect(const Ray &p_ray)
{
	if (!WorldTransform.IsIdentity())
	{
		const Ray &invRay = p_ray.ApplyInverse(WorldTransform);
		return (m_pShape->GetBoundingVolume()->Intersects(invRay) && 
				m_pShape->Intersects(invRay));
	}
	else
	{
		return (m_pShape->GetBoundingVolume()->Intersects(p_ray) && 
				m_pShape->Intersects(p_ray));
	}
}
//----------------------------------------------------------------------------------------------
std::string GeometricPrimitive::ToString(void) const { 
	return "[GeometricPrimitive]"; 
}
//----------------------------------------------------------------------------------------------
