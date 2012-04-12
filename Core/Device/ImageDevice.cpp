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
IDevice::AccessType ImageDevice::GetAccessType(void) const {
	return IDevice::ReadWrite;
}
//----------------------------------------------------------------------------------------------
void ImageDevice::BeginFrame(void) { }
//----------------------------------------------------------------------------------------------
void ImageDevice::EndFrame(void)  {
	m_pImageIO->Save(*m_pImage, m_strFilename);
}
//----------------------------------------------------------------------------------------------
void ImageDevice::Set(int p_nX, int p_nY, const Spectrum &p_spectrum) {
	m_pImage->Set(p_nX, p_nY, RGBPixel(p_spectrum[0], p_spectrum[1], p_spectrum[2]));
}
//----------------------------------------------------------------------------------------------
void ImageDevice::Set(float p_fX, float p_fY, const Spectrum &p_spectrum) {
	Set((int)p_fX, (int)p_fY, p_spectrum); 
}
//----------------------------------------------------------------------------------------------
void ImageDevice::Get(int p_nX, int p_nY, Spectrum &p_spectrum) const
{
	const RGBPixel pixel = m_pImage->Get(p_nX, p_nY);
	p_spectrum.Set(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
void ImageDevice::Get(float p_fX, float p_fY, Spectrum &p_spectrum) const
{
	// TODO: Provide option to enable bilinear interpolation
	const RGBPixel pixel = m_pImage->Get((int)p_fX, (int)p_fY);
	p_spectrum.Set(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
Spectrum ImageDevice::Get(int p_nX, int p_nY) const
{
	const RGBPixel pixel = m_pImage->Get(p_nX, p_nY);
	return Spectrum(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
Spectrum ImageDevice::Get(float p_fX, float p_fY) const
{
	const RGBPixel pixel = m_pImage->Get((int)p_fX, (int)p_fY);
	return Spectrum(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
void ImageDevice::WriteRadianceBufferToDevice(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, 
	RadianceBuffer *p_pRadianceBuffer, int p_nDeviceX, int p_nDeviceY)
{
	int width = m_pImage->GetWidth(),
		height = m_pImage->GetHeight();

	for (int srcY = p_nRegionY, dstY = p_nDeviceY; srcY < p_pRadianceBuffer->GetHeight(); ++srcY, ++dstY)
	{
		for (int srcX = p_nRegionX, dstX = p_nDeviceX; srcX < p_pRadianceBuffer->GetWidth(); ++srcX, ++dstX)
		{
			this->Set(width - (dstX + 1), height - (dstY + 1), p_pRadianceBuffer->Get(srcX, srcY).Final);
		}
	}
}
//----------------------------------------------------------------------------------------------
std::string ImageDevice::GetFilename(void) const {
	return m_strFilename;
}
//----------------------------------------------------------------------------------------------
void ImageDevice::SetFilename(const std::string &p_strFilename) {
	m_strFilename = p_strFilename;
}
//----------------------------------------------------------------------------------------------
IImageIO *ImageDevice::GetImageWriter(void) const {
	return m_pImageIO;
}
//----------------------------------------------------------------------------------------------
void ImageDevice::SetImageWriter(IImageIO *p_pImageIO) {
	m_pImageIO = p_pImageIO;
}
//----------------------------------------------------------------------------------------------
Image *ImageDevice::GetImage(void) const {
	return m_pImage;
}
//----------------------------------------------------------------------------------------------
