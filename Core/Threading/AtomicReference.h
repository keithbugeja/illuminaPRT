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
#include "Threading/Atomic.h"

namespace Illumina 
{
	namespace Core
	{
		class AtomicReference
		{
		public:
			inline static void* CompareAndSwap(void **p_pReference, void *p_pNewReference, void *p_pComparand)
			{
				#if defined(__ARCHITECTURE_X64__) 
					return (void*)AtomicInt64::CompareAndSwap((Int64*)p_pReference, (Int64)p_pNewReference, (Int64)p_pComparand);
				#else
					return (void*)AtomicInt32::CompareAndSwap((Int32*)p_pReference, (Int32)p_pNewReference, (Int32)p_pComparand);
				#endif
			}

			inline static bool CompareAndSet(void **p_pReference, void *p_pNewReference, void *p_pComparand)
			{
				#if defined(__ARCHITECTURE_X64__) 
					return (Int64)p_pComparand == AtomicInt64::CompareAndSwap((Int64*)p_pReference, (Int64)p_pNewReference, (Int64)p_pComparand);
				#else
					return (Int32)p_pComparand == AtomicInt32::CompareAndSwap((Int32*)p_pReference, (Int32)p_pNewReference, (Int32)p_pComparand);
				#endif
			}
		};

		template<class T> 
		struct StampedReference32
		{
			volatile Int32 Stamp;
			volatile T* Reference;

			StampedReference32(Int32 p_stamp, T *p_reference)
				: Stamp(p_stamp)
				, Reference(p_reference)
			{ }

			inline Int32* GetAddress(void) { return (Int32*)(&Stamp); }
		};

		template<class T> 
		struct StampedReference64
		{
			volatile Int64 Stamp;
			volatile T* Reference;

			StampedReference64(Int64 p_stamp, T *p_reference)
				: Stamp(p_stamp)
				, Reference(p_reference)
			{ }
			
			inline Int64* GetAddress(void) { return (Int64*)(&Stamp); }
		};

		template<class T>
		class AtomicStampedReference32
		{
			ALIGN_16 StampedReference32<T> m_stampedReference;

		public:
			AtomicStampedReference32(void)
				: m_stampedReference(0, NULL) 
			{ }

			AtomicStampedReference32(Int32 p_nStamp, T *p_pReference)
				: m_stampedReference(p_nStamp, p_pReference)
			{ }

			bool TryStamp(T *p_pReference, Int32 p_nStamp) 
			{
				StampedReference32<T> comparand(m_stampedReference.Stamp, p_pReference);
				return Atomic::DoubleCompareAndSwap(m_stampedReference.GetAddress(), (Int32)p_pReference, p_nStamp, comparand.GetAddress());
			}

			bool CompareAndSet(T *p_pExpectedReference, T *p_pNewReference, Int32 p_nExpectedStamp, Int32 p_nNewStamp) 
			{
				StampedReference32<T> comparand(p_nExpectedStamp, p_pExpectedReference);
				return Atomic::DoubleCompareAndSwap(m_stampedReference.GetAddress(), (Int32)p_pNewReference, p_nNewStamp, comparand.GetAddress());
			}

			T* Get(Int32 *p_pnStamp) const
			{
				*p_pnStamp = m_stampedReference.Stamp;
				return (T*)(m_stampedReference.Reference);
			}

			T* GetReference(void) const {
				return (T*)(m_stampedReference.Reference);
			}

			Int32 GetStamp(void) const {
				return m_stampedReference.Stamp;
			}

			void Set(T *p_pReference, Int32 p_nStamp) 
			{
				m_stampedReference.Stamp = p_nStamp;
				m_stampedReference.Reference = p_pReference;
			}
		};

		template<class T>
		class AtomicStampedReference64
		{
			ALIGN_16 StampedReference64<T> m_stampedReference;

		public:
			AtomicStampedReference64(void)
				: m_stampedReference(0, NULL) 
			{ }

			AtomicStampedReference64(Int64 p_nStamp, T *p_pReference)
				: m_stampedReference(p_nStamp, p_pReference)
			{ }

			bool TryStamp(T *p_pReference, Int64 p_nStamp) 
			{
				StampedReference64<T> comparand(m_stampedReference.Stamp, p_pReference);
				return Atomic::DoubleCompareAndSwap(m_stampedReference.GetAddress(), (Int64)p_pReference, p_nStamp, comparand.GetAddress());
			}

			bool CompareAndSet(T *p_pExpectedReference, T *p_pNewReference, Int64 p_nExpectedStamp, Int64 p_nNewStamp) 
			{
				StampedReference64<T> comparand(p_nExpectedStamp, p_pExpectedReference);
				return Atomic::DoubleCompareAndSwap(m_stampedReference.GetAddress(), (Int64)p_pNewReference, p_nNewStamp, comparand.GetAddress());
			}

			T* Get(Int64 *p_pnStamp) const
			{
				*p_pnStamp = m_stampedReference.Stamp;
				return (T*)(m_stampedReference.Reference);
			}

			T* GetReference(void) const {
				return (T*)(m_stampedReference.Reference);
			}

			Int64 GetStamp(void) const {
				return m_stampedReference.Stamp;
			}

			void Set(T *p_pReference, Int64 p_pnStamp) 
			{
				m_stampedReference.Stamp = p_pnStamp;
				m_stampedReference.Reference = p_pReference;
			}
		};

		// Since we cannot typedef templates, create a new templated class which 
		// hides the current platform implementation
		#if defined(__ARCHITECTURE_X64__)
			template<class T> 
			class AtomicStampedReference 
				: public AtomicStampedReference64<T>
			{ 
			public:
				AtomicStampedReference(void)
					: AtomicStampedReference64<T>()
				{ }

				AtomicStampedReference(Int64 p_nStamp, T *p_pReference = NULL)
					: AtomicStampedReference64<T>(p_nStamp, p_pReference)
				{ }
			};
		#else
			template<class T> 
			class AtomicStampedReference 
				: public AtomicStampedReference32<T>
			{
			public: 
				AtomicStampedReference(void)
					: AtomicStampedReference32<T>()
				{ }

				AtomicStampedReference(Int32 p_nStamp, T *p_pReference = NULL)
					: AtomicStampedReference32<T>(p_nStamp, p_pReference)
				{ }
			};
		#endif
	}
}