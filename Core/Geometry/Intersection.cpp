//----------------------------------------------------------------------------------------------
//	Filename:	Intersection.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Geometry/Intersection.h"
#include "Staging/Primitive.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
Intersection::Intersection(void)
	: m_pPrimitive(NULL)
	, Surface()
	, WorldTransform()
	, RayEpsilon(0.0f)
{ }
//----------------------------------------------------------------------------------------------
IPrimitive* Intersection::GetPrimitive(void) { 
	return m_pPrimitive; 
}
//----------------------------------------------------------------------------------------------
void Intersection::SetPrimitive(IPrimitive* p_pPrimitive) { 
	m_pPrimitive = p_pPrimitive; 
}
//----------------------------------------------------------------------------------------------
