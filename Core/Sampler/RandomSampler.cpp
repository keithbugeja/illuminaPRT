//----------------------------------------------------------------------------------------------
//	Filename:	RandomSampler.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iostream>

#include "Sampler/RandomSampler.h"
#include "Geometry/Vector2.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
void RandomSampler::Reset(void)
{
	m_random.Reset();
}
//----------------------------------------------------------------------------------------------
void RandomSampler::Reset(unsigned int p_unSeed)
{
	m_random.Seed(p_unSeed);
}
//----------------------------------------------------------------------------------------------
void RandomSampler::GetSample(Sample *p_pSample)
{
	Get1DSamples(p_pSample->GetSequence(), p_pSample->Size());
}
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
float RandomSampler::Get1DSample(void) 
{
	return m_random.NextFloat();
}
//----------------------------------------------------------------------------------------------
Vector2 RandomSampler::Get2DSample(void) 
{
	return Vector2(m_random.NextFloat(), m_random.NextFloat());
}
//----------------------------------------------------------------------------------------------
std::string RandomSampler::ToString(void) const
{
	return "[Random Sampler]";
}
//----------------------------------------------------------------------------------------------
