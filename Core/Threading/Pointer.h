//----------------------------------------------------------------------------------------------
//	Filename:	Monitor.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Threading/Spinlock.h"

namespace Illumina 
{
	namespace Core
	{
		/*
		template<class T> class ReferenceCounted
		{
		protected:
			volatile long m_lReferenceCount;

		public:
			ReferenceCounted(void) 
				: m_lReferenceCount(0) 
			{ }

			virtual ~ReferenceCounted(void) { }

			inline void AddReference(void) 
			{
				m_lReferenceCount++;
			}

			inline void ReleaseReference(void) 
			{ 
				m_lReferenceCount--;
			}

			inline long GetReferenceCount(void) { return m_lReferenceCount; }
		};

		template<class T> class Reference
		{
		protected:
			T* m_pObject;
		
		public:
			Reference(void)
				: m_pObject(NULL)
			{ }

			Reference(T* p_pObject)
				: m_pObject(p_pObject)
			{
				m_pObject->AddReference();
			}

			Reference(Reference<T>& p_reference)
				: m_pObject(NULL) 
			{
				*this = p_reference;
			}

			T* operator->(void) { return m_pObject; }
			const T* operator->(void) const { return m_pObject; }

			T& operator*(void) { return *m_pObject; }
			const T& operator*(void) const { return *m_pObject; }

			Reference<T>& operator=(Reference<T>& p_reference) 
			{
				if (this->m_pObject == p_reference->m_pObject)
					return *this;

				if (p_reference.m_pObject != NULL) 
					p_reference.m_pObject->AddReference();

				if (this->m_pObject != NULL)
					this->m_pObject->ReleaseReference();

				m_pObject = p_reference->m_pObject;
			}
		};
		*/
	}
}
/*
class COREDLL ReferenceCounted {
public:
	ReferenceCounted() { nReferences = 0; }
	int nReferences;
private:
	ReferenceCounted(const ReferenceCounted &);
	ReferenceCounted &operator=(const ReferenceCounted &);
};
template <class T> class Reference {
public:
	// Reference Public Methods
	Reference(T *p = NULL) {
		ptr = p;
		if (ptr) ++ptr->nReferences;
	}
	Reference(const Reference<T> &r) {
		ptr = r.ptr;
		if (ptr) ++ptr->nReferences;
	}
	Reference &operator=(const Reference<T> &r) {
		if (r.ptr) r.ptr->nReferences++;
		if (ptr && --ptr->nReferences == 0) delete ptr;
		ptr = r.ptr;
		return *this;
	}
	Reference &operator=(T *p) {
		if (p) p->nReferences++;
		if (ptr && --ptr->nReferences == 0) delete ptr;
		ptr = p;
		return *this;
	}
	~Reference() {
		if (ptr && --ptr->nReferences == 0)
			delete ptr;
	}
	T *operator->() { return ptr; }
	const T *operator->() const { return ptr; }
	operator bool() const { return ptr != NULL; }
	bool operator<(const Reference<T> &t2) const {
		return ptr < t2.ptr;
	}
private:
	T *ptr;
};
*/