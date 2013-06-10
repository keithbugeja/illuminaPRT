//----------------------------------------------------------------------------------------------
//	Filename:	GLDisplayDevice.h
//	Author:		Keith Bugeja
//	Date:		08/06/2013
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------------------------
#include <boost/thread.hpp>
//----------------------------------------------------------------------------------------------
#include "Device/Device.h"
#include <GL/glfw.h>
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class GLDisplayDevice 
			: public IDevice
		{
		protected:
			GLuint m_gluiFramebufferTexture;

			// Image pointers used for double bufferring
			Image *m_pImage[2],
				*m_pFrontBuffer,
				*m_pBackBuffer;

			float *m_pSurface[2],
				*m_pFrontSurface,
				*m_pBackSurface;

			// Denotes active back buffer
			int m_nActiveBuffer;

			// Device properties
			bool m_bIsStreaming,
				m_bIsOpen;

		public:
			GLDisplayDevice(const std::string &p_strName, int p_nWidth, int p_nHeight);
			GLDisplayDevice(int p_nWidth, int p_nHeight);

			~GLDisplayDevice(void);

			//----------------------------------------------------------------------------------------------
			// Interface implementation methods
			//----------------------------------------------------------------------------------------------
			int GetWidth(void) const;
			int GetHeight(void) const;
			
			IDevice::AccessType GetAccessType(void) const;

			bool Open(void);
			void Close(void);

			void BeginFrame(void);
			void EndFrame(void);

			void Set(int p_nX, int p_nY, const Spectrum &p_spectrum);
			void Set(float p_fX, float p_fY, const Spectrum &p_spectrum);

			void Get(int p_nX, int p_nY, Spectrum &p_spectrum) const;
			void Get(float p_fX, float p_fY, Spectrum &p_spectrum) const;

			Spectrum Get(int p_nX, int p_nY) const;
			Spectrum Get(float p_fX, float p_fY) const;

			void WriteRadianceBufferToDevice(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, 
				RadianceBuffer *p_pRadianceBuffer, int p_nDeviceX, int p_nDeviceY);

			//----------------------------------------------------------------------------------------------
			// Type-specific methods
			//----------------------------------------------------------------------------------------------
		public:
			bool IsStreaming(void) const;
			void Stream(void);

			Image *GetImage(void) const;
		};
	}
}