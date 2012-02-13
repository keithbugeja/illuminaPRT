//----------------------------------------------------------------------------------------------
//	Filename:	Intersection.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Geometry/Intersection.h"
#include "Scene/Primitive.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
Intersection::Intersection(void)
	: m_pPrimitive(NULL)
	, m_pMaterial(NULL)
	, m_pLight(NULL)
	, Surface()
	, WorldTransform()
{ }
//----------------------------------------------------------------------------------------------
Intersection::Intersection(const Intersection &p_intersection)
	: m_pPrimitive(p_intersection.m_pPrimitive)
	, m_pMaterial(p_intersection.m_pMaterial)
	, m_pLight(p_intersection.m_pLight)
	, Surface(p_intersection.Surface)
	, WorldTransform(p_intersection.WorldTransform)
{ }
//----------------------------------------------------------------------------------------------
const Intersection& Intersection::operator=(const Intersection &p_intersection)
{
	m_pPrimitive = p_intersection.m_pPrimitive;
	m_pMaterial = p_intersection.m_pMaterial;
	m_pLight = p_intersection.m_pLight;
	Surface = p_intersection.Surface;
	WorldTransform = p_intersection.WorldTransform;
	
	return *this;
}
//----------------------------------------------------------------------------------------------
void Intersection::Reset(void)
{
	m_pLight = NULL;
	m_pMaterial = NULL;
	m_pPrimitive = NULL;

	Surface.Reset();
	WorldTransform.Reset();
}
//----------------------------------------------------------------------------------------------
bool Intersection::IsValid(void) const {
	return m_pPrimitive != NULL;
}
//----------------------------------------------------------------------------------------------
bool Intersection::IsEmissive(void) const {
	return m_pLight != NULL;
}
//----------------------------------------------------------------------------------------------
bool Intersection::HasMaterial(void) const {
		return m_pMaterial != NULL;
}
//----------------------------------------------------------------------------------------------
IPrimitive* Intersection::GetPrimitive(void) const { 
	return m_pPrimitive; 
}
//----------------------------------------------------------------------------------------------
void Intersection::SetPrimitive(IPrimitive *p_pPrimitive) { 
	m_pPrimitive = p_pPrimitive; 
}
//----------------------------------------------------------------------------------------------
IMaterial* Intersection::GetMaterial(void) const {
	return m_pMaterial;
}
//----------------------------------------------------------------------------------------------
void Intersection::SetMaterial(IMaterial *p_pMaterial) { 
	m_pMaterial = p_pMaterial; 
}
//----------------------------------------------------------------------------------------------
ILight* Intersection::GetLight(void) const {
	return m_pLight;
}
//----------------------------------------------------------------------------------------------
void Intersection::SetLight(ILight* p_pLight) {
	m_pLight = p_pLight;
}
//----------------------------------------------------------------------------------------------
