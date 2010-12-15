//----------------------------------------------------------------------------------------------
//	Filename:	RandomSampler.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Sampler/RandomSampler.h"
#include "Geometry/Vector2.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
void RandomSampler::Get2DSamples(Vector2 *p_pSamples, int p_nSampleCount)
{
	for (int i = 0; i < p_nSampleCount; i++)
		p_pSamples[i].Set(m_random.NextFloat(), m_random.NextFloat());
}
//----------------------------------------------------------------------------------------------
void RandomSampler::Get1DSamples(float *p_pSamples, int p_nSampleCount)
{
	for (int i = 0; i < p_nSampleCount; i++)
		p_pSamples[i] = m_random.NextFloat();
}
//----------------------------------------------------------------------------------------------
std::string RandomSampler::ToString(void) const
{
	return "[Random Sampler]";
}
//----------------------------------------------------------------------------------------------
