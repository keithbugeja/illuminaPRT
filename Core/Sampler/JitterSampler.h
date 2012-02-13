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

			void Reset(void);

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

			void Reset(void);

			void Get2DSamples(Vector2 *p_pSamples, int p_nSampleCount);
			void Get1DSamples(float *p_pSamples, int p_nSampleCount);

			float Get1DSample(void);
			Vector2 Get2DSample(void);

			std::string ToString(void) const;
		};

		template <int TSequenceSize, int TSeed1, int TSeed2>
		class PrecomputationSampler
			: public ISampler
		{
		private:
			Random m_random;

			Vector2 m_pSampleList[TSequenceSize];
			int m_nSampleIndex;

		public:
			void Reset(void) { m_nSampleIndex = 0; }

			PrecomputationSampler(const std::string &p_strName) : ISampler(p_strName) { GenerateSamples(TSeed1, TSeed2); }
			PrecomputationSampler(void) { GenerateSamples(TSeed1, TSeed2); }

		protected:
			void GenerateSamples(int p_nSeed1, int p_nSeed2)
			{
				for (int index = 0; index < TSequenceSize; ++index)
				{
					m_pSampleList[index].Set(m_random.NextFloat(), m_random.NextFloat());
					//m_pSampleList[index].Set(VanDerCorput(index, p_nSeed1), Sobol2(index, p_nSeed2));
				}

				m_nSampleIndex = 0;
			}
			//----------------------------------------------------------------------------------------------
			float VanDerCorput(unsigned int n, const unsigned int scramble)
			{
				n = (n << 16) | (n >> 16);
				n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);
				n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);
				n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
				n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
				n ^= scramble;

				return (float)n / (float)0x100000000LL;
			}
			//----------------------------------------------------------------------------------------------
			float Sobol2(unsigned int n, unsigned int scramble)
			{
				for (unsigned int v = 1 << 31; n != 0; n >>= 1, v ^= v >> 1)
					if (n & 0x1) scramble ^= v;

				return (float)scramble / (float)0x100000000LL;
			}

		public:
			//----------------------------------------------------------------------------------------------
			void Get2DSamples(Vector2 *p_pSamples, int p_nSampleCount)
			{
				for (int index = 0; index < p_nSampleCount; index++)
				{
					p_pSamples[index].Set(m_pSampleList[m_nSampleIndex].X, m_pSampleList[m_nSampleIndex].Y);
		
					if (++m_nSampleIndex >= TSequenceSize)
						m_nSampleIndex = 0;
				}
			}
			//----------------------------------------------------------------------------------------------
			void Get1DSamples(float *p_pSamples, int p_nSampleCount)
			{
				for (int index = 0; index < p_nSampleCount; index++)
				{
					p_pSamples[index] = m_pSampleList[m_nSampleIndex].X;
		
					if (++m_nSampleIndex >= TSequenceSize)
						m_nSampleIndex = 0;
				}
			}
			//----------------------------------------------------------------------------------------------
			float Get1DSample(void) 
			{
					float result = m_pSampleList[m_nSampleIndex].X;
		
					if (++m_nSampleIndex >= TSequenceSize)
						m_nSampleIndex = 0;

					return result;
			}
			//----------------------------------------------------------------------------------------------
			Vector2 Get2DSample(void) 
			{
					Vector2 result(m_pSampleList[m_nSampleIndex]);
		
					if (++m_nSampleIndex >= TSequenceSize)
						m_nSampleIndex = 0;

					return result;
			}
			//----------------------------------------------------------------------------------------------
			std::string ToString(void) const
			{
				return "[PrecomputationSampler Sampler]";
			}
			//----------------------------------------------------------------------------------------------
		};
	} 
}