//----------------------------------------------------------------------------------------------
//	Filename:	Cloneable.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Exception/Exception.h"

namespace Illumina 
{
	namespace Core
	{
		class ICloneable
		{
			enum CloneType {
				Deep,
				Shallow
			};

			virtual ICloneable* Clone(CloneType p_cloneType = Deep, ICloneable* p_out = NULL) {
				throw new Exception("Method not supported!");
			}
		};
	} 
}