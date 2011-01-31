//----------------------------------------------------------------------------------------------
//	Filename:	TentFilter.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Implementation for Rays in R3
//----------------------------------------------------------------------------------------------
#pragma once

#include "Filter/Filter.h"

namespace Illumina 
{
	namespace Core
	{
		class TentFilter 
			: public IFilter
		{
		public:
			TentFilter(void);
			TentFilter(const std::string &p_strName);

			void operator()(Vector2 *p_pSamples, int p_nSampleCount);
		};
	} 
}