//----------------------------------------------------------------------------------------------
//	Filename:	List.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/shared_ptr.hpp>
#include <fstream>

#include "System/Platform.h"
#include "Threading/Spinlock.h"
#include "Threading/Atomic.h"

namespace Illumina 
{
	namespace Core
	{
		// TODO:
		//	Base all lists on a common interface

		template<class T>
		class WaitFreeList
		{
		protected:
			struct WaitFreeListNode
			{
				WaitFreeListNode *pNext;
				T *pRecordList;
			};

			WaitFreeListNode m_baseNode;

		protected:
			Int64 m_nPageCapacity,
				  m_nRecordIndex,
				  m_nRecordCount,
				  m_nSize;

		public:
			WaitFreeList(int p_nPageCapacity = 100)
				: m_nPageCapacity(p_nPageCapacity)
				, m_nRecordIndex(0)
				, m_nRecordCount(0)
				, m_nSize(0)
			{
				m_baseNode.pNext = NULL;
				m_baseNode.pRecordList = new T[m_nPageCapacity];
			}

			inline const T& operator[](size_t p_index) const 
			{
				if (p_index < (size_t)m_nSize)
				{
					Int64 blockIndex = p_index / m_nPageCapacity,
						blockPosition = p_index % m_nPageCapacity;

					WaitFreeListNode *pTail;

					for (pTail = &m_baseNode; blockIndex > 0; 
						 pTail = pTail->pNext, blockIndex--);

					return pTail->pRecordList[blockPosition];
				}
			}

			inline T& operator[](size_t p_index) 
			{
				if (p_index < (size_t)m_nSize)
				{
					Int64 blockIndex = p_index / m_nPageCapacity,
						blockPosition = p_index % m_nPageCapacity;

					WaitFreeListNode *pTail;

					for (pTail = &m_baseNode; blockIndex > 0; 
						 pTail = pTail->pNext, blockIndex--);

					return pTail->pRecordList[blockPosition];
				}
			}

			inline size_t Size(void) const 
			{
				return (size_t)m_nSize;
			}

			void PushBack(const T &p_obj) 
			{
				// Increment record index atomically
				Int64 currentIndex = Illumina::Core::AtomicInt64::FetchAndAdd(&m_nRecordIndex, 1);
				
				// Get block index and block position
				Int64 blockIndex = currentIndex / m_nPageCapacity,
					blockPosition = currentIndex % m_nPageCapacity;

				// Find block for insert
				WaitFreeListNode *pTail; 
				Int64 blockCount = 0;

				for (pTail = &m_baseNode; pTail->pNext != NULL && blockCount < blockIndex;
						pTail = pTail->pNext, blockCount++);

				if (blockIndex > blockCount) 
				{
					for (Int64 i = 0; i < blockIndex - blockCount; i++) 
					{
						if (pTail->pNext == NULL)
						{
							WaitFreeListNode *pNode = new WaitFreeListNode();
							pNode->pRecordList = new T[m_nPageCapacity];
							pNode->pNext = NULL;

							if (pTail->pNext == (WaitFreeListNode*)Illumina::Core::AtomicInt64::CompareAndSwap((Int64*)&(pTail->pNext), (Int64)pNode, (Int64)NULL))
							{
								delete[] pNode->pRecordList;
								delete pNode;
							}
						}

						pTail = pTail->pNext;						
					}
				}

				// Insert record
				pTail->pRecordList[blockPosition] = p_obj;

				// Increment record count
				Illumina::Core::AtomicInt64::Add(&m_nRecordCount, 1);
				
				// Update size if record count and record index match
				Int64 value = m_nRecordIndex;
				if (m_nRecordCount == value) m_nSize = value;
			}
		};

		// #define __DEBUG_LIST__
		#if defined(__DEBUG_LIST__)
			template<class T> 
			class List
			{
			protected:
				std::vector<T> m_list;
				Spinlock m_lock;

			public:			
				List(int p_nCapacity = 10, int p_nGrowth = 5)
				{
				}

				~List(void) 
				{
				}

				inline const T& operator[](size_t p_index) const {
					return m_list[p_index];
				}

				inline T& operator[](size_t p_index) {
					return m_list[p_index];
				}

				inline size_t Capacity() const {
					return m_list.capacity();
				}

				inline size_t Size() const {
					return m_list.size();
				}
			
				inline bool IsEmpty() const {
					return (m_list.size() == 0);
				}
			
				inline T& At(size_t p_index) 
				{
					return m_list[p_index];
				}

				inline const T& At(size_t p_index) const 
				{
					return m_list[p_index];
				}

				inline T& Front(void) 
				{
					return m_list.front();
				}
						
				inline const T& Front(void) const 
				{
					return m_list.front();
				}

				inline T& Back(void) 
				{
					return m_list.back();
				}

				inline const T& Back(void) const 
				{
					return m_list.back();
				}

				void Clear(void) 
				{
					m_list.clear();
				}

				void PushBack(const T &p_obj) 
				{
					// m_lock.Lock();
					m_list.push_back(p_obj);
					// m_lock.Unlock();
				}

				void PushBack(const List<T> &p_objList)
				{
					// TODO : Optimise this, or it will run like a dog on a limp
					for (int nIdx = 0, count = (int)p_objList.Size(); nIdx < count; nIdx++)
						PushBack(p_objList[nIdx]);
				}

				void PopBack(void) 
				{
					m_list.pop_back();
				}
			};
		#else
			template<class T> 
			class List
			{
			protected:
				T* m_list;
				Spinlock m_lock;

				size_t m_capacity,
					   m_growth,
					   m_size;

			public:			
				List(int p_nCapacity = 10, int p_nGrowth = 10)
					: m_capacity(p_nCapacity)
					, m_growth(p_nGrowth)
					, m_size(0)
				{
					m_list = (T*)AlignedMalloc(sizeof(T) * m_capacity, 16);
					BOOST_ASSERT(m_list != NULL);
				}

				~List(void) 
				{
					Safe_AlignedFree(m_list);
				}

				inline const T& operator[](size_t p_index) const {
					return m_list[p_index];
				}

				inline T& operator[](size_t p_index) {
					return m_list[p_index];
				}

				inline size_t Capacity() const {
					return m_capacity;
				}

				inline size_t Size() const {
					return m_size;
				}
			
				inline bool IsEmpty() const {
					return (m_size == 0);
				}
			
				inline T& At(size_t p_index) 
				{
					BOOST_ASSERT(p_index >= 0 && p_index < m_size);
					return m_list[p_index];
				}

				inline const T& At(size_t p_index) const 
				{
					BOOST_ASSERT(p_index >= 0 && p_index < m_size);
					return m_list[p_index];
				}

				inline T& Front(void) 
				{
					BOOST_ASSERT(m_size > 0);
					return m_list[0];
				}
						
				inline const T& Front(void) const 
				{
					BOOST_ASSERT(m_size > 0);
					return m_list[0];
				}

				inline T& Back(void) 
				{
					BOOST_ASSERT(m_size > 0);
					return m_list[m_size - 1];
				}

				inline const T& Back(void) const 
				{
					BOOST_ASSERT(m_size > 0);
					return m_list[m_size - 1];
				}

				void Clear(void) 
				{
					AtomicInt32::Exchange((Int32*)&m_size, 0);
				}

				void PushBack(const T &p_obj) 
				{
					m_lock.Lock();
				
					// Do we need array to grow?
					if (m_size + 1 == m_capacity)
					{
						m_capacity *= m_growth;

						T *list = (T*)AlignedMalloc(sizeof(T) * m_capacity, 16);
						BOOST_ASSERT(list != NULL);

						for (int idx = 0, count = (int)m_size; idx < count; idx++) {
							new (list + idx) T(m_list[idx]);
						}
					
						Safe_AlignedFree(m_list);
						m_list = list;
					}

					// Pushback object
					new (m_list + m_size++) T(p_obj);

					m_lock.Unlock();
				}

				void PushBack(const List<T> &p_objList)
				{
					// TODO : Optimise this, or it will run like a dog
					for (int nIdx = 0, count = (int)p_objList.Size(); nIdx < count; nIdx++)
						PushBack(p_objList[nIdx]);
				}

				void PopBack(void) 
				{
					BOOST_ASSERT(m_size > 0)
					AtomicInt32::Decrement(&m_size);
				}
			};
		#endif

		template<class T> 
		class IImmutableList
		{
		protected:
			T *m_list;
			int m_nSize;

		public:			
			~IImmutableList(void) 
			{ }

			inline const T& operator[](size_t p_index) const {
				return m_list[p_index];
			}

			inline size_t Size() const {
				return m_nSize;
			}
			
			inline bool IsEmpty() const {
				return (m_nSize == 0);
			}
			
			inline const T& At(size_t p_index) const {
				return m_list[p_index];
			}
						
			inline const T& Front(void) const {
				return m_list[0];
			}

			inline const T& Back(void) const {
				return m_list[m_nSize - 1];
			}
		};

		template<class T>
		class ImmutableDiskList :
			IImmutableList<T>
		{
			using IImmutableList<T>::m_list;
			using IImmutableList<T>::m_nSize;

		protected:
			boost::iostreams::mapped_file m_imageFile;			

		public:
			ImmutableDiskList(const std::string &p_strFilename) 
			{
				m_imageFile.open(p_strFilename);
				m_list = (T*)m_imageFile.const_data();
			}

			~ImmutableDiskList(void)
			{
				m_imageFile.close();
			}

			static void Make(List<T> *p_pList, const std::string &p_strOutputFilename)
			{
				std::ofstream imageFile;

				// Open image file writer stream
				imageFile.open(p_strOutputFilename.c_str(), std::ios::binary);

				for (int nIdx = 0; nIdx < p_pList->Size(); ++nIdx)
				{
					T *pElement = &(p_pList->At(nIdx));
					imageFile.write((const char*)pElement, sizeof(T));
				}
				
				imageFile.close();				
			}
		};
	}
}