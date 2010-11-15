//----------------------------------------------------------------------------------------------
//	Filename:	DummyManager.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/Dummy.h"
#include "System/FactoryManager.h"

namespace Illumina 
{ 
	namespace Core 
	{
		typedef FactoryManager<IDummy> DummyManager;
	}
}