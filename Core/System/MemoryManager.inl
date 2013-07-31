//----------------------------------------------------------------------------------------------
//	Filename:	MemoryManager.cpp
//	Author:		Kevin Napoli
//	Date:		01/07/2013
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;

template <class T>
MemoryManager<T>::MemoryManager(unsigned int numElems, unsigned int alignment)
		: numElems_ ( numElems ), alignment_ ( alignment )
{
	//need to make sure that alignment is a power of 2
	origin_ = new char[sizeof(T) * numElems + alignment];
	//align memory
	begin_ = next_ = (T *)(((uintptr_t)origin_ + alignment - 1) & ~(uintptr_t)(alignment - 1));
	end_ = begin_ + numElems;
	//init all to default - revise
	//for(T * t = begin_; t < end_; t++)
	//{
	//    new (t) T();
	//}
	//std::cout << "Mem Constructed!\n";
}

//BUGGY do not use - should only copy settings
template <class T>
MemoryManager<T>::MemoryManager(const MemoryManager& orig)
{
	//destroy
	for(T * begin = begin_; begin < next_; begin++)
	{
		begin->~T();
	}
	//delete
	delete[] origin_;
	//make copy
	origin_ = new char[sizeof(T) * numElems_ + alignment_];
	//align memory
	begin_ = next_ = (T *)(((uintptr_t)origin_ + alignment_ - 1) & ~(alignment_ - 1));
	end_ = begin_ + numElems_;
	//init all to default - revise - WILL NOT COPY NODE DATA
	for(T * t = begin_; t < end_; t++)
	{
		new (t) T();
	}
	//done copying
}

template <class T>
MemoryManager<T>::~MemoryManager()
{
	//destroy
	for(T * begin = begin_; begin < next_; begin++)
	{
		begin->~T();
	}
	//delete
	delete[] origin_;
}

//getters
template <class T>
T * MemoryManager<T>::getNext()
{
	//boost::lock_guard<boost::mutex> lock ( mut_ );
	if(!freed_.empty())
	{
		T * ret (freed_.front() );
		freed_.pop();
		new (ret) T();
		return ret;
	}
	if(next_ == end_) throw -1;
	new (next_) T();
	return next_++;
}

template <class T>
T * MemoryManager<T>::getNext(unsigned int amount)
{
	//boost::lock_guard<boost::mutex> lock ( mut_ );
	//cannot allocate from queue at the moment since must be contiguous
	if(next_ + amount >= end_) throw -1; //or allocate more but wont be contiguous
	for(unsigned int i = 0; i < amount; i++)
	{
		new (next_ + i) T();
	}
	T * temp ( next_ );
	next_ += amount;
	return temp;
}

//don't use free !! 
template <class T>
void MemoryManager<T>::free(T * node)
{
	//boost::lock_guard<boost::mutex> lock ( mut_ );
	node->~T();
	freed_.push(node);
}

