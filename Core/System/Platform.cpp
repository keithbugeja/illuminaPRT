//----------------------------------------------------------------------------------------------
//	Filename:	Platform.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <boost/thread/thread.hpp>

#include "System/Platform.h"
using namespace Illumina::Core;

#if (!defined(__COMPILER_APPLE_GCC__))
	#include <boost/chrono.hpp>
	using namespace boost::chrono;
#endif

//----------------------------------------------------------------------------------------------
boost::timer Platform::m_timer;
typedef boost::chrono::high_resolution_clock Clock;
//----------------------------------------------------------------------------------------------
double Platform::GetTime(void) 
{
	#if (defined(__COMPILER_APPLE_GCC__))
		return m_timer.elapsed();
	#else
		return Clock::now().time_since_epoch().count();
	#endif
}
//----------------------------------------------------------------------------------------------
long long int Platform::GetCycles(void)
{
	#if (defined(__COMPILER_MSVC__))
		return __rdtsc();
	#elif (defined(__COMPILER_GCC__) || defined(__COMPILER_APPLE_GCC__))
		register long long tsc asm("eax") = 0;
		asm volatile (".byte 15, 49" : : : "eax", "edx");
		return tsc;
	#else
		return 0;
	#endif
}
//----------------------------------------------------------------------------------------------
double Platform::ToSeconds(double p_fTimelapse)
{
	#if (defined(__COMPILER_APPLE_GCC__))
		return p_fTimelapse;
	#else
		return ((double)Clock::period::num / Clock::period::den) * p_fTimelapse;
	#endif
}
//----------------------------------------------------------------------------------------------
int Platform::GetProcessorCount(void) 
{
	return boost::thread::hardware_concurrency();
}
//----------------------------------------------------------------------------------------------
