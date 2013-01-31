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

#include "Device/BufferedImageDevice.h"

#include "Image/Image.h"
#include "Image/ImageIO.h"

#include "Spectrum/Spectrum.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
BufferedImageDevice::BufferedImageDevice(int p_nWidth, int p_nHeight, IImageIO *p_pImageIO, const std::string &p_strFilename, int p_nBufferSize, bool p_bKillFilterOnExit)
	: m_pImage(NULL)
	, m_pImageIO(p_pImageIO)
	, m_nImageCacheBaseFrame(0)
	, m_nImageCacheIndex(0)
	, m_strFilename(p_strFilename)
	, m_strTag()
	, m_bKillFilterOnExit(p_bKillFilterOnExit)
{ 
	ImageCache imageCache;

	for (int frame = 0; frame < p_nBufferSize; frame++)
	{
		imageCache.m_dfTimeStamp = 0;
		imageCache.m_pImage = new Image(p_nWidth, p_nHeight);

		m_imageCacheList.push_back(imageCache);
	}
	
	m_pImage = m_imageCacheList[0].m_pImage;
}
//----------------------------------------------------------------------------------------------
BufferedImageDevice::BufferedImageDevice(const std::string &p_strName, int p_nWidth, int p_nHeight, IImageIO *p_pImageIO, const std::string &p_strFilename, int p_nBufferSize, bool p_bKillFilterOnExit)
	: IDevice(p_strName) 
	, m_pImage(NULL)
	, m_pImageIO(p_pImageIO)
	, m_nImageCacheBaseFrame(0)
	, m_nImageCacheIndex(0)
	, m_strFilename(p_strFilename)
	, m_strTag()
	, m_bKillFilterOnExit(p_bKillFilterOnExit)
{ 
	ImageCache imageCache;

	for (int frame = 0; frame < p_nBufferSize; frame++)
	{
		imageCache.m_dfTimeStamp = 0;
		imageCache.m_pImage = new Image(p_nWidth, p_nHeight);

		m_imageCacheList.push_back(imageCache);
	}
	
	m_pImage = m_imageCacheList[0].m_pImage;
}
//----------------------------------------------------------------------------------------------
BufferedImageDevice::~BufferedImageDevice() 
{
	for (std::vector<ImageCache>::iterator imageIterator = m_imageCacheList.begin(); 
		 imageIterator != m_imageCacheList.end(); imageIterator++)
	{
		delete (*imageIterator).m_pImage;
	}

	m_imageCacheList.clear();

	if (m_bKillFilterOnExit)
		delete m_pImageIO;
}
//----------------------------------------------------------------------------------------------
int BufferedImageDevice::GetWidth(void) const { 
	return m_pImage->GetWidth(); 
}
//----------------------------------------------------------------------------------------------
int BufferedImageDevice::GetHeight(void) const { 
	return m_pImage->GetHeight(); 
}
//----------------------------------------------------------------------------------------------
IDevice::AccessType BufferedImageDevice::GetAccessType(void) const {
	return IDevice::ReadWrite;
}
//----------------------------------------------------------------------------------------------
bool BufferedImageDevice::Open(void)
{ 
	m_nImageCacheBaseFrame = 0;
	m_nImageCacheIndex = 0;

	return true; 
}
//----------------------------------------------------------------------------------------------
void BufferedImageDevice::Close(void) { }
//----------------------------------------------------------------------------------------------
void BufferedImageDevice::BeginFrame(void) 
{
}
//----------------------------------------------------------------------------------------------
void BufferedImageDevice::EndFrame(void)  
{
	// std::cout << "BufferedImage : [" << m_nImageCacheIndex << " of " << m_imageCacheList.size() << "]" << std::endl;

	m_imageCacheList[m_nImageCacheIndex].m_dfTimeStamp = Platform::ToSeconds(Platform::GetTime());

	// Next frame
	if (++m_nImageCacheIndex == m_imageCacheList.size())
	{
		// Commit
		boost::filesystem::path imagePath(m_strFilename);

		// Open timestamps file
		std::ofstream timestamps;
		std::string timestampFilename = (imagePath.parent_path() / (m_strTag + "timestamps.txt")).string();
		timestamps.open(timestampFilename.c_str(), std::ios::ate | std::ofstream::app);

		for (int frame = 0; frame < m_imageCacheList.size(); frame++)
		{
			std::stringstream imageNameStream;
			imageNameStream << m_strTag << std::setfill('0') << std::setw(5) << (frame + m_nImageCacheBaseFrame);
			std::string imageFilename = (imagePath.parent_path() / (imageNameStream.str() + imagePath.extension().string())).string();
			std::string imageTimeStamp = boost::lexical_cast<std::string>(m_imageCacheList[frame].m_dfTimeStamp);
			
			// std::cout << "Persisting : " << imageFilename << " at " << imageTimeStamp << std::endl;

			// Save image
			m_pImageIO->Save(*(m_imageCacheList[frame].m_pImage), imageFilename);

			// Write timestamp
			timestamps << imageFilename << "\t" << imageTimeStamp << std::endl;
		}

		// Close timestamps
		timestamps.close();

		// Next round
		m_nImageCacheIndex = 0;
		m_nImageCacheBaseFrame += m_imageCacheList.size();
	}

	// Assign next slot in buffer
	m_pImage = m_imageCacheList[m_nImageCacheIndex].m_pImage;
}
//----------------------------------------------------------------------------------------------
void BufferedImageDevice::Set(int p_nX, int p_nY, const Spectrum &p_spectrum) {
	m_pImage->Set(p_nX, p_nY, RGBPixel(p_spectrum[0], p_spectrum[1], p_spectrum[2]));
}
//----------------------------------------------------------------------------------------------
void BufferedImageDevice::Set(float p_fX, float p_fY, const Spectrum &p_spectrum) {
	Set((int)p_fX, (int)p_fY, p_spectrum); 
}
//----------------------------------------------------------------------------------------------
void BufferedImageDevice::Get(int p_nX, int p_nY, Spectrum &p_spectrum) const
{
	const RGBPixel pixel = m_pImage->Get(p_nX, p_nY);
	p_spectrum.Set(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
void BufferedImageDevice::Get(float p_fX, float p_fY, Spectrum &p_spectrum) const
{
	// TODO: Provide option to enable bilinear interpolation
	const RGBPixel pixel = m_pImage->Get((int)p_fX, (int)p_fY);
	p_spectrum.Set(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
Spectrum BufferedImageDevice::Get(int p_nX, int p_nY) const
{
	const RGBPixel pixel = m_pImage->Get(p_nX, p_nY);
	return Spectrum(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
Spectrum BufferedImageDevice::Get(float p_fX, float p_fY) const
{
	const RGBPixel pixel = m_pImage->Get((int)p_fX, (int)p_fY);
	return Spectrum(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
void BufferedImageDevice::WriteRadianceBufferToDevice(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, 
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
void BufferedImageDevice::SetTag(const std::string &p_strTag) {
	m_strTag = p_strTag;
}
//----------------------------------------------------------------------------------------------
std::string BufferedImageDevice::GetFilename(void) const {
	return m_strFilename;
}
//----------------------------------------------------------------------------------------------
void BufferedImageDevice::SetFilename(const std::string &p_strFilename) {
	m_strFilename = p_strFilename;
}
//----------------------------------------------------------------------------------------------
IImageIO *BufferedImageDevice::GetImageWriter(void) const {
	return m_pImageIO;
}
//----------------------------------------------------------------------------------------------
void BufferedImageDevice::SetImageWriter(IImageIO *p_pImageIO) {
	m_pImageIO = p_pImageIO;
}
//----------------------------------------------------------------------------------------------
Image *BufferedImageDevice::GetImage(void) const {
	return m_pImage;
}
//----------------------------------------------------------------------------------------------
