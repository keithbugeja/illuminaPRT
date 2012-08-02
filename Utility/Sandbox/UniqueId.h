//----------------------------------------------------------------------------------------------
//	Filename:	UniqueID.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once

#include <Threading/Atomic.h>
//----------------------------------------------------------------------------------------------
#include "Logger.h"

using namespace Illumina::Core;

class UniqueID 
{
protected:
	Int32 m_nID;

public:
	UniqueID(void) : m_nID(0) { }

	inline int GetNext(void) {
		return (int)AtomicInt32::FetchAndAdd(&m_nID, (Int32)1);
	}
};