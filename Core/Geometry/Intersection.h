//----------------------------------------------------------------------------------------------
//	Filename:	Intersection.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"

#include "Geometry/Transform.h"
#include "Shape/DifferentialSurface.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		class Intersection
		{
		protected:
			IPrimitive *m_pPrimitive;

		public:
			DifferentialSurface Surface;
			Transformation WorldTransform;
			float RayEpsilon;

		public:
			Intersection(void);

			IPrimitive* GetPrimitive(void);
			void SetPrimitive(IPrimitive* p_pPrimitive);
		};
	}
}