//----------------------------------------------------------------------------------------------
//	Filename:	Factory.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/ArgumentMap.h"

namespace Illumina
{
	namespace Core
	{
		template<class T>
		class Factory
		{
		public:
			virtual T *CreateInstance(void) = 0;
			virtual T *CreateInstance(ArgumentMap &p_argumentMap) = 0;
		};
	}
}