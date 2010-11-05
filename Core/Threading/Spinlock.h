//----------------------------------------------------------------------------------------------
//	Filename:	Spinlock.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Threading/Lock.h"

namespace Illumina {
	namespace Core 
	{
		class Spinlock 
			: public ILock
		{
		protected:
			int m_backoff;
			volatile long m_lock;

		public:
			Spinlock(int p_backoff = 100);

			inline bool TryLock(void);
			inline void Lock(void);
			inline void Unlock(void);
		};
	}
}

#include "Spinlock.inl"
