//----------------------------------------------------------------------------------------------
//	Filename:	ImageDevice.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Device/ImageDevice.h"

#include "Image/Image.h"
#include "Image/ImageIO.h"

#include "Spectrum/Spectrum.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
ImageDevice::ImageDevice(int p_nWidth, int p_nHeight, IImageIO *p_pImageIO, const std::string &p_strFilename, bool p_bKillFilterOnExit)
	: m_pImage(new Image(p_nWidth, p_nHeight))
	, m_pImageIO(p_pImageIO)
	, m_strFilename(p_strFilename)
	, m_bKillFilterOnExit(p_bKillFilterOnExit)
{ }
//----------------------------------------------------------------------------------------------
ImageDevice::ImageDevice(const std::string &p_strName, int p_nWidth, int p_nHeight, IImageIO *p_pImageIO, const std::string &p_strFilename, bool p_bKillFilterOnExit)
	: IDevice(p_strName) 
	, m_pImage(new Image(p_nWidth, p_nHeight))
	, m_pImageIO(p_pImageIO)
	, m_strFilename(p_strFilename)
	, m_bKillFilterOnExit(p_bKillFilterOnExit)
{ }
//----------------------------------------------------------------------------------------------
ImageDevice::~ImageDevice() 
{
	delete m_pImage;

	if (m_bKillFilterOnExit)
		delete m_pImageIO;
}
//----------------------------------------------------------------------------------------------
int ImageDevice::GetWidth(void) const { 
	return m_pImage->GetWidth(); 
}
//----------------------------------------------------------------------------------------------
int ImageDevice::GetHeight(void) const { 
	return m_pImage->GetHeight(); 
}
//----------------------------------------------------------------------------------------------
void ImageDevice::BeginFrame(void) { }
//----------------------------------------------------------------------------------------------
void ImageDevice::EndFrame(void) {
	ToneMap();
	m_pImageIO->Save(*m_pImage, m_strFilename);
}
//----------------------------------------------------------------------------------------------
void ImageDevice::Set(int p_nX, int p_nY, const Spectrum &p_spectrum)
{
	RGBPixel pixel(p_spectrum[0], p_spectrum[1], p_spectrum[2]);
	m_pImage->Set(p_nX, p_nY, pixel);
}
//----------------------------------------------------------------------------------------------
void ImageDevice::Set(float p_fX, float p_fY, const Spectrum &p_spectrum) {
	Set((int)p_fX, (int)p_fY, p_spectrum); 
}
//----------------------------------------------------------------------------------------------
// TODO:
// Add formal tone mapping subsystem and parameterise TMOs for image devices
void ImageDevice::ToneMap(void)
{
	RGBPixel Lw(0), Ld;

	for (int y = 0; y < m_pImage->GetHeight(); y++)
	{
		for (int x = 0; x < m_pImage->GetWidth(); x++)
		{
			RGBPixel pixel = m_pImage->Get(x,y);

			Lw.R += Maths::Log(pixel.R + Maths::Epsilon);
			Lw.G += Maths::Log(pixel.G + Maths::Epsilon);
			Lw.B += Maths::Log(pixel.B + Maths::Epsilon);
		}
	}

	Lw.R = Maths::Exp(1 / (m_pImage->GetWidth() * m_pImage->GetHeight()) * Lw.R);
	Lw.G = Maths::Exp(1 / (m_pImage->GetWidth() * m_pImage->GetHeight()) * Lw.G);
	Lw.B = Maths::Exp(1 / (m_pImage->GetWidth() * m_pImage->GetHeight()) * Lw.B);

	for (int y = 0; y < m_pImage->GetHeight(); y++)
	{
		for (int x = 0; x < m_pImage->GetWidth(); x++)
		{
			Ld = m_pImage->Get(x,y);

			Ld.R = HDRToLDR(Ld.R * (0.18f / Lw.R));
			Ld.G = HDRToLDR(Ld.G * (0.18f / Lw.G));
			Ld.B = HDRToLDR(Ld.B * (0.18f / Lw.B));
			
			m_pImage->Set(x,y, Ld);
		}
	}
}
//----------------------------------------------------------------------------------------------
float ImageDevice::HDRToLDR(float p_fValue)
{
	return (float)(p_fValue / (p_fValue + 1));
}
//----------------------------------------------------------------------------------------------
