//----------------------------------------------------------------------------------------------
//	Filename:	DisplayDevice.h
//	Author:		Keith Bugeja
//	Date:		20/07/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------------------------
#include <boost/thread.hpp>
//----------------------------------------------------------------------------------------------
#include "Device/Device.h"
#include "External/Video/VideoStream.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class DisplayDevice 
			: public IDevice
		{
		protected:
			// Output display sink
			DisplayVideoStream m_displayVideoStream;

			// Image pointers used for double bufferring
			Image *m_pImage[2],
				*m_pFrontBuffer,
				*m_pBackBuffer;

			// Denotes active back buffer
			int m_nActiveBuffer;

			// Device properties
			bool m_bIsStreaming,
				m_bIsOpen;

		public:
			DisplayDevice(const std::string &p_strName, int p_nWidth, int p_nHeight);
			DisplayDevice(int p_nWidth, int p_nHeight);

			~DisplayDevice(void);

			//----------------------------------------------------------------------------------------------
			// Interface implementation methods
			//----------------------------------------------------------------------------------------------
			uint32_t GetWidth(void) const;
			uint32_t GetHeight(void) const;
			
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