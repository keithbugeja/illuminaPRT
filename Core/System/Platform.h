//----------------------------------------------------------------------------------------------
//	Filename:	Platform.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#if !defined(__APPLE__)
#include <malloc.h>
#else
// #include <stdio.h>
#endif

#include <boost/timer.hpp>

//----------------------------------------------------------------------------------------------
//	Compiler detection.
//----------------------------------------------------------------------------------------------
#if defined(__APPLE_CC__)

	// Detected compiler :: GCC
	#define __COMPILER_APPLE_GCC__

	#include <stdint.h>
	#include <float.h>
	#include <limits.h>

#elif defined(__GNUC__)

	// Detected compiler :: GCC
	#define __COMPILER_GCC__

	#include <stdint.h>
	#include <float.h>
	#include <limits.h>
	#include <inttypes.h>

#elif defined(_MSC_VER)

	// Detected compiler :: Microsoft Visual C++ compiler
	#define __COMPILER_MSVC__

#else

	#error Compiler unsupported.

#endif

//----------------------------------------------------------------------------------------------
//	Platform detection.
//----------------------------------------------------------------------------------------------
#if (defined(_WIN64) || defined(_WIN32))

	// Detect OS :: Windows
	#define __PLATFORM_WINDOWS__
	
	#define WIN32_LEAN_AND_MEAN 

	// General include files for windows platform
	#include <windows.h>

	// Detect Architecture :: 32/64 bit platform
	#if defined(_WIN64)
		#define __ARCHITECTURE_X64__
	#else
		#define __ARCHITECTURE_X86__
	#endif

#elif defined(__linux__)

	// Detect OS :: Linux
	#define __PLATFORM_LINUX__
	#include <float.h>

	// Detect Architecture :: 32/64 bit platform
	#if (defined(__x86_64__))
		#define __ARCHITECTURE_X64__
	#else
		#define __ARCHITECTURE_X86__
	#endif

#elif (defined(__APPLE__) || defined(__MACOSX__) || defined(__APPLE_GCC__))

	// Detect OS :: OSX
	#define __PLATFORM_OSX__

	// Detect Architecture :: 32/64 bit platform
	#if (defined(__x86_64__))
		#define __ARCHITECTURE_X64__
	#else
		#define __ARCHITECTURE_X86__
	#endif

#else
	
	#error Platform unsupported.

#endif

//----------------------------------------------------------------------------------------------
//	Setup some compiler specific stuff
//----------------------------------------------------------------------------------------------
#if defined __COMPILER_APPLE_GCC__
	
	// Macro for a stronger inlining hint
	#define FORCEINLINE				inline __attribute__((always_inline))

	// Hint no function inlining
	#define NOINLINE				__attribute__((noinline))
	
	// Alignment macro for specifying 16-byte boundary alignment of types
	#define ALIGN_16				__attribute__ ((aligned (16)))

	// Define Int32 and Int64
	typedef uint32_t Int32;
	typedef uint64_t Int64;

	// Aligned malloc and free call ordinary malloc and free functions
	inline void* AlignedMalloc(size_t size, int boundary) 
	{
        return valloc(size);
		/* void *memory;
		posix_memalign(&memory, boundary, size);
		return memory; */
	}

	template<class T> void AlignedFree(T*& p) {
		free(p);
	}

#elif defined __COMPILER_GCC__
	
	// Macro for a stronger inlining hint
	#define FORCEINLINE				inline __attribute__((always_inline))

	// Hint no function inlining
	#define NOINLINE				__attribute__((noinline))

	// Alignment macro for specifying 16-byte boundary alignment of types
	#define ALIGN_16				__attribute__ ((aligned (16)))

	// Define Int32 and Int64
	typedef uint32_t Int32;
	typedef uint64_t Int64;

	// Aligned malloc and free call ordinary malloc and free functions
	inline void* AlignedMalloc(size_t size, int boundary) 
	{
		void *memory; 
		posix_memalign(&memory, boundary, size);
		return memory;
	}

	template<class T> void AlignedFree(T*& p) {
		free(p);
	}

#elif defined __COMPILER_MSVC__
	
	// Macro for a stronger inlining hint
	#define FORCEINLINE				__forceinline

	// Hint no function inlining
	#define NOINLINE				__declspec(noinline)

	// Alignment macro for specifying 16-byte boundary alignment of types
	#define ALIGN_16				__declspec(align(16))

	// Define Int32 and Int64
	typedef __int32 Int32;
	typedef __int64 Int64;

	// Aligned Malloc and Free, which allow user to specify alignment boundary
	inline void* AlignedMalloc(size_t size, int boundary) {
		return _aligned_malloc(size, boundary);
	}

	template<class T> void AlignedFree(T*& p) {
		_aligned_free(p);
	}

#endif

// MakeInt64 (Little-endian)
inline Int64 MakeInt64(Int32 hi, Int32 lo) {
	return ((Int64)(hi & 0xFFFFFFFF) | ((Int64)(lo & 0xFFFFFFFF) << 32));
}

// GetHiWord (Little-endian)
inline Int32 GetHiWord(Int64 value) {
	return (Int32)(value & 0xFFFFFFFF);
}

// GetLoWord (Little-endian)
inline Int32 GetLoWord(Int64 value) {
	return (Int32)((value >> 32) & 0xFFFFFFFF);
}

// Safe function for freeing memory
template<class T> void Safe_AlignedFree(T*& p)
{
	if (p) AlignedFree(p);
	p = NULL;
}

template<class T> void Safe_Delete(T*& p)
{
	if (p) delete p;
	p = NULL;
}

//----------------------------------------------------------------------------------------------
// SSE stuff
//----------------------------------------------------------------------------------------------

// Include SSE headers
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h> // SSE2
#include <pmmintrin.h> // SSE3

// #define SSE_ENABLED

// turn those verbose intrinsics into something readable.
#define loadps(mem)			_mm_load_ps((const float * const)(mem))
#define storess(ss,mem)		_mm_store_ss((float * const)(mem),(ss))
#define minss				_mm_min_ss
#define maxss				_mm_max_ss
#define minps				_mm_min_ps
#define maxps				_mm_max_ps
#define mulps				_mm_mul_ps
#define subps				_mm_sub_ps
#define rotatelps(ps)		_mm_shuffle_ps((ps),(ps), 0x39)	// a,b,c,d -> b,c,d,a
#define muxhps(low,high)	_mm_movehl_ps((low),(high))	// low{a,b,c,d}|high{e,f,g,h} = {c,d,g,h}

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class Platform
		{
		private:
			static boost::timer m_timer;

		public:
			static long long int GetCycles(void);
			static int GetProcessorCount(void);
			static double GetTime(void);
			static double ToSeconds(double);
		};
	}
}
//----------------------------------------------------------------------------------------------
