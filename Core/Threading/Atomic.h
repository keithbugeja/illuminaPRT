//----------------------------------------------------------------------------------------------
//	Filename:	Atomic.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//  TODO:	Type sizes are based on the MSVC++ compiler definitions - have to revise classes and
//			make sure they don't break on other compilers under 64-bit compilation.
//----------------------------------------------------------------------------------------------
#pragma once

#include <utility>

#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>

#include "System/Platform.h"
#include "Threading/Spinlock.h"

namespace Illumina 
{
	namespace Core
	{
		class AtomicInt32
		{
		private:
			volatile long m_int32;

		public:
			AtomicInt32(long p_lValue) 
				: m_int32(p_lValue)
			{ }

			AtomicInt32(const AtomicInt32 &p_int32)
				: m_int32(p_int32.m_int32)
			{ }

			long operator++(void) {
				return _InterlockedIncrement(&m_int32);
			}

			long operator++(int) 
			{
				#if defined(__ARCHITECTURE_X64__)
					return _InterlockedExchangeAdd(&m_int32, 1);
				#else
					long *pInt32 = (long*)&m_int32, result;

					__asm {		
						xor ecx, ecx
						inc ecx
						mov eax, pInt32
						lock xadd dword ptr[eax], ecx
						mov result, ecx
					}
					
					return result;
				#endif				
			}

			long operator--(void) {
				return _InterlockedDecrement(&m_int32);
			}

			long operator--(int) 
			{
				#if defined(__ARCHITECTURE_X64__)
					return _InterlockedExchangeAdd(&m_int32, -1);
				#else
					long *pInt32 = (long*)&m_int32, result;

					__asm {		
						xor ecx, ecx
						dec ecx
						mov eax, pInt32
						lock xadd dword ptr[eax], ecx
						mov result, ecx
					}
					
					return result;
				#endif				
			}

			long operator+=(long p_lValue) {
				#if defined(__ARCHITECTURE_X64__)
					return _InterlockedAdd(&m_int32, p_lValue);
				#else
					long *pInt32 = (long*)&m_int32, result;

					__asm {
						mov ecx, p_lValue
						mov eax, pInt32
						lock xadd dword ptr[eax], ecx
						add ecx, p_lValue
						mov result, ecx
					}
					
					return result;
				#endif
			}

			long operator-=(long p_lValue) {
				#if defined(__ARCHITECTURE_X64__)
					return _InterlockedAdd(&m_int32, -p_lValue);
				#else
					long *pInt32 = (long*)&m_int32, result;

					__asm {		
						xor ecx, ecx
						sub ecx, p_lValue
						mov eax,  pInt32
						lock xadd dword ptr[eax], ecx
						add ecx, p_lValue
						mov result, ecx
					}
					
					return result;
				#endif
			}

			long operator=(long p_lValue) {
				return _InterlockedExchange(&m_int32, p_lValue);
			}

			operator long(void) { return m_int32; }
			
			inline long FetchAndAdd(long p_lIncrement)
			{
				#if defined(__ARCHITECTURE_X64__)
					return _InterlockedExchangeAdd(&m_int32, p_lIncrement);
				#else
					long *pInt32 = (long*)&m_int32, result;

					__asm {
						mov ecx, p_lIncrement
						mov	eax, pInt32
						lock xadd dword ptr[eax], ecx
						mov result, ecx
					}
					
					return result;
				#endif	
			}

			static inline long Add(long *p_pValue, long p_lIncrement) 
			{
				#if defined(__ARCHITECTURE_X64__)
					return _InterlockedAdd(p_pValue, p_lIncrement);
				#else
					long result;

					__asm {		
						mov ecx, p_lIncrement
						mov eax, p_pValue
						lock xadd DWORD PTR[eax], ecx
						add ecx, p_lIncrement
						mov result, ecx
					}
					
					return result;
				#endif
			}

			static inline long Increment(long *p_pValue) {
				return _InterlockedIncrement(p_pValue);
			}

			static inline long Decrement(long *p_pValue) {
				return _InterlockedDecrement(p_pValue);
			}

			static inline long Exchange(long *p_pValue, long p_lExchange) {
				return _InterlockedExchange(p_pValue, p_lExchange);
			}

			static inline long FetchAndAdd(long *p_pValue, long p_lIncrement) 
			{
				#if defined(__ARCHITECTURE_X64__)
					return _InterlockedExchangeAdd(p_pValue, p_lIncrement);
				#else
					long result;

					__asm {
						mov ecx, p_lIncrement
						mov	eax, p_pValue
						lock xadd dword ptr[eax], ecx
						mov result, ecx
					}
					
					return result;
				#endif	
			}

			static inline long CompareAndSwap(long *p_pDestination, long p_lExchange, long p_lComparand) 
			{
				#if defined(__ARCHITECTURE_X64__)
					return _InterlockedCompareExchange(p_pDestination, p_lExchange, p_lComparand);
				#else
					long result;

					__asm {
						mov eax, p_lComparand
						mov ecx, p_lExchange
						mov edx, p_pDestination
						lock cmpxchg dword ptr[edx], ecx
						mov result, eax
					}
				#endif
			}

			std::string ToString(void) const {
				return boost::str(boost::format("%d") % m_int32);
			}
		};

		#if defined(__ARCHITECTURE_X64__)

			class AtomicInt64
			{
			private:
				volatile long long m_int64;

			public:
				AtomicInt64(long long p_llValue) 
					: m_int64(p_llValue)
				{ }

				AtomicInt64(const AtomicInt64 &p_int64)
					: m_int64(p_int64.m_int64)
				{ }

				long long operator++(void) {
					return _InterlockedIncrement64(&m_int64);
				}

				long long operator++(int) {
					return _InterlockedExchangeAdd64(&m_int64, 1);
				}

				long long operator--(void) {
					return _InterlockedDecrement64(&m_int64);
				}
				
				long long operator--(int) {
					return _InterlockedExchangeAdd64(&m_int64, -1);
				}			

				long long operator+=(long long p_llValue) {
					return _InterlockedAdd64(&m_int64, p_llValue);
				}

				long long operator-=(long long p_llValue) {
					return _InterlockedAdd64(&m_int64, -p_llValue);
				}

				long long operator=(long long p_llValue) {
					return _InterlockedExchange64(&m_int64, p_llValue);
				}

				operator long long(void) { return m_int64; }
				
				inline long long FetchAndAdd(long long p_llIncrement) {
					return _InterlockedExchangeAdd64(&m_int64, p_llIncrement);
				}

				static inline long long Add(long long *p_pValue, long long p_llIncrement) {
					return _InterlockedAdd64(p_pValue, p_llIncrement);
				}

				static inline long long Increment(long long *p_pValue) {
					return _InterlockedIncrement64(p_pValue);
				}

				static inline long long Decrement(long long *p_pValue) {
					return _InterlockedDecrement64(p_pValue);
				}

				static inline long long Exchange(long long *p_pValue, long long p_llExchange) {
					return _InterlockedExchange64(p_pValue, p_llExchange);
				}

				static inline long long FetchAndAdd(long long *p_pValue, long long p_llIncrement) {
					return _InterlockedExchangeAdd64(p_pValue, p_llIncrement);
				}

				static inline long long CompareAndSwap(long long *p_pDestination, long long p_llExchange, long long p_llComparand) {
					return _InterlockedCompareExchange64(p_pDestination, p_llExchange, p_llComparand);
				}

				std::string ToString(void) const {
					return boost::str(boost::format("%d") % m_int64);
				}
			};

		#endif

		class Atomic
		{
		public:
			inline static bool CompareAndSet(void **p_pReference, void *p_pNewReference, void *p_pComparand) 
			{
				#if defined(__ARCHITECTURE_X64__) 
					return (long long)p_pComparand == _InterlockedCompareExchange64((long long*)p_pReference, (long long)p_pNewReference, (long long)p_pComparand);
				#else
					return (long)p_pComparand == _InterlockedCompareExchange((long*)p_pReference, (long)p_pNewReference, (long)p_pComparand);
				#endif
			}

			inline static void* CompareAndSwap(void **p_pReference, void *p_pNewReference, void *p_pComparand) 
			{
				#if defined(__ARCHITECTURE_X64__) 
					return (void*)_InterlockedCompareExchange64((long long*)p_pReference, (long long)p_pNewReference, (long long)p_pComparand);
				#else
					return (void*)_InterlockedCompareExchange((long*)p_pReference, (long)p_pNewReference, (long)p_pComparand);
				#endif
			}

			inline static Int32 CompareAndSwap(Int32 *p_pDestination, Int32 p_nExchange, Int32 p_nComparand) {
				return (Int32)_InterlockedCompareExchange((long*)p_pDestination, (long)p_nExchange, (long)p_nComparand);
			}

			inline static Int64 CompareAndSwap(Int64 *p_pDestination, Int64 p_nExchange, Int64 p_nComparand) {
				return _InterlockedCompareExchange64((long long*)p_pDestination, (long long)p_nExchange, (long long)p_nComparand);
			}

			inline static bool DoubleCompareAndSwap(Int32 *p_pDestination, Int32 p_nExchangeHi, Int32 p_nExchangeLo, Int32 *p_pComparandResult)
			{	
				long long exchange = (long long)MakeInt64(p_nExchangeHi, p_nExchangeLo),
					comparand = (long long)MakeInt64(p_pComparandResult[0], p_pComparandResult[1]);

				long long result = _InterlockedCompareExchange64((__int64 volatile*)p_pDestination, (__int64)exchange, (__int64)comparand);
				
				if (result == comparand) 
					return true;

				Int64 i64Result = (Int64)result;

				p_pComparandResult[0] = GetHiWord(i64Result);
				p_pComparandResult[1] = GetLoWord(i64Result);

				return false;
			}

			#if defined(__ARCHITECTURE_X64__) 
			inline static bool DoubleCompareAndSwap(Int64 *p_pDestination, Int64 p_nExchangeHi, Int64 p_nExchangeLo, Int64 *p_pComparandResult)
			{
				// Note that _InterlockedCompareExchange128 requires data to be aligned on 16-byte boundaries!
				return (_InterlockedCompareExchange128(
						(__int64 volatile*)p_pDestination,
						(__int64)p_nExchangeHi, (__int64)p_nExchangeLo,
						(__int64*)p_pComparandResult) != 0);
			}
			#endif
		};
	}
}