#pragma once

#include "Geometry/Transform.h"
#include "Shape/DifferentialSurface.h"
#include "Staging/Primitive.h"

namespace Illumina 
{
	namespace Core
	{
		class Intersection
		{
		protected:
			Transformation m_worldTransform;
			DifferentialSurface m_differentialSurface;
			IPrimitive *m_pPrimitive;
			float m_fRayEpsilon;

		public:
			Intersection(void)
				: m_worldTransform()
				, m_differentialSurface()
				, m_pPrimitive(NULL)
				, m_fRayEpsilon(0.0f)
			{ }

			DifferentialSurface& GetDifferentialSurface(void) { return m_differentialSurface; }
			Transformation& GetWorldTransform(void) { return m_worldTransform; }
			
			IPrimitive* GetPrimitive(void) { return m_pPrimitive; }
			void SetPrimitive(IPrimitive* p_pPrimitive) { m_pPrimitive = p_pPrimitive; }

			float GetRayEpsilon(void) { return m_fRayEpsilon; }
			void SetRayEpsilon(float p_fRayEpsilon) { m_fRayEpsilon = p_fRayEpsilon; }
		};
	}
}