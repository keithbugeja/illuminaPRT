//----------------------------------------------------------------------------------------------
//	Filename:	CollectionPrimitive.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>

#include "Staging/Primitive.h"

namespace Illumina 
{
	namespace Core
	{
		class Aggregate : public IPrimitive
		{
		public:
			List<IPrimitive*> PrimitiveList;

			virtual void Prepare(void) { };

			std::string ToString(void) const { return "Aggregate"; }
		};
	} 
}