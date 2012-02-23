//----------------------------------------------------------------------------------------------
//	Filename:	Image.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	A simple image class to store an array of RGB colours that can be accessed
//  via one or two-dimensional indexing. 
//----------------------------------------------------------------------------------------------
#include "Image/Image.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
Image::Image(int p_nWidth, int p_nHeight)
	: RGBSurface(p_nWidth, p_nHeight)
{ }
//----------------------------------------------------------------------------------------------
Image::Image(int p_nWidth, int p_nHeight, const RGBPixel &p_rgb)
	: RGBSurface(p_nWidth, p_nHeight, p_rgb)
{ }
//----------------------------------------------------------------------------------------------
Image::Image(int p_nWidth, int p_nHeight, RGBPixel *p_pRGBBuffer)
	: RGBSurface(p_nWidth, p_nHeight, p_pRGBBuffer)
{ }
//----------------------------------------------------------------------------------------------
void Image::GammaCorrect(float p_fGamma)
{
	float fPower = 1.0f / p_fGamma;

	for (int i = 0; i < m_nWidth * m_nHeight; i++)
	{
		m_bitmap[i].Set(Maths::Pow(m_bitmap[i].R, fPower),
			Maths::Pow(m_bitmap[i].G, fPower),
			Maths::Pow(m_bitmap[i].B, fPower));
	}
}
//----------------------------------------------------------------------------------------------
void Image::ToneMap(void) {
	ToneMap(this);
}
//----------------------------------------------------------------------------------------------
void Image::ToneMap(Image *p_pImage) const
{
	RGBPixel Lw(0), Ld, 
		*pSource = m_bitmap;

	int nArea = 
		m_nHeight * m_nWidth;

	for (int index = 0; index < nArea; ++index, ++pSource)
	{
		Lw.R += Maths::Log(pSource->R + Maths::Epsilon);
		Lw.R += Maths::Log(pSource->G + Maths::Epsilon);
		Lw.R += Maths::Log(pSource->B + Maths::Epsilon);
	}

	float fAreaInv = 
		1.f / ((float)nArea);

	Lw.R = Maths::Exp(fAreaInv * Lw.R);
	Lw.G = Maths::Exp(fAreaInv * Lw.G);
	Lw.B = Maths::Exp(fAreaInv * Lw.B);

	pSource = p_pImage->m_bitmap;

	for (int index = 0; index < nArea; ++index, ++pSource)
	{
		pSource->R *= 0.18f / Lw.R;
		pSource->R /= pSource->R + 1;

		pSource->G *= 0.18f / Lw.G;
		pSource->G /= pSource->G + 1;

		pSource->B *= 0.18f / Lw.B;
		pSource->B /= pSource->B + 1;
	}
}
//----------------------------------------------------------------------------------------------
