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
	#if (defined(__PLATFORM_WINDOWS__) && defined(__COMPILER_MSVC__))
		return (!m_lock && !_interlockedbittestandset(&m_lock, 0));
	#endif
}
//----------------------------------------------------------------------------------------------
inline void Spinlock::Lock(void)
{
	#if (defined(__PLATFORM_WINDOWS__) && defined(__COMPILER_MSVC__))
		int backoff = 0, index;

		do 
		{
			backoff += m_backoff;

			while(m_lock) 
			{
				for (index = 0; index < backoff; index++) 
					_mm_pause(); 
			}
		} while(_interlockedbittestandset(&m_lock, 0));
	#endif
}
//----------------------------------------------------------------------------------------------
inline void Spinlock::Unlock(void)
{
	m_lock = 0;
}
//----------------------------------------------------------------------------------------------
