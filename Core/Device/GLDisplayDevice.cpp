//----------------------------------------------------------------------------------------------
//	Filename:	GLDisplayDevice.cpp
//	Author:		Keith Bugeja
//	Date:		08/07/2013
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Image/Image.h"
#include "Spectrum/Spectrum.h"
#include "Device/GLDisplayDevice.h"

//----------------------------------------------------------------------------------------------
// #include <GL/glfw.h>
#pragma comment(lib, "opengl32.lib")
// #pragma comment(lib, "glu32.lib")

//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
GLDisplayDevice::GLDisplayDevice(const std::string &p_strName, 
	int p_nWidth, int p_nHeight)	
	: IDevice(p_strName) 
	, m_nActiveBuffer(0)
	, m_bIsOpen(false)
{
	m_pImage[0] = new Image(p_nWidth, p_nHeight);
	m_pImage[1] = new Image(p_nWidth, p_nHeight);

	m_pFrontBuffer = m_pImage[m_nActiveBuffer];
	m_pBackBuffer = m_pImage[1 - m_nActiveBuffer];

	m_pSurface[0] = new float[p_nWidth * p_nHeight * 3];
	m_pSurface[1] = new float[p_nWidth * p_nHeight * 3];

	m_pFrontSurface = m_pSurface[m_nActiveBuffer];
	m_pBackSurface = m_pSurface[1 - m_nActiveBuffer];
}
//----------------------------------------------------------------------------------------------
GLDisplayDevice::GLDisplayDevice(int p_nWidth, int p_nHeight)
	: m_nActiveBuffer(0)
	, m_bIsOpen(false)
{
	m_pImage[0] = new Image(p_nWidth, p_nHeight);
	m_pImage[1] = new Image(p_nWidth, p_nHeight);

	m_pFrontBuffer = m_pImage[m_nActiveBuffer];
	m_pBackBuffer = m_pImage[1 - m_nActiveBuffer];

	m_pSurface[0] = new float[p_nWidth * p_nHeight * 3];
	m_pSurface[1] = new float[p_nWidth * p_nHeight * 3];

	m_pFrontSurface = m_pSurface[m_nActiveBuffer];
	m_pBackSurface = m_pSurface[1 - m_nActiveBuffer];
}
//----------------------------------------------------------------------------------------------
GLDisplayDevice::~GLDisplayDevice() 
{
	Safe_Delete(m_pImage[0]);
	Safe_Delete(m_pImage[1]);

	Safe_Delete(m_pSurface[0]);
	Safe_Delete(m_pSurface[1]);
}
//----------------------------------------------------------------------------------------------
int GLDisplayDevice::GetWidth(void) const { 
	return m_pBackBuffer->GetWidth(); 
}
//----------------------------------------------------------------------------------------------
int GLDisplayDevice::GetHeight(void) const { 
	return m_pBackBuffer->GetHeight(); 
}
//----------------------------------------------------------------------------------------------
IDevice::AccessType GLDisplayDevice::GetAccessType(void) const {
	return IDevice::ReadWrite;
}
//----------------------------------------------------------------------------------------------
bool GLDisplayDevice::Open(void) 
{
	// If device is already open, return error
	if (m_bIsOpen) return false;

	// Initialise
	if (glfwInit() && 
		glfwOpenWindow(GetWidth(), GetHeight(), 8, 8, 8, 0, 0, 0, GLFW_WINDOW))
	{
		glfwSetWindowTitle("Illumina PRT GLDisplayDevice");

		glGenTextures(1, &m_gluiFramebufferTexture);
		glBindTexture(GL_TEXTURE_2D, m_gluiFramebufferTexture);

		// when texture area is small, bilinear filter the closest mipmap
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);

		glEnable(GL_TEXTURE_2D);

		glShadeModel(GL_SMOOTH);
		glClearColor(0.5, 0.0, 0.5, 0.0);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
 
		glBindTexture(GL_TEXTURE_2D, m_gluiFramebufferTexture);
	}
	else
	{
		glfwTerminate();
		return false;
	}

	// Start streaming feed
	m_bIsStreaming = true;

	// Device is open
	m_bIsOpen = true;

	return true;
}
//----------------------------------------------------------------------------------------------
bool GLDisplayDevice::IsStreaming(void) const {
	return m_bIsStreaming;
}
//----------------------------------------------------------------------------------------------
void GLDisplayDevice::Stream(void) 
{
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_pFrontBuffer->GetWidth(), m_pFrontBuffer->GetHeight(), 0, GL_RGB, GL_FLOAT, (void*)m_pFrontSurface);

	glBegin(GL_QUADS);
	glTexCoord2d(1, 1); glVertex3f(-1.0, -1.0, -1.0);
	glTexCoord2d(1, 0); glVertex3f(-1.0, 1.0, -1.0);
	glTexCoord2d(0, 0); glVertex3f(1.0, 1.0, -1.0);
	glTexCoord2d(0, 1); glVertex3f(1.0, -1.0, -1.0);
	glEnd();
 
	glfwSwapBuffers();
}
//----------------------------------------------------------------------------------------------
void GLDisplayDevice::Close(void) 
{ 
	// Cannot close device if not open!
	if (m_bIsOpen)
	{
		// Stop streaming
		m_bIsStreaming = false;

		// Delete allocated texture
		glDeleteTextures(1, &m_gluiFramebufferTexture);

		// Close window
		glfwCloseWindow();

		// Terminate
		glfwTerminate();

		// Device is no longer open
		m_bIsOpen = false;
	}
}
//----------------------------------------------------------------------------------------------
void GLDisplayDevice::BeginFrame(void) { }
//----------------------------------------------------------------------------------------------
void GLDisplayDevice::EndFrame(void)  
{
	// Double buffering swap
	m_nActiveBuffer = 1 - m_nActiveBuffer;

	m_pFrontBuffer = m_pImage[m_nActiveBuffer];
	m_pBackBuffer = m_pImage[1 - m_nActiveBuffer];

	m_pFrontSurface = m_pSurface[m_nActiveBuffer];
	m_pBackSurface = m_pSurface[1 - m_nActiveBuffer];

	//!!!!!
	this->Stream();
}
//----------------------------------------------------------------------------------------------
void GLDisplayDevice::Set(int p_nX, int p_nY, const Spectrum &p_spectrum) {
	RGBPixel pixel(p_spectrum[0], p_spectrum[1], p_spectrum[2]);
	m_pBackBuffer->Set(p_nX, p_nY, pixel);
	
	float *component = m_pBackSurface + (p_nX + p_nY * GetWidth()) * 3;
	*component++ = pixel.R;
	*component++ = pixel.G;
	*component = pixel.B;
}
//----------------------------------------------------------------------------------------------
void GLDisplayDevice::Set(float p_fX, float p_fY, const Spectrum &p_spectrum) {
	Set((int)p_fX, (int)p_fY, p_spectrum); 
}
//----------------------------------------------------------------------------------------------
void GLDisplayDevice::Get(int p_nX, int p_nY, Spectrum &p_spectrum) const
{
	const RGBPixel pixel = m_pBackBuffer->Get(p_nX, p_nY);
	p_spectrum.Set(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
void GLDisplayDevice::Get(float p_fX, float p_fY, Spectrum &p_spectrum) const
{
	const RGBPixel pixel = m_pBackBuffer->Get((int)p_fX, (int)p_fY);
	p_spectrum.Set(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
Spectrum GLDisplayDevice::Get(int p_nX, int p_nY) const
{
	const RGBPixel pixel = m_pBackBuffer->Get(p_nX, p_nY);
	return Spectrum(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
Spectrum GLDisplayDevice::Get(float p_fX, float p_fY) const
{
	const RGBPixel pixel = m_pBackBuffer->Get((int)p_fX, (int)p_fY);
	return Spectrum(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
void GLDisplayDevice::WriteRadianceBufferToDevice(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, 
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
Image *GLDisplayDevice::GetImage(void) const {
	return m_pBackBuffer;
}
//----------------------------------------------------------------------------------------------