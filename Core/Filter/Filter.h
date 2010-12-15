//----------------------------------------------------------------------------------------------
//	Filename:	Filter.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Implementation for Rays in R3
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"

#include "Geometry/Vector2.h"
#include "Geometry/Vector3.h"

namespace Illumina 
{
	namespace Core
	{
		class IFilter
		{
		public:
			virtual void operator()(Vector2 *p_pSamples, int p_nSampleCount) = 0;
		};
	} 
}