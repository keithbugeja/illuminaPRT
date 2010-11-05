//----------------------------------------------------------------------------------------------
//	Filename:	Shape.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Implementation for Rays in R3
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>

#include "Maths/Random.h"
#include "Geometry/Vector2.h"
#include "Geometry/Vector3.h"

namespace Illumina 
{
	namespace Core
	{
		class ISampler
		{
		public:
			virtual void Get2DSamples(Vector2* p_pSamples, int p_nSampleCount) = 0;
			virtual void Get1DSamples(float* p_pSamples, int p_nSampleCount) = 0;

			virtual std::string ToString() { return "ISampler"; }
		};

		class RandomSampler
		{
		private:
			Random m_random;

		public:
			void Get2DSamples(Vector2 *p_pSamples, int p_nSampleCount)
			{
				for (int i = 0; i < p_nSampleCount; i++)
					p_pSamples[i].Set(m_random.NextFloat(), m_random.NextFloat());
			}

			void Get1DSamples(float *p_pSamples, int p_nSampleCount)
			{
				for (int i = 0; i < p_nSampleCount; i++)
					p_pSamples[i] = m_random.NextFloat();
			}
		};

		class JitterSampler
		{
		private:
			Random m_random;

		public:
			void Get2DSamples(Vector2 *p_pSamples, int p_nSampleCount)
			{
				int sqrtSampleCount = (int)Maths::Sqrt((float)p_nSampleCount);

				for (int i = 0; i < sqrtSampleCount; i++)
				{
					for (int j = 0; j < sqrtSampleCount; j++)
					{
						p_pSamples[i * sqrtSampleCount + j].Set(
							(i + m_random.NextFloat()) / sqrtSampleCount,
							(j + m_random.NextFloat()) / sqrtSampleCount);
					}
				}
			}

			void Get1DSamples(float *p_pSamples, int p_nSampleCount)
			{
				for (int i = 0; i < p_nSampleCount; i++)
					p_pSamples[i] = (i + m_random.NextFloat()) / p_nSampleCount;
			}
		};
	} 
}