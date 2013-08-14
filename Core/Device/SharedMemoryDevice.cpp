//----------------------------------------------------------------------------------------------
//	Filename:	DisplayDevice.cpp
//	Author:		Keith Bugeja
//	Date:		12/07/2013
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Image/Image.h"
#include "Spectrum/Spectrum.h"
#include "Device/SharedMemoryDevice.h"

//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
SharedMemoryDevice::SharedMemoryDevice(const std::string &p_strName, 
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
SharedMemoryDevice::SharedMemoryDevice(int p_nWidth, int p_nHeight)
	: m_nActiveBuffer(0)
	, m_bIsOpen(false)
{
	m_pImage[0] = new Image(p_nWidth, p_nHeight);
	m_pImage[1] = new Image(p_nWidth, p_nHeight);

	m_pFrontBuffer = m_pImage[m_nActiveBuffer];
	m_pBackBuffer = m_pImage[1 - m_nActiveBuffer];
}
//----------------------------------------------------------------------------------------------
SharedMemoryDevice::~SharedMemoryDevice() 
{
	Safe_Delete(m_pImage[0]);
	Safe_Delete(m_pImage[1]);
}
//----------------------------------------------------------------------------------------------
int SharedMemoryDevice::GetWidth(void) const { 
	return m_pBackBuffer->GetWidth(); 
}
//----------------------------------------------------------------------------------------------
int SharedMemoryDevice::GetHeight(void) const { 
	return m_pBackBuffer->GetHeight(); 
}
//----------------------------------------------------------------------------------------------
IDevice::AccessType SharedMemoryDevice::GetAccessType(void) const {
	return IDevice::ReadWrite;
}
//----------------------------------------------------------------------------------------------
bool SharedMemoryDevice::Open(void) 
{
	// If device is already open, return error
	if (m_bIsOpen) return false;

	int segmentSize = m_pFrontBuffer->GetWidth() * m_pFrontBuffer->GetHeight() * 3;

	#if (defined __PLATFORM_WINDOWS__)
	// Create a native windows shared memory object.
	m_pSharedMemorySink = new boost::interprocess::windows_shared_memory(
		boost::interprocess::create_only, "IlluminaPRT_OutputSink", 
		boost::interprocess::read_write, segmentSize);
	#else
	// Create shared memory object for POSIX compliant systems
	// ...
	#endif
	
	// Map the whole shared memory in this process
	m_pSharedMemoryRegion = new boost::interprocess::mapped_region(
		*m_pSharedMemorySink, boost::interprocess::read_write);

	// Clear shm object
	std::memset(m_pSharedMemoryRegion->get_address(), 0, m_pSharedMemoryRegion->get_size());

	// Start streaming feed
	m_bIsStreaming = true;

	// Device is open
	m_bIsOpen = true;

	return true;
}
//----------------------------------------------------------------------------------------------
bool SharedMemoryDevice::IsStreaming(void) const {
	return m_bIsStreaming;
}
//----------------------------------------------------------------------------------------------
void SharedMemoryDevice::Stream(void) 
{
	RGBPixel4F *src;
	unsigned char *dst;
	int index;

	for (src = m_pFrontBuffer->GetSurfaceBuffer(),
		 dst = (unsigned char*)m_pSharedMemoryRegion->get_address(),
		 index = 0; index < m_pFrontBuffer->GetArea(); index++, src++)
	{
		*dst++ = (unsigned char)(255 * src->R);
		*dst++ = (unsigned char)(255 * src->G);
		*dst++ = (unsigned char)(255 * src->G);
	}
}
//----------------------------------------------------------------------------------------------
void SharedMemoryDevice::Close(void) 
{ 
	// Cannot close device if not open!
	if (m_bIsOpen)
	{
		// Stop streaming
		m_bIsStreaming = false;

		// Close stream
		delete m_pSharedMemoryRegion;
		delete m_pSharedMemorySink;

		// Device is no longer open
		m_bIsOpen = false;
	}
}
//----------------------------------------------------------------------------------------------
void SharedMemoryDevice::BeginFrame(void) { }
//----------------------------------------------------------------------------------------------
void SharedMemoryDevice::EndFrame(void)  
{
	// Double buffering swap
	m_nActiveBuffer = 1 - m_nActiveBuffer;
	m_pFrontBuffer = m_pImage[m_nActiveBuffer];
	m_pBackBuffer = m_pImage[1 - m_nActiveBuffer];

	//!!!!!
	this->Stream();
}
//----------------------------------------------------------------------------------------------
void SharedMemoryDevice::Set(int p_nX, int p_nY, const Spectrum &p_spectrum) {
	m_pBackBuffer->Set(p_nX, p_nY, RGBPixel(p_spectrum[0], p_spectrum[1], p_spectrum[2]));
}
//----------------------------------------------------------------------------------------------
void SharedMemoryDevice::Set(float p_fX, float p_fY, const Spectrum &p_spectrum) {
	Set((int)p_fX, (int)p_fY, p_spectrum); 
}
//----------------------------------------------------------------------------------------------
void SharedMemoryDevice::Get(int p_nX, int p_nY, Spectrum &p_spectrum) const
{
	const RGBPixel pixel = m_pBackBuffer->Get(p_nX, p_nY);
	p_spectrum.Set(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
void SharedMemoryDevice::Get(float p_fX, float p_fY, Spectrum &p_spectrum) const
{
	const RGBPixel pixel = m_pBackBuffer->Get((int)p_fX, (int)p_fY);
	p_spectrum.Set(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
Spectrum SharedMemoryDevice::Get(int p_nX, int p_nY) const
{
	const RGBPixel pixel = m_pBackBuffer->Get(p_nX, p_nY);
	return Spectrum(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
Spectrum SharedMemoryDevice::Get(float p_fX, float p_fY) const
{
	const RGBPixel pixel = m_pBackBuffer->Get((int)p_fX, (int)p_fY);
	return Spectrum(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
void SharedMemoryDevice::WriteRadianceBufferToDevice(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, 
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
Image *SharedMemoryDevice::GetImage(void) const {
	return m_pBackBuffer;
}
//----------------------------------------------------------------------------------------------