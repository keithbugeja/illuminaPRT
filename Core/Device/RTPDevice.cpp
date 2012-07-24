//----------------------------------------------------------------------------------------------
//	Filename:	RTPDevice.cpp
//	Author:		Keith Bugeja
//	Date:		21/07/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Image/Image.h"
#include "Spectrum/Spectrum.h"
#include "Device/RTPDevice.h"

//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
// This method runs on its own thread, to keep feed steady
//----------------------------------------------------------------------------------------------
void RTPDeviceStream(RTPDevice *p_pRTPDevice)
{
	double frameBudget = 0.05; // 1.f / p_pRTPDevice->GetFrameRate();
	double nextDeadline = Platform::ToSeconds(Platform::GetTime());
	
	while(p_pRTPDevice->IsStreaming())
	{
		if ((nextDeadline) <= Platform::ToSeconds(Platform::GetTime()))
		{
			nextDeadline += frameBudget;
			p_pRTPDevice->Stream();
			boost::this_thread::sleep(boost::posix_time::microseconds(5));
		} 
		//else
		//{
		//	double time = nextDeadline - Platform::ToSeconds(Platform::GetTime());
		//	boost::this_thread::sleep(boost::posix_time::microseconds((int64_t)(time * 1e+6)));
		//}
	}
}

//----------------------------------------------------------------------------------------------
RTPDevice::RTPDevice(const std::string &p_strName, 
	int p_nWidth, int p_nHeight, const std::string &p_strAddress, int p_nPort, 
	int p_nFramesPerSecond, IVideoStream::VideoCodec p_videoCodec)	
	: IDevice(p_strName) 
	, m_nActiveBuffer(0)
	, m_strAddress(p_strAddress)
	, m_nPort(p_nPort)
	, m_nFramesPerSecond(p_nFramesPerSecond)
	, m_videoCodec(p_videoCodec)
	, m_bIsOpen(false)
{
	m_pImage[0] = new Image(p_nWidth, p_nHeight);
	m_pImage[1] = new Image(p_nWidth, p_nHeight);

	m_pFrontBuffer = m_pImage[m_nActiveBuffer];
	m_pBackBuffer = m_pImage[1 - m_nActiveBuffer];
}
//----------------------------------------------------------------------------------------------
RTPDevice::RTPDevice(int p_nWidth, int p_nHeight, 
	const std::string &p_strAddress, int p_nPort, 
	int p_nFramesPerSecond, IVideoStream::VideoCodec p_videoCodec)
	: m_nActiveBuffer(0)
	, m_strAddress(p_strAddress)
	, m_nPort(p_nPort)
	, m_nFramesPerSecond(p_nFramesPerSecond)
	, m_videoCodec(p_videoCodec)
	, m_bIsOpen(false)
{
	m_pImage[0] = new Image(p_nWidth, p_nHeight);
	m_pImage[1] = new Image(p_nWidth, p_nHeight);

	m_pFrontBuffer = m_pImage[m_nActiveBuffer];
	m_pBackBuffer = m_pImage[1 - m_nActiveBuffer];
}
//----------------------------------------------------------------------------------------------
RTPDevice::~RTPDevice() 
{
	Safe_Delete(m_pImage[0]);
	Safe_Delete(m_pImage[1]);
}
//----------------------------------------------------------------------------------------------
int RTPDevice::GetWidth(void) const { 
	return m_pBackBuffer->GetWidth(); 
}
//----------------------------------------------------------------------------------------------
int RTPDevice::GetHeight(void) const { 
	return m_pBackBuffer->GetHeight(); 
}
//----------------------------------------------------------------------------------------------
IDevice::AccessType RTPDevice::GetAccessType(void) const {
	return IDevice::ReadWrite;
}
//----------------------------------------------------------------------------------------------
bool RTPDevice::Open(void) 
{
	// If device is already open, return error
	if (m_bIsOpen) return false;

	// Set network address and port
	m_networkVideoStream.SetNetworkAddress(m_strAddress);
	m_networkVideoStream.SetPortNumber(m_nPort);

	// Initialise network stream
	m_networkVideoStream.Initialise(m_pFrontBuffer->GetWidth(), m_pFrontBuffer->GetHeight(), 
		m_nFramesPerSecond, 400000, m_videoCodec);

	// Start streaming feed
	m_bIsStreaming = true;

	m_streamingThread = boost::thread(
		boost::bind(RTPDeviceStream, this)
	);	

	// Device is open
	m_bIsOpen = true;

	return true;
}
//----------------------------------------------------------------------------------------------
bool RTPDevice::IsStreaming(void) const {
	return m_bIsStreaming;
}
//----------------------------------------------------------------------------------------------
void RTPDevice::Stream(void) {
	m_networkVideoStream.Stream(m_pFrontBuffer);
}
//----------------------------------------------------------------------------------------------
void RTPDevice::Close(void) 
{ 
	// Cannot close device if not open!
	if (m_bIsOpen)
	{
		// Stop streaming
		m_bIsStreaming = false;
		m_streamingThread.join();

		// Close stream
		m_networkVideoStream.Shutdown();

		// Device is no longer open
		m_bIsOpen = false;
	}
}
//----------------------------------------------------------------------------------------------
void RTPDevice::BeginFrame(void) { }
//----------------------------------------------------------------------------------------------
void RTPDevice::EndFrame(void)  
{
	// Double buffering swap
	m_nActiveBuffer = 1 - m_nActiveBuffer;
	m_pFrontBuffer = m_pImage[m_nActiveBuffer];
	m_pBackBuffer = m_pImage[1 - m_nActiveBuffer];
}
//----------------------------------------------------------------------------------------------
void RTPDevice::Set(int p_nX, int p_nY, const Spectrum &p_spectrum) {
	m_pBackBuffer->Set(p_nX, p_nY, RGBPixel(p_spectrum[0], p_spectrum[1], p_spectrum[2]));
}
//----------------------------------------------------------------------------------------------
void RTPDevice::Set(float p_fX, float p_fY, const Spectrum &p_spectrum) {
	Set((int)p_fX, (int)p_fY, p_spectrum); 
}
//----------------------------------------------------------------------------------------------
void RTPDevice::Get(int p_nX, int p_nY, Spectrum &p_spectrum) const
{
	const RGBPixel pixel = m_pBackBuffer->Get(p_nX, p_nY);
	p_spectrum.Set(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
void RTPDevice::Get(float p_fX, float p_fY, Spectrum &p_spectrum) const
{
	const RGBPixel pixel = m_pBackBuffer->Get((int)p_fX, (int)p_fY);
	p_spectrum.Set(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
Spectrum RTPDevice::Get(int p_nX, int p_nY) const
{
	const RGBPixel pixel = m_pBackBuffer->Get(p_nX, p_nY);
	return Spectrum(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
Spectrum RTPDevice::Get(float p_fX, float p_fY) const
{
	const RGBPixel pixel = m_pBackBuffer->Get((int)p_fX, (int)p_fY);
	return Spectrum(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
void RTPDevice::WriteRadianceBufferToDevice(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, 
	RadianceBuffer *p_pRadianceBuffer, int p_nDeviceX, int p_nDeviceY)
{
	int width = m_pBackBuffer->GetWidth(),
		height = m_pBackBuffer->GetHeight();

	for (int srcY = p_nRegionY, dstY = p_nDeviceY; srcY < p_pRadianceBuffer->GetHeight(); ++srcY, ++dstY)
	{
		for (int srcX = p_nRegionX, dstX = p_nDeviceX; srcX < p_pRadianceBuffer->GetWidth(); ++srcX, ++dstX)
		{
			this->Set(width - (dstX + 1), height - (dstY + 1), p_pRadianceBuffer->Get(srcX, srcY).Final);
		}
	}
}
//----------------------------------------------------------------------------------------------
IVideoStream::VideoCodec RTPDevice::GetCodec(void) const {
	return m_videoCodec;
}
//----------------------------------------------------------------------------------------------
void RTPDevice::SetCodec(IVideoStream::VideoCodec p_videoCodec)
{
	BOOST_ASSERT(!m_bIsOpen);
	m_videoCodec = p_videoCodec;
}
//----------------------------------------------------------------------------------------------
int RTPDevice::GetFrameRate(void) const {
	return m_nFramesPerSecond;
}
//----------------------------------------------------------------------------------------------
void RTPDevice::SetFrameRate(int p_nFramesPerSecond)
{
	BOOST_ASSERT(!m_bIsOpen);
	m_nFramesPerSecond = p_nFramesPerSecond;
}
//----------------------------------------------------------------------------------------------
int RTPDevice::GetPort(void) const {
	return m_nPort;
}
//----------------------------------------------------------------------------------------------
void RTPDevice::SetPort(int p_nPort)
{
	BOOST_ASSERT(!m_bIsOpen);
	m_nPort = p_nPort;
}
//----------------------------------------------------------------------------------------------
std::string RTPDevice::GetAddress(void) const {
	return m_strAddress;
}
//----------------------------------------------------------------------------------------------
void RTPDevice::SetAddress(const std::string &p_strAddress) 
{
	BOOST_ASSERT(!m_bIsOpen);
	m_strAddress = p_strAddress;
}
//----------------------------------------------------------------------------------------------
Image *RTPDevice::GetImage(void) const {
	return m_pBackBuffer;
}
//----------------------------------------------------------------------------------------------