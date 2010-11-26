//----------------------------------------------------------------------------------------------
//	Filename:	Monitor.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/shared_ptr.hpp>

#include "Threading/ReentrantLock.h"

namespace Illumina 
{
	namespace Core
	{
		template<class T> class Monitor; 

		template<class T> class CallProxy 
		{
			T *m_pObject;
			ReentrantLock *m_pLock;
			mutable bool m_bIsOwner;

			CallProxy(T *p_pObject, Spinlock *p_pLock)
				: m_pObject(p_pObject)
				, m_pLock(p_pLock)
				, m_bIsOwner(true) 
			{ }

			CallProxy(const CallProxy &p_callProxy)
				: m_pObject(p_callProxy.m_pObject)
				, m_pLock(p_callProxy.m_pLock)
				, m_bIsOwner(true)
			{ 
				p_callProxy.m_bIsOwner = false; 
			}

			CallProxy& operator= (const CallProxy&); // prevent assignment

		public:
			template<class U> friend class Monitor;
			
			inline T* operator->() const {
				return m_pObject;
			}
			
			inline ~CallProxy() 
			{ 
				if (m_bIsOwner) 
				{
					std::cout<<"Unlock()"<<std::endl;
					m_pLock->Unlock(); 
				}
			}
		};

		template<class T> class Monitor
		{
			Spinlock m_lock;
			T* m_pObject;
			int *m_nReferenceCount;
			
			inline void IncRefCount() const 
			{
				if (m_nReferenceCount) 
					++*m_nReferenceCount;
			}
			
			inline void DecRefCount() const 
			{
				if (m_nReferenceCount && --*m_nReferenceCount == 0) 
				{ 
					delete m_pObject; 
					delete m_nReferenceCount; 
				}
			}
		
		public:
			Monitor(T &p_object)
				: m_pObject(&p_object)
				, m_nReferenceCount(0)
			{ }

			Monitor(T *p_pObject)
				: m_pObject(p_pObject)
				, m_nReferenceCount(new int(1))
			{ }

			Monitor(const Monitor &p_monitor)
				: m_pObject(p_monitor.m_pObject)
				, m_nReferenceCount(p_monitor.m_nReferenceCount)
			{ 
				IncRefCount(); 
			}

			Monitor& operator=(const Monitor &p_monitor)
			{
				p_monitor.IncRefCount();
				DecRefCount();
				m_pObject = p_monitor.m_pObject;
				// owned = p_monitor.owned;
				
				return *this;
			}

			inline ~Monitor() {
				DecRefCount();
			}

			inline CallProxy<T> operator->(void)
			{
				std::cout<<"Lock()"<<std::endl;
				m_lock.Lock(); 
				return CallProxy<T>(m_pObject, &m_lock);
			}
			
			inline T& operator()() const {
				return *m_pObject;
			}

			inline void Enter(void) {
				m_lock.Lock(); 
			}

			inline void Exit(void) {
				m_lock.Unlock();
			}
		};
	}
}