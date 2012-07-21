//----------------------------------------------------------------------------------------------
//	Filename:	VideoDevice.h
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
		class RTPDevice 
			: public IDevice
		{
		protected:
			// Output network video stream
			NetworkVideoStream m_networkVideoStream;

			// Image pointers used for double bufferring
			Image *m_pImage[2],
				*m_pFrontBuffer,
				*m_pBackBuffer;

			// Denotes active back buffer
			int m_nActiveBuffer;

			// Thread for asynchronous streaming
			boost::thread m_streamingThread;
			bool m_bIsStreaming;

			// Device properties
			IVideoStream::VideoCodec m_videoCodec;
			int m_nFramesPerSecond;

			std::string m_strAddress;
			int m_nPort;

			bool m_bIsOpen;

		public:
			RTPDevice(const std::string &p_strName, int p_nWidth, int p_nHeight, const std::string &p_strAddress, int p_nPort = 6666, int p_nFramesPerSecond = 25, IVideoStream::VideoCodec p_videoCodec = IVideoStream::MPEG1); 
			RTPDevice(int p_nWidth, int p_nHeight, const std::string &p_strAddress, int p_nPort = 6666, int p_nFramesPerSecond = 25, IVideoStream::VideoCodec p_videoCodec = IVideoStream::MPEG1); 
			~RTPDevice(void);

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

			std::string GetFilename(void) const;
			void SetFilename(const std::string &p_strFilename);

			IVideoStream::VideoCodec GetCodec(void) const;
			void SetCodec(IVideoStream::VideoCodec p_videoCodec);

			int GetFrameRate(void) const;
			void SetFrameRate(int p_nFramesPerSecond);

			int GetPort(void) const;
			void SetPort(int p_nPort);

			std::string GetAddress(void) const;
			void SetAddress(const std::string &p_strAddress);

			Image *GetImage(void) const;
		};
	}
}