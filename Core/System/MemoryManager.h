//----------------------------------------------------------------------------------------------
//	Filename:	MemoryManager.h
//	Author:		Kevin Napoli
//	Date:		01/07/2013
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <queue>

#include "System/IlluminaPRT.h"

namespace Illumina 
{
	namespace Core
	{
		template<class T>
		class MemoryManager 
		{
		public:
			MemoryManager(unsigned int numElems, unsigned int alignment);
			MemoryManager(const MemoryManager& orig);
			~MemoryManager();
			
			//getters
			T * getNext();
			T * getNext(unsigned int amount);
			void free(T *);
		
		private:
			char * origin_;
			T * begin_;
			T * next_;
			T * end_;
			unsigned int numElems_, alignment_;
			std::queue<T *> freed_;
		};
	}
}

#include "MemoryManager.inl"