//----------------------------------------------------------------------------------------------
//	Filename:	UniqueID.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "UniqueID.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
volatile long long TransientUID::m_llNextID = 0;