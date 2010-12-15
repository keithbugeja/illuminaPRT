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
ImageDevice::ImageDevice(int p_nWidth, int p_nHeight, IImageIO *p_pImageIO, const std::string &p_strFilename)
	: m_pImage(new Image(p_nWidth, p_nHeight))
	, m_pImageIO(p_pImageIO)
	, m_strFilename(p_strFilename)
{ }
//----------------------------------------------------------------------------------------------
ImageDevice::~ImageDevice() {
	delete m_pImage;
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
	m_pImageIO->Save(*m_pImage, m_strFilename);
}
//----------------------------------------------------------------------------------------------
void ImageDevice::Set(int p_nX, int p_nY, const Spectrum &p_spectrum)
{
	RGBPixel pixel(HDRToLDR(p_spectrum[0]), HDRToLDR(p_spectrum[1]), HDRToLDR(p_spectrum[2]));
	m_pImage->Set(p_nX, p_nY, pixel);
}
//----------------------------------------------------------------------------------------------
void ImageDevice::Set(float p_fX, float p_fY, const Spectrum &p_spectrum) {
	Set((int)p_fX, (int)p_fY, p_spectrum); 
}
//----------------------------------------------------------------------------------------------
float ImageDevice::HDRToLDR(float p_fValue)
{
	return (float)(p_fValue / (p_fValue + 1));
}
//----------------------------------------------------------------------------------------------
