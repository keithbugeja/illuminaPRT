//----------------------------------------------------------------------------------------------
//	Filename:	ImageDevice.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

#include "Device/VideoDevice.h"
#include "Spectrum/Spectrum.h"
#include "Image/Image.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
VideoDevice::VideoDevice(const std::string &p_strName, int p_nWidth, int p_nHeight, const std::string &p_strFilename, int p_nFramesPerSecond, IVideoStream::VideoCodec p_videoCodec)
	: IDevice(p_strName) 
	, m_pImage(new Image(p_nWidth, p_nHeight))
	, m_strFilename(p_strFilename)
	, m_nFramesPerSecond(p_nFramesPerSecond)
	, m_videoCodec(p_videoCodec)
	, m_bIsOpen(false)
{ }
//----------------------------------------------------------------------------------------------
VideoDevice::VideoDevice(int p_nWidth, int p_nHeight, const std::string &p_strFilename, int p_nFramesPerSecond, IVideoStream::VideoCodec p_videoCodec)
	: m_pImage(new Image(p_nWidth, p_nHeight))
	, m_strFilename(p_strFilename)
	, m_nFramesPerSecond(p_nFramesPerSecond)
	, m_videoCodec(p_videoCodec)
	, m_bIsOpen(false)
{ }
//----------------------------------------------------------------------------------------------
VideoDevice::~VideoDevice() 
{
	Safe_Delete(m_pImage);
}
//----------------------------------------------------------------------------------------------
uint32_t VideoDevice::GetWidth(void) const { 
	return m_pImage->GetWidth(); 
}
//----------------------------------------------------------------------------------------------
uint32_t VideoDevice::GetHeight(void) const { 
	return m_pImage->GetHeight(); 
}
//----------------------------------------------------------------------------------------------
IDevice::AccessType VideoDevice::GetAccessType(void) const {
	return IDevice::ReadWrite;
}
//----------------------------------------------------------------------------------------------
bool VideoDevice::Open(void) 
{
	// If device is already open, return error
	if (m_bIsOpen) return false;

	// Set filename
	m_fileVideoStream.SetFilename(m_strFilename);

	// Initialise file stream
	m_fileVideoStream.Initialise(m_pImage->GetWidth(), m_pImage->GetHeight(),
		m_nFramesPerSecond, 400000, m_videoCodec);

	// Device is open
	m_bIsOpen = true;

	return true;
}
//----------------------------------------------------------------------------------------------
void VideoDevice::Close(void) 
{ 
	if (m_bIsOpen)
	{
		// Shutdown device
		m_fileVideoStream.Shutdown();

		// Device is closed
		m_bIsOpen = false;
	}

}
//----------------------------------------------------------------------------------------------
void VideoDevice::BeginFrame(void) { }
//----------------------------------------------------------------------------------------------
void VideoDevice::EndFrame(void)  
{
	// Encode to file output stream
	m_fileVideoStream.Stream(m_pImage);
}
//----------------------------------------------------------------------------------------------
void VideoDevice::Set(int p_nX, int p_nY, const Spectrum &p_spectrum) {
	m_pImage->Set(p_nX, p_nY, RGBPixel(p_spectrum[0], p_spectrum[1], p_spectrum[2]));
}
//----------------------------------------------------------------------------------------------
void VideoDevice::Set(float p_fX, float p_fY, const Spectrum &p_spectrum) {
	Set((int)p_fX, (int)p_fY, p_spectrum); 
}
//----------------------------------------------------------------------------------------------
void VideoDevice::Get(int p_nX, int p_nY, Spectrum &p_spectrum) const
{
	const RGBPixel pixel = m_pImage->Get(p_nX, p_nY);
	p_spectrum.Set(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
void VideoDevice::Get(float p_fX, float p_fY, Spectrum &p_spectrum) const
{
	const RGBPixel pixel = m_pImage->Get((int)p_fX, (int)p_fY);
	p_spectrum.Set(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
Spectrum VideoDevice::Get(int p_nX, int p_nY) const
{
	const RGBPixel pixel = m_pImage->Get(p_nX, p_nY);
	return Spectrum(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
Spectrum VideoDevice::Get(float p_fX, float p_fY) const
{
	const RGBPixel pixel = m_pImage->Get((int)p_fX, (int)p_fY);
	return Spectrum(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
void VideoDevice::WriteRadianceBufferToDevice(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, 
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
IVideoStream::VideoCodec VideoDevice::GetCodec(void) const {
	return m_videoCodec;
}
//----------------------------------------------------------------------------------------------
void VideoDevice::SetCodec(IVideoStream::VideoCodec p_videoCodec)
{
	BOOST_ASSERT(!m_bIsOpen);
	m_videoCodec = p_videoCodec;
}
//----------------------------------------------------------------------------------------------
int VideoDevice::GetFrameRate(void) const {
	return m_nFramesPerSecond;
}
//----------------------------------------------------------------------------------------------
void VideoDevice::SetFrameRate(int p_nFramesPerSecond)
{
	BOOST_ASSERT(!m_bIsOpen);
	m_nFramesPerSecond = p_nFramesPerSecond;
}
//----------------------------------------------------------------------------------------------
std::string VideoDevice::GetFilename(void) const {
	return m_strFilename;
}
//----------------------------------------------------------------------------------------------
void VideoDevice::SetFilename(const std::string &p_strFilename) 
{
	BOOST_ASSERT(!m_bIsOpen);
	m_strFilename = p_strFilename;
}
//----------------------------------------------------------------------------------------------
Image *VideoDevice::GetImage(void) const {
	return m_pImage;
}
//----------------------------------------------------------------------------------------------