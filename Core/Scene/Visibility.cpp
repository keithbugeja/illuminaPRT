//----------------------------------------------------------------------------------------------
//	Filename:	Visibility.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Geometry/Vector3.h"
#include "Scene/Visibility.h"
#include "Scene/Scene.h"
#include "Space/Space.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
VisibilityQuery::VisibilityQuery(Scene *p_pScene)
	: m_pScene(p_pScene)
{ }
//----------------------------------------------------------------------------------------------
VisibilityQuery::VisibilityQuery(Scene *p_pScene, const Ray &p_queryRay) 
	: m_pScene(p_pScene)
	, m_queryRay(p_queryRay)
{ }
//----------------------------------------------------------------------------------------------
VisibilityQuery::VisibilityQuery(Scene *p_pScene, const Vector3 &p_segmentStart, const Vector3 &p_segmentEnd)
	: m_pScene(p_pScene)
	, m_queryRay(p_segmentStart, p_segmentEnd - p_segmentStart)
{ }
//----------------------------------------------------------------------------------------------
VisibilityQuery::VisibilityQuery(Scene *p_pScene, const Vector3 &p_segmentStart, float p_fEpsilonStart, const Vector3 &p_segmentEnd, float p_fEpsilonEnd)
	: m_pScene(p_pScene)
	, m_queryRay(p_segmentStart, p_segmentEnd - p_segmentStart, p_fEpsilonStart, 1.0f - p_fEpsilonEnd)
{ }
//----------------------------------------------------------------------------------------------
bool VisibilityQuery::IsOccluded(void)
{
	// BOOST_ASSERT(m_pScene != NULL);
	return m_pScene->Intersects(m_queryRay);
}
//----------------------------------------------------------------------------------------------
bool VisibilityQuery::IsOccluded(IPrimitive *p_pExclude)
{
	// BOOST_ASSERT(m_pScene != NULL);
	return m_pScene->Intersects(m_queryRay, p_pExclude);
}
//----------------------------------------------------------------------------------------------
void VisibilityQuery::SetSegment(const Vector3 &p_segmentStart, const Vector3 &p_segmentEnd)
{
	m_queryRay.Set(p_segmentStart, p_segmentEnd - p_segmentStart, 0.0f, 1.0f);
}
//----------------------------------------------------------------------------------------------
void VisibilityQuery::SetSegment(const Vector3 &p_segmentStart, float p_fEpsilonStart, const Vector3 &p_segmentEnd, float p_fEpsilonEnd)
{
	m_queryRay.Set(p_segmentStart, p_segmentEnd - p_segmentStart, p_fEpsilonStart, 1.0f - p_fEpsilonEnd);
}
//----------------------------------------------------------------------------------------------
void VisibilityQuery::SetSegment(const Vector3 &p_segmentStart, const Vector3 &p_segmentDirection, float p_fSegmentLength, float p_fSegmentEpsilon)
{
	m_queryRay.Set(p_segmentStart, p_segmentDirection, p_fSegmentEpsilon, p_fSegmentLength - p_fSegmentEpsilon);
}
//----------------------------------------------------------------------------------------------
