//----------------------------------------------------------------------------------------------
//	Filename:	Platform.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <malloc.h>
#include <boost/timer.hpp>

#if (defined(_WIN64) || defined(_WIN32))
	// Detect OS :: Windows
	#define __PLATFORM_WINDOWS__

	// General include files for windows platform
	#include <windows.h>

	// Are we using the Visual C++ compiler?
	#if defined(_MSC_VER)
		#include <intrin.h>

		// Detected compiler :: Microsoft Visual C++ compiler
		#define __COMPILER_MSVC__

		// Detect Architecture :: 32/64 bit platform
		#if defined(_WIN64)
			#define __ARCHITECTURE_X64__

			// SSE functionality :: Enable SSE
			#define SSE_ENABLED
		#else
			#define __ARCHITECTURE_X86__
		#endif

		// Define Int32 and Int64
		typedef __int32 Int32;
		typedef __int64 Int64;

		// Alignment macro for specifying 16-byte boundary alignment of types
		#if defined(SSE_ENABLED)
			#include <xmmintrin.h>
			#include <emmintrin.h>
			#include <mmintrin.h>

			#define ALIGN_16 _declspec(align(16))
		#else
			#define ALIGN_16
		#endif

		// Aligned Malloc and Free, which allow user to specify alignment boundary
		inline void* AlignedMalloc(size_t size, int boundary) {
			return _aligned_malloc(size, boundary);
		}

		template<class T> void AlignedFree(T*& p) {
			_aligned_free(p);
		}
	#else
		#if defined(__GNUC__)
			#include <inttypes.h>

			// Detected compiler :: GCC
			#define __COMPILER_GCC__

			// Detect Architecture :: 32/64 bit platform
			#if (defined(__x86_64) || defined(__x86_64__))
				#define __ARCHITECTURE_X64__
			#else
				#define __ARCHITECTURE_X86__
			#endif

			#define ALIGN_16

			// Define Int32 and Int64
			typedef uint32_t Int32;
			typedef uint64_t Int64;
		#endif

		// Aligned malloc and free call ordinary malloc and free functions
		inline void* AlignedMalloc(size_t size, int boundary) {
			return malloc(size);
		}

		template<class T> void AlignedFree(T*& p) {
			free(p);
		}
	#endif
#else
	#if (defined(__linux) || defined(__linux__))

		// Detect OS :: Linux
		#define __PLATFORM_LINUX__

		#if defined(__GNUC__)
			#include <inttypes.h>

			// Detected compiler :: GCC
			#define __COMPILER_GCC__

			// Detect Architecture :: 32/64 bit platform
			#if (defined(__x86_64) || defined(__x86_64__))
					#define __ARCHITECTURE_X64__
			#else
					#define __ARCHITECTURE_X86__
			#endif

			#define ALIGN_16

			// Define Int32 and Int64
			typedef uint32_t Int32;
			typedef uint64_t Int64;

			// Aligned malloc and free call ordinary malloc and free functions
			inline void* AlignedMalloc(size_t size, int boundary) {
				return malloc(size);
			}

			template<class T> void AlignedFree(T*& p) {
				free(p);
			}
		#endif
	#else
		#define __COMPILER_UNKNOWN__
		#define __PLATFORM_UNKNOWN__
		#define __ARCHITECTURE_X86__
		#define ALIGN_16

		// Aligned malloc and free call ordinary malloc and free functions
		inline void* AlignedMalloc(size_t size, int boundary) {
			return malloc(size);
		}

		template<class T> void AlignedFree(T*& p) {
			free(p);
		}
	#endif
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
		};
	}
}
//----------------------------------------------------------------------------------------------
