//----------------------------------------------------------------------------------------------
//	Filename:	ImageDevice.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iomanip>
#include <sstream>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include "Device/ImageDevice.h"

#include "Image/Image.h"
#include "Image/ImageIO.h"

#include "Spectrum/Spectrum.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
ImageDevice::ImageDevice(int p_nWidth, int p_nHeight, bool p_bTimeStamp, IImageIO *p_pImageIO, const std::string &p_strFilename, bool p_bKillFilterOnExit)
	: m_pImage(new Image(p_nWidth, p_nHeight))
	, m_pImageIO(p_pImageIO)
	, m_nFrameNumber(0)
	, m_bTimeStamp(p_bTimeStamp)
	, m_strFilename(p_strFilename)
	, m_bKillFilterOnExit(p_bKillFilterOnExit)
{ }
//----------------------------------------------------------------------------------------------
ImageDevice::ImageDevice(const std::string &p_strName, int p_nWidth, int p_nHeight, bool p_bTimeStamp, IImageIO *p_pImageIO, const std::string &p_strFilename, bool p_bKillFilterOnExit)
	: IDevice(p_strName) 
	, m_pImage(new Image(p_nWidth, p_nHeight))
	, m_pImageIO(p_pImageIO)
	, m_nFrameNumber(0)
	, m_bTimeStamp(p_bTimeStamp)
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
bool ImageDevice::Open(void) { 
	return true; 
}
//----------------------------------------------------------------------------------------------
void ImageDevice::Close(void) { }
//----------------------------------------------------------------------------------------------
void ImageDevice::BeginFrame(void) { }
//----------------------------------------------------------------------------------------------
void ImageDevice::EndFrame(void)  
{
	if (m_bTimeStamp) 
	{
		m_nFrameNumber++;

		boost::filesystem::path imagePath(m_strFilename);
		
		std::stringstream imageNameStream;
		imageNameStream << std::setfill('0') << std::setw(5) << m_nFrameNumber;
		std::string imageFilename = (imagePath.parent_path() / (imageNameStream.str() + imagePath.extension().string())).string();
		std::string imageTimeStamp = boost::lexical_cast<std::string>(Platform::ToSeconds(Platform::GetTime()));

		
		std::ofstream timestamps;
		std::string timestampFilename = (imagePath.parent_path() / "timestamps.txt").string();
		timestamps.open(timestampFilename.c_str(), std::ios::ate | std::ofstream::app);
		timestamps << imageFilename << "\t" << imageTimeStamp << std::endl;
		timestamps.close();

		/*
		boost::filesystem::path filenamePath(m_strFilename);
		std::string timestampedFilename = m_strFilename + 
			"[" + boost::lexical_cast<std::string>(Platform::ToSeconds(Platform::GetTime())) + "]" + filenamePath.extension().string();
		*/

		// std::string timestampedFilename = m_strFilename + "_" + ;
		std::cout<<"Commit :: " << imageFilename << std::endl;
		m_pImageIO->Save(*m_pImage, imageFilename);
	} else {
		std::cout<<"Commit :: " << m_strFilename << std::endl;
		m_pImageIO->Save(*m_pImage, m_strFilename);
	}
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
