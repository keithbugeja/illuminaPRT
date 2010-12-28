//----------------------------------------------------------------------------------------------
//	Filename:	JitterSampler.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Sampler/JitterSampler.h"
#include "Geometry/Vector2.h"
#include "Maths/Maths.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
void JitterSampler::Get2DSamples(Vector2 *p_pSamples, int p_nSampleCount)
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
//----------------------------------------------------------------------------------------------
void JitterSampler::Get1DSamples(float *p_pSamples, int p_nSampleCount)
{
	for (int i = 0; i < p_nSampleCount; i++)
		p_pSamples[i] = (i + m_random.NextFloat()) / p_nSampleCount;
}
//----------------------------------------------------------------------------------------------
float JitterSampler::Get1DSample(void)
{
	return m_random.NextFloat();
}
//----------------------------------------------------------------------------------------------
Vector2 JitterSampler::Get2DSample(void)
{
	return Vector2(m_random.NextFloat() * 0.5f, (1 + m_random.NextFloat()) * 0.5f);
}
//----------------------------------------------------------------------------------------------
std::string JitterSampler::ToString(void) const
{
	return "[Jitter Sampler]";
}
//----------------------------------------------------------------------------------------------
