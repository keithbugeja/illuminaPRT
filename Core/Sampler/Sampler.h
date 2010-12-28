//----------------------------------------------------------------------------------------------
//	Filename:	Sampler.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		class ISampler
		{
		public:
			virtual void Get2DSamples(Vector2* p_pSamples, int p_nSampleCount) = 0;
			virtual void Get1DSamples(float* p_pSamples, int p_nSampleCount) = 0;
			
			virtual float Get1DSample(void) = 0;
			virtual Vector2 Get2DSample(void) = 0;

			virtual std::string ToString(void) const = 0;
		};
	} 
}