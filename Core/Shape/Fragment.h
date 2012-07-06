//----------------------------------------------------------------------------------------------
//	Filename:	Fragment.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"

#include "Geometry/Ray.h"
#include "Geometry/Basis.h"
#include "Geometry/BoundingVolume.h"

#include "Shape/DifferentialSurface.h"

namespace Illumina 
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		// IFragment
		//----------------------------------------------------------------------------------------------
		class IFragment 
		{
		public:
			virtual bool HasGroup(void) const = 0;
			virtual int GetGroupId(void) const = 0;
		};
	} 
}