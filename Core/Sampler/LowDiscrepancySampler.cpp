//----------------------------------------------------------------------------------------------
//	Filename:	LowDiscrepancySampler.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iostream>

#include "Sampler/LowDiscrepancySampler.h"
#include "Sampler/QuasiRandom.h"
#include "Geometry/Vector2.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
void LowDiscrepancySampler::Reset(void)
{
	m_random.Reset();

	m_seedVDC = m_random.Next();
	m_seedS2 = m_random.Next();

	m_next = 0;
}
//----------------------------------------------------------------------------------------------
void LowDiscrepancySampler::Reset(unsigned int p_unSeed)
{
	m_random.Seed(p_unSeed);

	m_seedVDC = m_random.Next();
	m_seedS2 = m_random.Next();

	m_next = 0;
}
//----------------------------------------------------------------------------------------------
void LowDiscrepancySampler::GetSample(Sample *p_pSample)
{
	Get1DSamples(p_pSample->GetSequence(), p_pSample->Size());
}
//----------------------------------------------------------------------------------------------
void LowDiscrepancySampler::Get2DSamples(Vector2 *p_pSamples, int p_nSampleCount)
{
	unsigned int next = (unsigned int)AtomicInt32::FetchAndAdd((Int32*)&m_next, p_nSampleCount);

	for (int i = 0; i < p_nSampleCount; i++)
		p_pSamples[i].Set(QuasiRandomSequence::VanDerCorput(next + i, m_seedVDC), QuasiRandomSequence::Sobol2(next + i, m_seedS2));
}
//----------------------------------------------------------------------------------------------
void LowDiscrepancySampler::Get1DSamples(float *p_pSamples, int p_nSampleCount)
{
	unsigned int next = (unsigned int)AtomicInt32::FetchAndAdd((Int32*)&m_next, p_nSampleCount);

	for (int i = 0; i < p_nSampleCount; i++)
		p_pSamples[i] = QuasiRandomSequence::VanDerCorput(next + i, m_seedVDC);
}
//----------------------------------------------------------------------------------------------
float LowDiscrepancySampler::Get1DSample(void) 
{
	return QuasiRandomSequence::VanDerCorput(AtomicInt32::Increment((Int32*)&m_next), m_seedVDC);
}
//----------------------------------------------------------------------------------------------
Vector2 LowDiscrepancySampler::Get2DSample(void) 
{
	unsigned int next = (unsigned int)AtomicInt32::Increment((Int32*)&m_next);

	return Vector2(QuasiRandomSequence::VanDerCorput(next, m_seedVDC), QuasiRandomSequence::Sobol2(next, m_seedS2));
}
//----------------------------------------------------------------------------------------------
std::string LowDiscrepancySampler::ToString(void) const
{
	return "[Low Discrepancy Sampler]";
}
//----------------------------------------------------------------------------------------------
