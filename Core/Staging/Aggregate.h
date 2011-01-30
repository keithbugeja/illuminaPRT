//----------------------------------------------------------------------------------------------
//	Filename:	Aggregate.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Scene/Primitive.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		class Aggregate : public IPrimitive
		{
		public:
			List<IPrimitive*> PrimitiveList;

			virtual void Prepare(void) { };

			std::string ToString(void) const { return "[Aggregate]"; }
		};
	} 
}