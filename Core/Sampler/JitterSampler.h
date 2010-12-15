//----------------------------------------------------------------------------------------------
//	Filename:	JitterSampler.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Sampler/Sampler.h"
#include "Maths/Random.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		class JitterSampler 
			: public ISampler
		{
		private:
			Random m_random;

		public:
			void Get2DSamples(Vector2 *p_pSamples, int p_nSampleCount);
			void Get1DSamples(float *p_pSamples, int p_nSampleCount);

			std::string ToString(void) const;
		};
	} 
}