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
void Image::MakeTestCard(void)
{
	RGBPixel cardHues[9] = {RGBPixel::White, RGBPixel::Black, RGBPixel::Green, 
		RGBPixel::Blue, RGBPixel::Red, RGBPixel::Blue, 
		RGBPixel::Green, RGBPixel::Black, RGBPixel::White};

	for (int y = 0; y < this->GetHeight(); ++y)
	{
		for (int x = 0; x < this->GetWidth(); ++x)
		{
			RGBPixel pixel = cardHues[(int)((((float)x) / (this->GetWidth() + 1)) * 8.f)];
			this->Set(x, y, pixel);
		}
	}
}
//----------------------------------------------------------------------------------------------
