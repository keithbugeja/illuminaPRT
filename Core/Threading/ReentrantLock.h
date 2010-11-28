//----------------------------------------------------------------------------------------------
//	Filename:	ReentrantLock.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include "Threading/Lock.h"

namespace Illumina
{
	namespace Core
	{
		class ReentrantLock : public ILock
		{
		protected:
			boost::recursive_mutex m_lock;

		public:
			ReentrantLock(void) {
				//m_lock.initialize();
			}

			~ReentrantLock(void) {
				//m_lock.destroy();
			}

			inline bool TryLock(void)
			{
				return m_lock.try_lock();
			}

			inline void Lock(void) {
				m_lock.lock();
			}

			inline void Unlock(void) {
				m_lock.unlock();
			}
		};
	}
}
