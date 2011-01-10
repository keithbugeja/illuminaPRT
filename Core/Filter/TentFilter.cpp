//----------------------------------------------------------------------------------------------
//	Filename:	TentFilter.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Implementation for Rays in R3
//----------------------------------------------------------------------------------------------
#include "Filter/TentFilter.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
void TentFilter::operator()(Vector2 *p_pSamples, int p_nSampleCount)
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
//----------------------------------------------------------------------------------------------
