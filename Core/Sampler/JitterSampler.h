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
			JitterSampler(const std::string &p_strName) : ISampler(p_strName) { }
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
			MultijitterSampler(const std::string &p_strName) : ISampler(p_strName) { }
			MultijitterSampler(void) { }

			void Get2DSamples(Vector2 *p_pSamples, int p_nSampleCount);
			void Get1DSamples(float *p_pSamples, int p_nSampleCount);

			float Get1DSample(void);
			Vector2 Get2DSample(void);

			std::string ToString(void) const;
		};

		class PrecomputationSampler
			: public ISampler
		{
		private:
			Random m_random;

			Vector2 *m_sample2DList;
			float *m_sampleList;
			int m_sampleIndex;
			int m_sampleCount;

		protected:
			void GenerateSamples(int p_nSampleCount);

		public:
			PrecomputationSampler(const std::string &p_strName) : ISampler(p_strName) { GenerateSamples(4096); }
			PrecomputationSampler(void) { GenerateSamples(4096); }

			void Get2DSamples(Vector2 *p_pSamples, int p_nSampleCount);
			void Get1DSamples(float *p_pSamples, int p_nSampleCount);

			float Get1DSample(void);
			Vector2 Get2DSample(void);

			std::string ToString(void) const;
		};
	} 
}