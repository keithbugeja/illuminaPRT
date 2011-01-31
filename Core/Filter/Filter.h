//----------------------------------------------------------------------------------------------
//	Filename:	Filter.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "System/FactoryManager.h"

#include "Object/Object.h"

#include "Geometry/Vector2.h"
#include "Geometry/Vector3.h"

//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		// IFilter : Abstract base class for pixel filtering methods. 
		//----------------------------------------------------------------------------------------------
		class IFilter :
			public Object
		{
		protected:
			IFilter(const std::string &p_strName) : Object(p_strName) { }
			IFilter(void) { }

		public:
			virtual void operator()(Vector2 *p_pSamples, int p_nSampleCount) = 0;
			virtual std::string ToString(void) const { return "IFilter"; }
		};

		//----------------------------------------------------------------------------------------------
		// FilterManager : All Filter factories must register with object.
		//----------------------------------------------------------------------------------------------
		typedef FactoryManager<IFilter> FilterManager;
	} 
}