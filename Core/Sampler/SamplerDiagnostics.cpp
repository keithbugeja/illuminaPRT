//----------------------------------------------------------------------------------------------
//	Filename:	SamplerDiagnostics.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Sampler/SamplerDiagnostics.h"
#include "Maths/Statistics.h"
#include "Image/ImageIO.h"
#include "Image/ImagePPM.h"

#include "PrecomputationSampler.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
float SamplerDiagnostics::FrequencyTest(ISampler *p_pSampler, int p_nSequenceLength)
{
	int sequence = 0;

	for (int j = 0; j < p_nSequenceLength; j++)
		sequence += p_pSampler->Get1DSample() > 0.5f ? 1 : -1;

	float absSObs = Maths::Abs((float)sequence);
	float sqrtSeqLen = Maths::Sqrt((float)p_nSequenceLength);

	float sObs = absSObs / sqrtSeqLen;

	return Statistics::Erfc(sObs / Maths::Sqrt(2.f));
}
//----------------------------------------------------------------------------------------------
float SamplerDiagnostics::ChiSquareTest(ISampler *p_pSampler, int p_nSequenceLength)
{
	float bins[10];

	for (int j = 0; j < 10; j++)
		bins[j] = 0.f;

	for (int j = 0; j < p_nSequenceLength; j++)
	{
		float X = p_pSampler->Get1DSample();
		bins[(int)Maths::Floor(X * 10.f)]++;
	}

	float E = (float)p_nSequenceLength / 10.f,
		X2 = 0;

	for (int j = 0; j < 10; j++)
		X2 += Maths::Sqr(bins[j] - E);

	return X2 / E;
}
//----------------------------------------------------------------------------------------------
void SamplerDiagnostics::DistributionTest(ISampler *p_pSampler, int p_nSequenceLength, const std::string &p_strOutput)
{
	Image image(512, 512, RGBPixel::White);

	Vector2 sample;

	for (int j = 0; j < p_nSequenceLength; j++)
	{
		sample = p_pSampler->Get2DSample();

		image.Set((int)(sample.X * 512.f), (int)(sample.Y * 512.f), RGBPixel::Black);
	}

	ImagePPM imageIO;
	imageIO.Save(image, p_strOutput);
}
//----------------------------------------------------------------------------------------------
