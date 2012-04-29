//----------------------------------------------------------------------------------------------
//	Filename:	LowDiscrepancySampler.h
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
		//----------------------------------------------------------------------------------------------
		// Quasi-random sequence generator
		//----------------------------------------------------------------------------------------------
		class QuasiRandomSequence 
		{
		public:
			//----------------------------------------------------------------------------------------------
			static float Halton(unsigned int index, unsigned int base)
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
			static float VanDerCorput(unsigned int n)
			{
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
			static float VanDerCorput(unsigned int n, const unsigned int scramble)
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
			static float Sobol2(unsigned int n, unsigned int scramble)
			{
				for (unsigned int v = 1 << 31; n != 0; n >>= 1, v ^= v >> 1)
					if (n & 0x1) scramble ^= v;

				return (float)scramble / (float)0x100000000LL;
			}
		};
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
		// Sequence Generators
		//----------------------------------------------------------------------------------------------
		class ISequenceGenerator
		{
		public:
			virtual float operator()(void) = 0;
		};
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
		template<int TSeed>
		class HaltonSequenceGenerator 
			: public ISequenceGenerator
		{
		private:
			int m_nSequenceId;
		
		public:
			HaltonSequenceGenerator(void) : m_nSequenceId(0) { }
			float operator()(void) { return QuasiRandomSequence::Halton(m_nSequenceId++, TSeed); }
		};
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
		template<int TSeed>
		class SobolSequenceGenerator
			: public ISequenceGenerator
		{
		private:
			int m_nSequenceId;
		
		public:
			SobolSequenceGenerator(void) : m_nSequenceId(0) { }
			float operator()(void) { return QuasiRandomSequence::Sobol2(m_nSequenceId++, TSeed); }
		};
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
		template<int TSeed>
		class VanDerCorputSequenceGenerator
			: public ISequenceGenerator
		{
		private:
			int m_nSequenceId;
		
		public:
			VanDerCorputSequenceGenerator(void) : m_nSequenceId(0) { }
			float operator()(void) { return QuasiRandomSequence::VanDerCorput(m_nSequenceId++); }
		};
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
		template<int TSeed>
		class RandomSequenceGenerator
			: public ISequenceGenerator
		{
		private:
			Random m_random;
		
		public:
			RandomSequenceGenerator(void) : m_random(TSeed) { }
			float operator()(void) { return m_random.NextFloat(); }
		};
		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------
		template <int TSequenceSize, class TFirstGenerator, class TSecondGenerator>
		class PrecomputedSampler
			: public ISampler
		{
		private:
			float m_pSampleList[TSequenceSize + 2];
			int m_nSampleIndex;

		protected:
			void GenerateSamples(void)
			{
				BOOST_ASSERT(((TSequenceSize + 1) - Maths::Pow(2, Maths::Ld(TSequenceSize + 1))) == 0);

				TFirstGenerator first;
				TSecondGenerator second;

				for (int index = 0; index < TSequenceSize + 2;)
				{
					m_pSampleList[index++] = first();
					m_pSampleList[index++] = second();
				}

				// Reset sample index
				Reset();
			}

		public:
			void Reset(void) { m_nSampleIndex = 0; }

			PrecomputedSampler(const std::string &p_strName) : ISampler(p_strName) { GenerateSamples(); }
			PrecomputedSampler(void) { GenerateSamples(); }
		
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
					m_nSampleIndex += size;
				}
			}
			//----------------------------------------------------------------------------------------------
			void Get2DSamples(Vector2 *p_pSamples, int p_nSampleCount)
			{
				for (int index = p_nSampleCount; index; --index)
				{
					p_pSamples->X = m_pSampleList[m_nSampleIndex++];
					p_pSamples->Y = m_pSampleList[m_nSampleIndex++];

					p_pSamples++; m_nSampleIndex &= TSequenceSize;
					
					//m_nSampleIndex &= TSequenceSize;
					//if (m_nSampleIndex >= TSequenceSize)
						//m_nSampleIndex = 0;
				}
			}
			//----------------------------------------------------------------------------------------------
			void Get1DSamples(float *p_pSamples, int p_nSampleCount)
			{
				for (int index = p_nSampleCount; index; --index)
				{
					*p_pSamples++ = m_pSampleList[m_nSampleIndex++];
					m_nSampleIndex &= TSequenceSize;
					
					//if (++m_nSampleIndex >= TSequenceSize)
						//m_nSampleIndex = 0;
				}
			}
			//----------------------------------------------------------------------------------------------
			float Get1DSample(void) 
			{
					return m_pSampleList[(m_nSampleIndex = (m_nSampleIndex + 1) & TSequenceSize)];

					//float result = m_pSampleList[m_nSampleIndex];
					//if (++m_nSampleIndex >= TSequenceSize)
						//m_nSampleIndex = 0;
					//return result;
			}
			//----------------------------------------------------------------------------------------------
			Vector2 Get2DSample(void) 
			{
					float *sample = m_pSampleList + m_nSampleIndex;
					m_nSampleIndex = (m_nSampleIndex + 2) & TSequenceSize;
					return Vector2(sample[0], sample[1]);

					//Vector2 result(m_pSampleList[m_nSampleIndex++], m_pSampleList[m_nSampleIndex++]);
					//if (m_nSampleIndex >= TSequenceSize)
						//m_nSampleIndex = 0;
					//return result;
			}
			//----------------------------------------------------------------------------------------------
			std::string ToString(void) const
			{
				return "[PrecomputationSampler Sampler]";
			}
			//----------------------------------------------------------------------------------------------
		};

		typedef PrecomputedSampler<0x01FFFF, HaltonSequenceGenerator<7>, HaltonSequenceGenerator<5>> PrecomputedHaltonSampler;
		typedef PrecomputedSampler<0x01FFFF, SobolSequenceGenerator<3>, VanDerCorputSequenceGenerator<7>> PrecomputedSobolSampler;
		typedef PrecomputedSampler<0x01FFFF, RandomSequenceGenerator<3331333>, RandomSequenceGenerator<63761>> PrecomputedRandomSampler;
	} 
}