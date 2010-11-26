//----------------------------------------------------------------------------------------------
//	Filename:	Spinlock.inl
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
inline bool Spinlock::TryLock(void)
{
	#if defined(__COMPILER_MSVC__)
		return (!m_lock && !_interlockedbittestandset(&m_lock, 0));
	#elif defined(__COMPILER_GCC__)
		return (!m_lock && !__sync_lock_test_and_set(&m_lock, 1));
	#else
		#pragma message ("No Atomic TAS primitive available!")
		return false;
	#endif
}
//----------------------------------------------------------------------------------------------
inline void Spinlock::Lock(void)
{
	int backoff = 0, index;
	
	#if defined(__COMPILER_MSVC__)
		do 
		{
			backoff += m_backoff;

			while(m_lock) 
			{
				for (index = 0; index < backoff; index++) 
					_mm_pause(); 
			}
		} while(_interlockedbittestandset(&m_lock, 0));
	#elif defined(__COMPILER_GCC__)
		do 
		{
			backoff += m_backoff;

			while(m_lock) {
				for (index = 0; index < backoff; index++);
			}
		} while(__sync_lock_test_and_set(&m_lock, 1));
	#else
		#pragma message ("No Atomic TAS primitive available!")
	#endif
}
//----------------------------------------------------------------------------------------------
inline void Spinlock::Unlock(void)
{
	#if defined(__COMPILER_MSVC__)
		m_lock = 0;
	#elif defined(__COMPILER_GCC__)
		__sync_lock_release(&m_lock);
	#endif
}
//----------------------------------------------------------------------------------------------
