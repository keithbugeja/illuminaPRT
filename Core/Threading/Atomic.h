//----------------------------------------------------------------------------------------------
//	Filename:	Atomic.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <utility>

#include <boost/format.hpp>
#include <boost/interprocess/detail/atomic.hpp>

#include "System/Platform.h"
#include "Threading/Spinlock.h"

namespace Illumina
{
	namespace Core
	{
		class AtomicInt32
		{
		private:
			volatile Int32 m_int32;

		public:
			AtomicInt32(Int32 p_nValue)
				: m_int32(p_nValue)
			{ }

			AtomicInt32(const AtomicInt32 &p_int32)
				: m_int32(p_int32.m_int32)
			{ }

			Int32 operator++(void) {
                return boost::interprocess::detail::atomic_inc32(reinterpret_cast<volatile uint32_t*>(&m_int32));
			}

			// Incorrect semantics
			Int32 operator++(int)
			{
                #if defined(__COMPILER_GCC__)
                    return __sync_fetch_and_add(&m_int32, 1);
				#elif defined(__COMPILER_MSVC__)
					#if defined(__ARCHITECTURE_X64__)
						return _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&m_int32), 1);
					#else
						long *pInt32 = reinterpret_cast<long*>(&m_int32), result;

						__asm {
							xor ecx;
							inc ecx;
							mov	eax, pInt32
							lock xadd dword ptr[eax], ecx
							mov result, ecx
						}

						return result;
					#endif
				#endif
			}

			Int32 operator--(void) {
				return boost::interprocess::detail::atomic_dec32(reinterpret_cast<volatile uint32_t*>(&m_int32));
			}

			Int32 operator--(int)
			{
                #if defined(__COMPILER_GCC__)
                    return __sync_fetch_and_sub(&m_int32, 1);
				#elif defined(__COMPILER_MSVC__)
					#if defined(__ARCHITECTURE_X64__)
						return _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&m_int32), -1);
					#else
						long *pInt32 = reinterpret_cast<long*>(&m_int32), result;

						__asm {
							xor ecx;
							inc ecx;
							mov	eax, pInt32
							lock xsub dword ptr[eax], ecx
							mov result, ecx
						}

						return result;
					#endif
				#endif
			}

			Int32 operator+=(Int32 p_nValue)
			{
				#if defined(__COMPILER_GCC__)
                    return __sync_add_and_fetch(&m_int32, p_nValue);
				#elif defined(__COMPILER_MSVC__)
					#if defined(__ARCHITECTURE_X64__)
						return _InterlockedAdd(reinterpret_cast<volatile long*>(&m_int32), p_nValue);
					#else
						Int32 *pInt32 = (Int32*)&m_int32, result;

						__asm {
							mov ecx, p_nValue
							mov eax, pInt32
							lock xadd dword ptr[eax], ecx
							add ecx, p_lValue
							mov result, ecx
						}

						return result;
					#endif
				#endif
			}

			Int32 operator-=(Int32 p_nValue)
			{
				#if defined(__COMPILER_GCC__)
                    return __sync_sub_and_fetch(&m_int32, p_nValue);
				#elif defined(__COMPILER_MSVC__)
					#if defined(__ARCHITECTURE_X64__)
						return _InterlockedAdd(reinterpret_cast<volatile long*>(&m_int32), -p_nValue);
					#else
						long *pInt32 = reinterpret_cast<volatile long*>(&m_int32), result;

						__asm {
							xor ecx, ecx
							sub ecx, p_nValue
							mov eax,  pInt32
							lock xadd dword ptr[eax], ecx
							add ecx, p_lValue
							mov result, ecx
						}

						return result;
					#endif
                #endif
			}

			Int32 operator=(Int32 p_nValue)
			{
				boost::interprocess::detail::atomic_write32(reinterpret_cast<volatile uint32_t*>(&m_int32), p_nValue);
				return p_nValue;
			}

			operator long(void) {
			    return (long)m_int32;
            }

			inline Int32 FetchAndAdd(Int32 p_nIncrement)
			{
				#if defined(__COMPILER_GCC__)
                    return __sync_fetch_and_add(&m_int32, p_nIncrement);
				#elif defined(__COMPILER_MSVC__)
					#if defined(__ARCHITECTURE_X64__)
						return _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&m_int32), p_nIncrement);
					#else
						long *pInt32 = reinterpret_cast<long*>(&m_int32), result;

						__asm {
							mov ecx, p_nIncrement
							mov	eax, pInt32
							lock xadd dword ptr[eax], ecx
							mov result, ecx
						}

						return result;
					#endif
				#endif
			}

			static inline Int32 Add(Int32 *p_pValue, Int32 p_nIncrement)
			{
				#if defined(__COMPILER_GCC__)
					return __sync_add_and_fetch(p_pValue, p_nIncrement);
				#elif defined(__COMPILER_MSVC__)
					#if defined(__ARCHITECTURE_X64__)
						return _InterlockedAdd(reinterpret_cast<long *>(p_pValue), p_nIncrement);
					#else
						long result;

						__asm {
							mov ecx, p_nIncrement
							mov eax, p_pValue
							lock xadd DWORD PTR[eax], ecx
							add ecx, p_lIncrement
							mov result, ecx
						}

						return result;
					#endif
				#endif
			}

			static inline Int32 Increment(Int32 *p_pValue) {
				return boost::interprocess::detail::atomic_inc32(reinterpret_cast<uint32_t*>(p_pValue));
			}

			static inline Int32 Decrement(Int32 *p_pValue) {
				return boost::interprocess::detail::atomic_dec32(reinterpret_cast<uint32_t*>(p_pValue));
			}

			static inline Int32 Exchange(Int32 *p_pValue, Int32 p_nExchange)
			{
				#if defined(__COMPILER_MSVC__)
					return _InterlockedExchange(reinterpret_cast<long *>(p_pValue), p_nExchange);
				#elif defined(__COMPILER_GCC__)
					Int32 nValue;

					do { nValue = *p_pValue; }
					while (nValue == __sync_val_compare_and_swap(p_pValue, nValue, p_nExchange));

					return nValue;
				#endif
			}

			static inline Int32 FetchAndAdd(Int32 *p_pValue, Int32 p_nIncrement)
			{
				#if defined(__COMPILER_GCC__)
					return __sync_fetch_and_add(p_pValue, p_nIncrement);
				#elif defined(__COMPILER_MSVC__)
					#if defined(__ARCHITECTURE_X64__)
						return _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(p_pValue), p_nIncrement);
					#else
						long result;

						__asm {
							mov ecx, p_nIncrement
							mov	eax, p_pValue
							lock xadd dword ptr[eax], ecx
							mov result, ecx
						}

						return result;
					#endif
				#endif
			}

			static inline Int32 CompareAndSwap(Int32 *p_pDestination, Int32 p_nExchange, Int32 p_nComparand)
			{
				#if defined(__COMPILER_GCC__)
					return __sync_val_compare_and_swap(p_pDestination, p_nComparand, p_nExchange);
				#elif defined(__COMPILER_MSVC__)
					#if defined(__ARCHITECTURE_X64__)
						return _InterlockedCompareExchange(reinterpret_cast<long *>(p_pDestination), p_nExchange, p_nComparand);
					#else
						Int32 result;

						__asm {
							mov eax, p_nComparand
								mov ecx, p_nExchange
								mov edx, p_pDestination
								lock cmpxchg dword ptr[edx], ecx
								mov result, eax
						}
					#endif
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
				volatile Int64 m_int64;

			public:
				AtomicInt64(Int64 p_nValue)
					: m_int64(p_nValue)
				{ }

				AtomicInt64(const AtomicInt64 &p_int64)
					: m_int64(p_int64.m_int64)
				{ }

				Int64 operator++(void)
				{
					#if defined(__COMPILER_MSVC__)
						return _InterlockedIncrement64(&m_int64);
					#elif defined(__COMPILER_GCC__)
						return __sync_add_and_fetch(&m_int64, 1);
					#endif
				}

				Int64 operator++(int)
				{
					#if defined(__COMPILER_MSVC__)
						return _InterlockedExchangeAdd64(&m_int64, 1);
					#elif defined(__COMPILER_GCC__)
						return __sync_fetch_and_add(&m_int64, 1);
					#endif
				}

				Int64 operator--(void)
				{
					#if defined(__COMPILER_MSVC__)
						return _InterlockedDecrement64(&m_int64);
					#elif defined(__COMPILER_GCC__)
						return __sync_sub_and_fetch(&m_int64, 1);
					#endif
				}

				Int64 operator--(int)
				{
					#if defined(__COMPILER_MSVC__)
						return _InterlockedExchangeAdd64(&m_int64, -1);
					#elif defined(__COMPILER_GCC__)
						return __sync_fetch_and_sub(&m_int64, 1);
					#endif
				}

				Int64 operator+=(Int64 p_nValue)
				{
					#if defined(__COMPILER_MSVC__)
						return _InterlockedAdd64(&m_int64, p_nValue);
					#elif defined(__COMPILER_GCC__)
						return __sync_add_and_fetch(&m_int64, p_nValue);
					#endif
				}

				Int64 operator-=(Int64 p_nValue)
				{
					#if defined(__COMPILER_MSVC__)
						return _InterlockedAdd64(&m_int64, -p_nValue);
					#elif defined(__COMPILER_GCC__)
						return __sync_sub_and_fetch(&m_int64, p_nValue);
					#endif
				}

				Int64 operator=(Int64 p_nValue)
				{
					#if defined(__COMPILER_MSVC__)
						_InterlockedExchange64(&m_int64, p_nValue);
						return m_int64;
					#elif defined(__COMPILER_GCC__)
						while(!__sync_bool_compare_and_swap(&m_int64, m_int64, p_nValue));
						return m_int64;
					#endif
				}

				AtomicInt64& operator=(const AtomicInt64 &p_int64)
				{
				    *this = p_int64.m_int64;
				    return *this;
				}

				operator long long(void) {
				    return (long long)m_int64;
                }

                operator unsigned long long(void) {
                    return (unsigned long long)m_int64;
                }

				inline Int64 FetchAndAdd(Int64 p_nIncrement)
				{
					#if defined(__COMPILER_MSVC__)
						return _InterlockedExchangeAdd64(&m_int64, p_nIncrement);
					#elif defined(__COMPILER_GCC__)
						return __sync_fetch_and_add(&m_int64, p_nIncrement);
					#endif
				}

				static inline Int64 Add(Int64 *p_pValue, Int64 p_nIncrement)
				{
					#if defined(__COMPILER_MSVC__)
						return _InterlockedAdd64(p_pValue, p_nIncrement);
					#elif defined(__COMPILER_GCC__)
						return __sync_add_and_fetch(p_pValue, p_nIncrement);
					#endif
				}

				static inline Int64 Increment(Int64 *p_pValue)
				{
					#if defined(__COMPILER_MSVC__)
						return _InterlockedIncrement64(p_pValue);
					#elif defined(__COMPILER_GCC__)
						return __sync_add_and_fetch(p_pValue, 1);
					#endif
				}

				static inline Int64 Decrement(Int64 *p_pValue)
				{
					#if defined(__COMPILER_MSVC__)
						return _InterlockedDecrement64(p_pValue);
					#elif defined(__COMPILER_GCC__)
						return __sync_sub_and_fetch(p_pValue, 1);
					#endif
				}

				static inline Int64 Exchange(Int64 *p_pValue, Int64 p_nExchange)
				{
					#if defined(__COMPILER_MSVC__)
						return _InterlockedExchange64(p_pValue, p_nExchange);
					#else
						Int64 nValue;

						do { nValue = *p_pValue; }
						while (nValue == __sync_val_compare_and_swap(p_pValue, nValue, p_nExchange));

						return nValue;
					#endif
				}

				static inline Int64 FetchAndAdd(Int64 *p_pValue, Int64 p_nIncrement)
				{
					#if defined(__COMPILER_MSVC__)
						return _InterlockedExchangeAdd64(p_pValue, p_nIncrement);
					#elif defined(__COMPILER_GCC__)
						return __sync_fetch_and_add(p_pValue, p_nIncrement);
					#endif
				}

				static inline Int64 CompareAndSwap(Int64 *p_pDestination, Int64 p_nExchange, Int64 p_nComparand)
				{
					#if defined(__COMPILER_MSVC__)
						return _InterlockedCompareExchange64(p_pDestination, p_nExchange, p_nComparand);
					#elif defined(__COMPILER_GCC__)
						return __sync_val_compare_and_swap(p_pDestination, p_nComparand, p_nExchange);
					#endif
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
				#if defined(__COMPILER_MSVC__)
					#if defined(__ARCHITECTURE_X64__)
						return (Int64)p_pComparand == _InterlockedCompareExchange64((long long*)p_pReference, (long long)p_pNewReference, (long long)p_pComparand);
					#else
						return (Int32)p_pComparand == _InterlockedCompareExchange((long*)p_pReference, (long)p_pNewReference, (long)p_pComparand);
					#endif
                #elif defined(__COMPILER_GCC__)
					#if defined(__ARCHITECTURE_X64__)
						return __sync_bool_compare_and_swap((Int64*)p_pReference, (Int64)p_pComparand, (Int64) p_pNewReference);
					#else
						return __sync_bool_compare_and_swap((Int32*)p_pReference, (Int32)p_pComparand, (Int32)p_pNewReference);
					#endif
				#endif
			}

			inline static void* CompareAndSwap(void **p_pReference, void *p_pNewReference, void *p_pComparand)
			{
				#if defined(__COMPILER_MSVC__)
					#if defined(__ARCHITECTURE_X64__)
						return (void*)_InterlockedCompareExchange64((long long*)p_pReference, (long long)p_pNewReference, (long long)p_pComparand);
					#else
						return (void*)_InterlockedCompareExchange((long*)p_pReference, (long)p_pNewReference, (long)p_pComparand);
					#endif
                #elif defined(__COMPILER_GCC__)
					#if defined(__ARCHITECTURE_X64__)
						return (void*)__sync_val_compare_and_swap((Int64*)p_pReference, (Int64)p_pComparand, (Int64)p_pNewReference);
					#else
						return (void*)__sync_val_compare_and_swap((Int32*)p_pReference, (Int32)p_pComparand, (Int32)p_pNewReference);
					#endif
				#endif
			}

			inline static Int32 CompareAndSwap(Int32 *p_pDestination, Int32 p_nExchange, Int32 p_nComparand) {
				#if defined(__COMPILER_MSVC__)
					return (Int32)_InterlockedCompareExchange((long*)p_pDestination, (long)p_nExchange, (long)p_nComparand);
                #elif defined(__COMPILER_GCC__)
					return __sync_val_compare_and_swap(p_pDestination, p_nComparand, p_nExchange);
				#endif
			}

			inline static Int64 CompareAndSwap(Int64 *p_pDestination, Int64 p_nExchange, Int64 p_nComparand) {
				#if defined(__COMPILER_MSVC__)
					return _InterlockedCompareExchange64((long long*)p_pDestination, (long long)p_nExchange, (long long)p_nComparand);
                #elif defined(__COMPILER_GCC__)
					return __sync_val_compare_and_swap(p_pDestination, p_nComparand, p_nExchange);
				#endif
			}

			inline static bool DoubleWidthCompareAndSwap(Int32 *p_pDestination, Int32 p_nExchangeHi, Int32 p_nExchangeLo, Int32 *p_pComparandResult)
			{
				long long exchange = (long long)MakeInt64(p_nExchangeHi, p_nExchangeLo),
					comparand = (long long)MakeInt64(p_pComparandResult[0], p_pComparandResult[1]);

				#if defined(__COMPILER_MSVC__)
					long long result = _InterlockedCompareExchange64((Int64 volatile*)p_pDestination, (Int64)exchange, (Int64)comparand);
                #elif defined(__COMPILER_GCC__)
					long long result = __sync_val_compare_and_swap((Int64*)p_pDestination, (Int64)comparand, (Int64)exchange);
				#endif

				if (result == comparand)
					return true;

				Int64 i64Result = (Int64)result;

				p_pComparandResult[0] = GetHiWord(i64Result);
				p_pComparandResult[1] = GetLoWord(i64Result);

				return false;
			}

			#if defined(__ARCHITECTURE_X64__)
				inline static bool DoubleWidthCompareAndSwap(Int64 *p_pDestination, Int64 p_nExchangeHi, Int64 p_nExchangeLo, Int64 *p_pComparandResult)
				{
					#if defined(__COMPILER_MSVC__)
						// Note that _InterlockedCompareExchange128 requires data to be aligned on 16-byte boundaries!
						return (_InterlockedCompareExchange128(
							(Int64 volatile*)p_pDestination,
							(Int64)p_nExchangeHi, (Int64)p_nExchangeLo,
							(Int64*)p_pComparandResult) != 0);
					#elif defined(__COMPILER_GCC__)
                        Int64 result;

                        asm __volatile__ (
                            "movq %2, %%rbx;"
                            "movq %3, %%rcx;"
                            "movq %4, %%rsi;"
                            "movq 0(%%esi), %%rax;"
                            "movq 8(%%esi), %%rdx;"
                            "movq %1, %%rsi;"
                            "lock cmpxchg16b (%%rsi);"
                            "jz   1f;"
                            "movq %3, %%rsi;"
                            "movq %%rax, 0(%%rsi);"
                            "movq %%rdx, 8(%%rsi);"
                        "1:  movq %0, %0;"
                            "setz %b0;"
                            : "=&a" (result)
                            : "m" (p_pDestination), "m" (p_nExchangeHi), "m" (p_nExchangeLo), "m" (p_pComparandResult)
                            : "cc", "memory", "rbx", "rcx", "rdx", "rsi"
                        );

                        return result;
					#endif
				}
			#endif
		};
	}
}
