//----------------------------------------------------------------------------------------------
//	Filename:	SamplerDiagnostics.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Sampler/Sampler.h"

//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		class SamplerDiagnostics
		{
		public:
			static float FrequencyTest(ISampler *p_pSampler, int p_nSequenceLength);
			static float ChiSquareTest(ISampler *p_pSampler, int p_nSequenceLength);
			static void DistributionTest(ISampler *p_pSampler, int p_nSequenceLength, const std::string &p_strOutput);
		};
		//----------------------------------------------------------------------------------------------
	}
}