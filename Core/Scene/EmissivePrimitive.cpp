//----------------------------------------------------------------------------------------------
//	Filename:	EmissivePrimitive.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Scene/EmissivePrimitive.h"
#include "Geometry/Intersection.h"
#include "Shape/Shape.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
EmissivePrimitive::EmissivePrimitive(void)
	: m_pLight(NULL)
{ }
//----------------------------------------------------------------------------------------------
IAreaLight* EmissivePrimitive::GetLight(void) const {
	return m_pLight;
}
//----------------------------------------------------------------------------------------------
void EmissivePrimitive::SetLight(IAreaLight *p_pLight) 
{
	m_pLight = p_pLight;
	m_pLight->SetWorldTransform(&this->WorldTransform);
}
//----------------------------------------------------------------------------------------------
bool EmissivePrimitive::Intersects(const Ray &p_ray, Intersection &p_intersection)
{
	if (GeometricPrimitive::Intersects(p_ray, p_intersection))
	{
		p_intersection.SetLight(m_pLight);
		return true;
	}

	return false;
}
//----------------------------------------------------------------------------------------------
std::string EmissivePrimitive::ToString(void) const { 
	return "[EmissivePrimitive]"; 
}
//----------------------------------------------------------------------------------------------
