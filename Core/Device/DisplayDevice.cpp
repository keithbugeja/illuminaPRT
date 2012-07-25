//----------------------------------------------------------------------------------------------
//	Filename:	DisplayDevice.cpp
//	Author:		Keith Bugeja
//	Date:		21/07/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Image/Image.h"
#include "Spectrum/Spectrum.h"
#include "Device/DisplayDevice.h"

//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
DisplayDevice::DisplayDevice(const std::string &p_strName, 
	int p_nWidth, int p_nHeight)	
	: IDevice(p_strName) 
	, m_nActiveBuffer(0)
	, m_bIsOpen(false)
{
	m_pImage[0] = new Image(p_nWidth, p_nHeight);
	m_pImage[1] = new Image(p_nWidth, p_nHeight);

	m_pFrontBuffer = m_pImage[m_nActiveBuffer];
	m_pBackBuffer = m_pImage[1 - m_nActiveBuffer];
}
//----------------------------------------------------------------------------------------------
DisplayDevice::DisplayDevice(int p_nWidth, int p_nHeight)
	: m_nActiveBuffer(0)
	, m_bIsOpen(false)
{
	m_pImage[0] = new Image(p_nWidth, p_nHeight);
	m_pImage[1] = new Image(p_nWidth, p_nHeight);

	m_pFrontBuffer = m_pImage[m_nActiveBuffer];
	m_pBackBuffer = m_pImage[1 - m_nActiveBuffer];
}
//----------------------------------------------------------------------------------------------
DisplayDevice::~DisplayDevice() 
{
	Safe_Delete(m_pImage[0]);
	Safe_Delete(m_pImage[1]);
}
//----------------------------------------------------------------------------------------------
int DisplayDevice::GetWidth(void) const { 
	return m_pBackBuffer->GetWidth(); 
}
//----------------------------------------------------------------------------------------------
int DisplayDevice::GetHeight(void) const { 
	return m_pBackBuffer->GetHeight(); 
}
//----------------------------------------------------------------------------------------------
IDevice::AccessType DisplayDevice::GetAccessType(void) const {
	return IDevice::ReadWrite;
}
//----------------------------------------------------------------------------------------------
bool DisplayDevice::Open(void) 
{
	// If device is already open, return error
	if (m_bIsOpen) return false;

	// Initialise network stream
	m_displayVideoStream.Initialise(m_pFrontBuffer->GetWidth(), m_pFrontBuffer->GetHeight(), 
		30, 8192, IVideoStream::Dummy);

	// Start streaming feed
	m_bIsStreaming = true;

	// Device is open
	m_bIsOpen = true;

	return true;
}
//----------------------------------------------------------------------------------------------
bool DisplayDevice::IsStreaming(void) const {
	return m_bIsStreaming;
}
//----------------------------------------------------------------------------------------------
void DisplayDevice::Stream(void) {
	m_displayVideoStream.Stream(m_pFrontBuffer);
}
//----------------------------------------------------------------------------------------------
void DisplayDevice::Close(void) 
{ 
	// Cannot close device if not open!
	if (m_bIsOpen)
	{
		// Stop streaming
		m_bIsStreaming = false;

		// Close stream
		m_displayVideoStream.Shutdown();

		// Device is no longer open
		m_bIsOpen = false;
	}
}
//----------------------------------------------------------------------------------------------
void DisplayDevice::BeginFrame(void) { }
//----------------------------------------------------------------------------------------------
void DisplayDevice::EndFrame(void)  
{
	// Double buffering swap
	m_nActiveBuffer = 1 - m_nActiveBuffer;
	m_pFrontBuffer = m_pImage[m_nActiveBuffer];
	m_pBackBuffer = m_pImage[1 - m_nActiveBuffer];

	//!!!!!
	this->Stream();
}
//----------------------------------------------------------------------------------------------
void DisplayDevice::Set(int p_nX, int p_nY, const Spectrum &p_spectrum) {
	m_pBackBuffer->Set(p_nX, p_nY, RGBPixel(p_spectrum[0], p_spectrum[1], p_spectrum[2]));
}
//----------------------------------------------------------------------------------------------
void DisplayDevice::Set(float p_fX, float p_fY, const Spectrum &p_spectrum) {
	Set((int)p_fX, (int)p_fY, p_spectrum); 
}
//----------------------------------------------------------------------------------------------
void DisplayDevice::Get(int p_nX, int p_nY, Spectrum &p_spectrum) const
{
	const RGBPixel pixel = m_pBackBuffer->Get(p_nX, p_nY);
	p_spectrum.Set(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
void DisplayDevice::Get(float p_fX, float p_fY, Spectrum &p_spectrum) const
{
	const RGBPixel pixel = m_pBackBuffer->Get((int)p_fX, (int)p_fY);
	p_spectrum.Set(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
Spectrum DisplayDevice::Get(int p_nX, int p_nY) const
{
	const RGBPixel pixel = m_pBackBuffer->Get(p_nX, p_nY);
	return Spectrum(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
Spectrum DisplayDevice::Get(float p_fX, float p_fY) const
{
	const RGBPixel pixel = m_pBackBuffer->Get((int)p_fX, (int)p_fY);
	return Spectrum(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
void DisplayDevice::WriteRadianceBufferToDevice(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, 
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
Image *DisplayDevice::GetImage(void) const {
	return m_pBackBuffer;
}
//----------------------------------------------------------------------------------------------