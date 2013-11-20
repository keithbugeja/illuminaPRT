//----------------------------------------------------------------------------------------------
//	Filename:	LowDiscrepancySampler.h
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
		class LowDiscrepancySampler 
			: public ISampler
		{
		private:
			Random m_random;
			unsigned int m_seedVDC, 
				m_seedS2, m_next;

		public:
			LowDiscrepancySampler(void) { Reset(); }
			LowDiscrepancySampler(const std::string &p_strName) : ISampler(p_strName) { Reset(); }

			void Reset(void);
			void Reset(unsigned int p_unSeed);

			void Get2DSamples(Vector2 *p_pSamples, int p_nSampleCount);
			void Get1DSamples(float *p_pSamples, int p_nSampleCount);

			float Get1DSample(void);
			Vector2 Get2DSample(void);

			void GetSample(Sample *p_pSample);

			std::string ToString(void) const;
		};
	} 
}