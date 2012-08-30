//----------------------------------------------------------------------------------------------
//	Filename:	VideoDevice.h
//	Author:		Keith Bugeja
//	Date:		20/07/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Device/Device.h"
#include "External/Video/VideoStream.h"
//----------------------------------------------------------------------------------------------

namespace Illumina
{
	namespace Core
	{
		class VideoDevice 
			: public IDevice
		{
		protected:
			// Output file video stream
			FileVideoStream m_fileVideoStream;

			// Image, storing last render
			Image *m_pImage;

			// Device properties
			IVideoStream::VideoCodec m_videoCodec;
			int m_nFramesPerSecond;

			std::string m_strFilename;

			bool m_bIsOpen;

		public:
			VideoDevice(const std::string &p_strName, int p_nWidth, int p_nHeight, const std::string &p_strFilename, int p_nFramesPerSecond = 25, IVideoStream::VideoCodec p_videoCodec = IVideoStream::MPEG1); 
			VideoDevice(int p_nWidth, int p_nHeight, const std::string &p_strFilename, int p_nFramesPerSecond = 25, IVideoStream::VideoCodec p_videoCodec = IVideoStream::MPEG1); 
			~VideoDevice(void);

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
			std::string GetFilename(void) const;
			void SetFilename(const std::string &p_strFilename);

			IVideoStream::VideoCodec GetCodec(void) const;
			void SetCodec(IVideoStream::VideoCodec p_videoCodec);

			int GetFrameRate(void) const;
			void SetFrameRate(int p_nFramesPerSecond);

			Image *GetImage(void) const;
		};
	}
}