//----------------------------------------------------------------------------------------------
//	Filename:	Lock.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/Platform.h"

namespace Illumina 
{
	namespace Core 
	{
		class ILock
		{
		public:
			virtual ~ILock(void) { }

			virtual bool TryLock(void) = 0;
			virtual void Lock(void) = 0;
			virtual void Unlock(void) = 0;
		};
	}
}