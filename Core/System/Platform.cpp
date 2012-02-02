//----------------------------------------------------------------------------------------------
//	Filename:	Platform.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <boost/thread/thread.hpp>
#include <boost/chrono.hpp>

#include "System/Platform.h"

using namespace Illumina::Core;
using namespace boost::chrono;

//----------------------------------------------------------------------------------------------
boost::timer Platform::m_timer;
typedef boost::chrono::high_resolution_clock Clock;
//----------------------------------------------------------------------------------------------
double Platform::GetTime(void) 
{
	//return m_timer.elapsed();
	return Clock::now().time_since_epoch().count();
}
//----------------------------------------------------------------------------------------------
long long int Platform::GetCycles(void)
{
	#if (defined(__COMPILER_MSVC__))
		return __rdtsc();
	#elif (defined(__COMPILER_GCC__))
		register long long tsc asm("eax");
		asm volatile (".byte 15, 49" : : : "eax", "edx");
		return tsc;
	else
		return 0;
	#endif
}
//----------------------------------------------------------------------------------------------
double Platform::ToSeconds(double p_fTimelapse)
{
	return ((double)Clock::period::num / Clock::period::den) * p_fTimelapse;
}
//----------------------------------------------------------------------------------------------
int Platform::GetProcessorCount(void) 
{
	return boost::thread::hardware_concurrency();
}
//----------------------------------------------------------------------------------------------
