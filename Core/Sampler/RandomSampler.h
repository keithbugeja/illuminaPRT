//----------------------------------------------------------------------------------------------
//	Filename:	RandomSampler.h
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
		class RandomSampler 
			: public ISampler
		{
		private:
			Random m_random;

		public:
			RandomSampler(void) { }
			RandomSampler(const std::string &p_strName) : ISampler(p_strName) { }

			void Reset(void);

			void Get2DSamples(Vector2 *p_pSamples, int p_nSampleCount);
			void Get1DSamples(float *p_pSamples, int p_nSampleCount);

			float Get1DSample(void);
			Vector2 Get2DSample(void);

			std::string ToString(void) const;
		};
	} 
}