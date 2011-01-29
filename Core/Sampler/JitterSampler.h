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
			JitterSampler(const std::string &p_strId) : ISampler(p_strId) { }
			JitterSampler(void) { }

			void Get2DSamples(Vector2 *p_pSamples, int p_nSampleCount);
			void Get1DSamples(float *p_pSamples, int p_nSampleCount);

			float Get1DSample(void);
			Vector2 Get2DSample(void);

			std::string ToString(void) const;
		};

		class MultijitterSampler
			: public ISampler
		{
		private:
			Random m_random;

		public:
			MultijitterSampler(const std::string &p_strId) : ISampler(p_strId) { }
			MultijitterSampler(void) { }

			void Get2DSamples(Vector2 *p_pSamples, int p_nSampleCount);
			void Get1DSamples(float *p_pSamples, int p_nSampleCount);

			float Get1DSample(void);
			Vector2 Get2DSample(void);

			std::string ToString(void) const;
		};
	} 
}