//----------------------------------------------------------------------------------------------
//	Filename:	MultijitterSampler.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Sampler/MultijitterSampler.h"
#include "Geometry/Vector2.h"
#include "Maths/Maths.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
void MultijitterSampler::Reset(void)
{
	m_random.Reset();
}
//----------------------------------------------------------------------------------------------
void MultijitterSampler::Reset(unsigned int p_unSeed)
{
	m_random.Seed(p_unSeed);
}
//----------------------------------------------------------------------------------------------
void MultijitterSampler::GetSample(Sample *p_pSample)
{
	Get1DSamples(p_pSample->GetSequence(), p_pSample->Size());
}
//----------------------------------------------------------------------------------------------
void MultijitterSampler::Get2DSamples(Vector2 *p_pSamples, int p_nSampleCount)
{
	int sqrtSampleCount = (int)Maths::Sqrt((float)p_nSampleCount);
	float subcellWidth = 1.0f /(float)sqrtSampleCount;

	for (int i = 0; i < sqrtSampleCount; i++)
	{
		for (int j = 0; j < sqrtSampleCount; j++)
		{
			p_pSamples[i * sqrtSampleCount + j].Set(
				i * sqrtSampleCount * subcellWidth + 
				j * subcellWidth + m_random.NextFloat() * subcellWidth,
				j * sqrtSampleCount * subcellWidth + 
				i * subcellWidth + m_random.NextFloat() * subcellWidth);
		}
	}

	for (int i = 0; i < sqrtSampleCount; i++)
	{
		for (int j = 0; j < sqrtSampleCount; j++)
		{
			int k = j + (int)(m_random.NextFloat() * (sqrtSampleCount - j - 1));
			float t = p_pSamples[i * sqrtSampleCount + j].U;
			p_pSamples[i * sqrtSampleCount + j].U = p_pSamples[i * sqrtSampleCount + k].U;
			p_pSamples[i * sqrtSampleCount + k].U = t;

			k = j + (int)(m_random.NextFloat() * (sqrtSampleCount - j - 1));
			t = p_pSamples[j * sqrtSampleCount + i].V;
			p_pSamples[j * sqrtSampleCount + i].V = p_pSamples[k * sqrtSampleCount + i].V;
			p_pSamples[k * sqrtSampleCount + i].V = t;
		}
	}
}
//----------------------------------------------------------------------------------------------
void MultijitterSampler::Get1DSamples(float *p_pSamples, int p_nSampleCount)
{
	for (int i = 0; i < p_nSampleCount; i++)
		p_pSamples[i] = (i + m_random.NextFloat()) / p_nSampleCount;
}
//----------------------------------------------------------------------------------------------
float MultijitterSampler::Get1DSample(void)
{
	return m_random.NextFloat();
}
//----------------------------------------------------------------------------------------------
Vector2 MultijitterSampler::Get2DSample(void)
{
	return Vector2(m_random.NextFloat() * 0.5f, (1 + m_random.NextFloat()) * 0.5f);
}
//----------------------------------------------------------------------------------------------
std::string MultijitterSampler::ToString(void) const
{
	return "[Multijitter Sampler]";
}
//----------------------------------------------------------------------------------------------