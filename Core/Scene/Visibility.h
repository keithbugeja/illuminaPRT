//----------------------------------------------------------------------------------------------
//	Filename:	Visibility.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "Geometry/Ray.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class Scene;

		class VisibilityQuery
		{
		protected:
			Scene* m_pScene;
			Ray m_queryRay;

		public:
			VisibilityQuery(Scene *p_pScene);
			VisibilityQuery(Scene *p_pScene, const Ray &p_queryRay);
			VisibilityQuery(Scene *p_pScene, const Vector3 &p_segmentStart, const Vector3 &p_segmentEnd);
			VisibilityQuery(Scene *p_pScene, const Vector3 &p_segmentStart, float p_fEpsilonStart, const Vector3 &p_segmentEnd, float p_fEpsilonEnd);

			bool IsOccluded(void); 
			bool IsOccluded(IPrimitive *p_pExclude);

			void SetSegment(const Vector3 &p_segmentStart, const Vector3 &p_segmentEnd);
			void SetSegment(const Vector3 &p_segmentStart, float p_fEpsilonStart, const Vector3 &p_segmentEnd, float p_fEpsilonEnd);
			void SetSegment(const Vector3 &p_segmentStart, const Vector3 &p_segmentDirection, float p_fSegmentLength, float p_fSegmentEpsilon);
		};
	}
}