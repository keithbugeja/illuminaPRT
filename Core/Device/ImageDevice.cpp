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
			
			//Ld.R = HDRToLDR(Ld.R);
			//Ld.G = HDRToLDR(Ld.G);
			//Ld.B = HDRToLDR(Ld.B);
			
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
void ImageDevice::WriteToBuffer(RGBBytePixel *p_buffer)
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
			
			//Ld.R = HDRToLDR(Ld.R);
			//Ld.G = HDRToLDR(Ld.G);
			//Ld.B = HDRToLDR(Ld.B);
			
			p_buffer->R = Ld.R * 255;
			p_buffer->G = Ld.G * 255;
			p_buffer->B = Ld.B * 255;

			p_buffer++;
		}
	}
}