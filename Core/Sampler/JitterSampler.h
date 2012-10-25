//----------------------------------------------------------------------------------------------
//	Filename:	JitterSampler.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <cstring>

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
			void Reset(unsigned int p_unSeed);

			void Get2DSamples(Vector2 *p_pSamples, int p_nSampleCount);
			void Get1DSamples(float *p_pSamples, int p_nSampleCount);

			float Get1DSample(void);
			Vector2 Get2DSample(void);

			void GetSample(Sample *p_pSample);

			std::string ToString(void) const;
		};

/*
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

			void GetSample(Sample *p_pSample);

			std::string ToString(void) const;
		};

		template <int TSequenceSize, int TSeed1, int TSeed2>
		class PrecomputationSampler
			: public ISampler
		{
		private:
			Random m_random;

			//Vector2 m_pSampleList[TSequenceSize];
			float m_pSampleList[TSequenceSize + 2];
			int m_nSampleIndex;

		public:
			void Reset(void) { m_nSampleIndex = 0; }

			PrecomputationSampler(const std::string &p_strName) : ISampler(p_strName) { GenerateSamples(TSeed1, TSeed2); }
			PrecomputationSampler(void) { GenerateSamples(TSeed1, TSeed2); }

		protected:
			void GenerateSamples(int p_nSeed1, int p_nSeed2)
			{
				int seed1 = p_nSeed1,
					seed2 = p_nSeed2;

				seed1 = 5;
				seed2 = 7;

				for (int index = 0; index < TSequenceSize + 2;)
				{
					//m_pSampleList[index++] = m_random.NextFloat();
					//m_pSampleList[index++] = m_random.NextFloat();
					
					m_pSampleList[index++] = Halton(index, seed1);
					m_pSampleList[index++] = Halton(index, seed2);

					//m_pSampleList[index++] = VanDerCorput(index, p_nSeed1);
					//m_pSampleList[index++] = Sobol2(index, p_nSeed2);
		
					//m_pSampleList[index].Set(VanDerCorput(index), Halton(index, 3));
					//m_pSampleList[index].Set(Halton(index, 2), Halton(index, 3));//Sobol2(index, p_nSeed2));//Halton(index, 3));
					//std::cout << m_pSampleList[index].ToString() << std::endl;
					//m_pSampleList[index].Set(m_random.NextFloat(), m_random.NextFloat());
					//m_pSampleList[index].Set(VanDerCorput(index, p_nSeed1), Sobol2(index, p_nSeed2));
				}

				m_nSampleIndex = 0;
			}
			//----------------------------------------------------------------------------------------------
			float Halton(unsigned int index, unsigned int base)
			{
				float result = 0;
				float f = 1.0 / base;
				int i = index;
				while (i > 0) 
				{
					result = result + f * (i % base);
					i = Maths::Floor(i / base);
					f = f / base;
				}

				return result;
			}
			//----------------------------------------------------------------------------------------------
			float VanDerCorput(unsigned int n)
			{
				// 0000 -> 0000
				// 0001 -> 1000
				// 0010 -> 0100
				// 0011 -> 1100
				// 0100 -> 0010
				// 0101 -> 1010
				// 0110 -> 0110
				// 0111 -> 1110
				// 1000 -> 0001
				// 1001 -> 1001
				// 1010 -> 0101
				// 1011 -> 1101
				// 1100 -> 0011
				// 1101 -> 1011
				// 1110 -> 0111
				// 1111 -> 1111

				const unsigned int nibble[] = { 0x0, 0x8, 0x4, 0xC, 
												0x2, 0xA, 0x6, 0xE, 
												0x1, 0x9, 0x5, 0xD, 
												0x3, 0xB, 0x7, 0xF};

				float num = (float)(
					(nibble[n & 0xF] << 28) +
					(nibble[(n >> 4) & 0xF] << 24) +
					(nibble[(n >> 8) & 0xF] << 20) +
					(nibble[(n >> 12) & 0xF] << 16) +
					(nibble[(n >> 16) & 0xF] << 12) +
					(nibble[(n >> 20) & 0xF] << 8) +
					(nibble[(n >> 24) & 0xF] << 4) +
					(nibble[(n >> 28) & 0xF]));

				return num / (float)0x100000000LL;
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
			void GetSample(Sample *p_pSample)
			{
				BOOST_ASSERT(p_pSample->Size() < TSequenceSize);

				float *sequence = p_pSample->GetSequence();
				int size = p_pSample->Size();

				// Cannot do in a single block copy
				if (m_nSampleIndex + size >= TSequenceSize)
				{
					int leftSize = TSequenceSize - m_nSampleIndex;

					std::memcpy(sequence, m_pSampleList + m_nSampleIndex, leftSize);
					std::memcpy(sequence + leftSize, m_pSampleList, size - leftSize);

					m_nSampleIndex = size - leftSize;
				}
				else
				{
					std::memcpy(sequence, m_pSampleList + m_nSampleIndex, size);
					m_sampleIndex += size;
				}
			}
			//----------------------------------------------------------------------------------------------
			void Get2DSamples(Vector2 *p_pSamples, int p_nSampleCount)
			{
				for (int index = 0; index < p_nSampleCount; index++)
				{
					p_pSamples[index].Set(m_pSampleList[m_nSampleIndex++], m_pSampleList[m_nSampleIndex++]);

					if (m_nSampleIndex >= TSequenceSize)
						m_nSampleIndex = 0;
				}
			}
			//----------------------------------------------------------------------------------------------
			void Get1DSamples(float *p_pSamples, int p_nSampleCount)
			{
				for (int index = 0; index < p_nSampleCount; index++)
				{
					p_pSamples[index] = m_pSampleList[m_nSampleIndex];

					if (++m_nSampleIndex >= TSequenceSize)
						m_nSampleIndex = 0;
				}
			}
			//----------------------------------------------------------------------------------------------
			float Get1DSample(void) 
			{
					float result = m_pSampleList[m_nSampleIndex];

					if (++m_nSampleIndex >= TSequenceSize)
						m_nSampleIndex = 0;

					return result;
			}
			//----------------------------------------------------------------------------------------------
			Vector2 Get2DSample(void) 
			{
					Vector2 result(m_pSampleList[m_nSampleIndex++], m_pSampleList[m_nSampleIndex++]);

					if (m_nSampleIndex >= TSequenceSize)
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
*/
	} 
}