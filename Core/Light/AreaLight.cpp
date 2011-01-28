//----------------------------------------------------------------------------------------------
//	Filename:	AreaLight.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Light/AreaLight.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
IAreaLight::IAreaLight(void)
	: m_pShape(NULL)
	, m_pWorldTransform(NULL)
{ }
//----------------------------------------------------------------------------------------------
IAreaLight::IAreaLight(const std::string &p_strId)
	: ILight(p_strId) 
	, m_pShape(NULL)
	, m_pWorldTransform(NULL)
{ }
//----------------------------------------------------------------------------------------------
IAreaLight::IAreaLight(Transformation *p_pWorldTransform, IShape* p_pShape)
	: m_pShape(p_pShape)
	, m_pWorldTransform(p_pWorldTransform)
{ }
//----------------------------------------------------------------------------------------------
IAreaLight::IAreaLight(const std::string &p_strId, Transformation *p_pWorldTransform, IShape* p_pShape)
	: ILight(p_strId) 
	, m_pShape(p_pShape)
	, m_pWorldTransform(p_pWorldTransform)
{ }
//----------------------------------------------------------------------------------------------
void IAreaLight::SetWorldTransform(Transformation *p_pWorldTransform) {
	m_pWorldTransform = p_pWorldTransform;
}
//----------------------------------------------------------------------------------------------
Transformation* IAreaLight::GetWorldTransform(void) const {
	return m_pWorldTransform;
}
//----------------------------------------------------------------------------------------------
IShape* IAreaLight::GetShape(void) const {
	return m_pShape;
}
//----------------------------------------------------------------------------------------------
void IAreaLight::SetShape(IShape* p_pShape) {
	m_pShape = p_pShape;
}
//----------------------------------------------------------------------------------------------
