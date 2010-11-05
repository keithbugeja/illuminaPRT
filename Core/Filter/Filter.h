//----------------------------------------------------------------------------------------------
//	Filename:	Filter.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Implementation for Rays in R3
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>

#include "Geometry/Vector2.h"
#include "Geometry/Vector3.h"

namespace Illumina 
{
	namespace Core
	{
		class IFilter
		{
		public:
			virtual void operator()(Vector2 *p_pSamples, int p_nSampleCount) = 0;
		};

		class BoxFilter
		{
		public:
			void operator()(Vector2 *p_pSamples, int p_nSampleCount)
			{
				for (int i = 0; i < p_nSampleCount; i++)
				{
					p_pSamples[i].Set(
						p_pSamples[i].X - 0.5f,
						p_pSamples[i].Y - 0.5f);
				}

			}
		};

		class TentFilter
		{
		public:
			void operator()(Vector2 *p_pSamples, int p_nSampleCount)
			{
				for (int i = 0; i < p_nSampleCount; i++)
				{
					float x = p_pSamples[i].X;
					float y = p_pSamples[i].Y;

					if (x < 0.5f) p_pSamples[i].X = Maths::Sqrt(2.0f * x) - 1.0f;
					else p_pSamples[i].X = Maths::Sqrt(2.0f - 2.0f * x);

					if (y < 0.5f) p_pSamples[i].Y = Maths::Sqrt(2.0f * y) - 1.0f;
					else p_pSamples[i].Y = Maths::Sqrt(2.0f - 2.0f * y);
				}
			}
		};
	} 
}