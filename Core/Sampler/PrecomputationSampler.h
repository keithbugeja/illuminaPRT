//----------------------------------------------------------------------------------------------
//	Filename:	LowDiscrepancySampler.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <cstring>

#include "Maths/Random.h"
#include "Sampler/Sampler.h"
#include "Sampler/QuasiRandom.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
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
			void Reset(unsigned int p_unSeed) { m_nSampleIndex = p_unSeed % TSequenceSize; }

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
				}
			}
			//----------------------------------------------------------------------------------------------
			void Get1DSamples(float *p_pSamples, int p_nSampleCount)
			{
				for (int index = p_nSampleCount; index; --index)
				{
					*p_pSamples++ = m_pSampleList[m_nSampleIndex++];
					m_nSampleIndex &= TSequenceSize;
				}
			}
			//----------------------------------------------------------------------------------------------
			float Get1DSample(void) 
			{
					return m_pSampleList[(m_nSampleIndex = (m_nSampleIndex + 1) & TSequenceSize)];
			}
			//----------------------------------------------------------------------------------------------
			Vector2 Get2DSample(void) 
			{
					float *sample = m_pSampleList + m_nSampleIndex;
					m_nSampleIndex = (m_nSampleIndex + 2) & TSequenceSize;
					return Vector2(sample[0], sample[1]);
			}
			//----------------------------------------------------------------------------------------------
			std::string ToString(void) const
			{
				return "[PrecomputationSampler Sampler]";
			}
			//----------------------------------------------------------------------------------------------
		};

		typedef PrecomputedSampler<0xFFFFFF, HaltonSequenceGenerator<7>, HaltonSequenceGenerator<5>> PrecomputedHaltonSampler;
		typedef PrecomputedSampler<0xFFFFFF, VanDerCorputSequenceGenerator<0>, SobolSequenceGenerator<0>> PrecomputedSobolSampler;
		typedef PrecomputedSampler<0xFFFFFF, RandomSequenceGenerator<3331333>, RandomSequenceGenerator<63761>> PrecomputedRandomSampler;
	} 
}