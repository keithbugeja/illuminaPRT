//----------------------------------------------------------------------------------------------
//	Filename:	VideoDevice.h
//	Author:		Keith Bugeja
//	Date:		20/07/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Device/Device.h"

//----------------------------------------------------------------------------------------------

struct AVCodec;
struct AVCodecContext;
struct AVFrame;

//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class VideoDevice 
			: public IDevice
		{
			// FFMpeg stuff
		protected:
			AVCodec *m_pCodec;
			AVCodecContext *m_pCodecContext;
			AVFrame *m_pPicture;

			int m_nOutputSize,
				m_nOutputBufferSize,
				m_nHadOutput;

			uint8_t *m_pOutputBuffer;

			FILE *m_videoFile;
		
		public:
			enum VideoCodec
			{
				MPEG1,
				MPEG2,
				H264,
				VP8
			};

		protected:
			Image *m_pImage;

			std::string m_strFilename;
			VideoCodec m_videoCodec;
			int m_nFramesPerSecond;

			bool m_bIsOpen;

		public:
			VideoDevice(const std::string &p_strName, int p_nWidth, int p_nHeight, const std::string &p_strFilename, int p_nFramesPerSecond = 25, VideoCodec p_videoCodec = MPEG1); 
			VideoDevice(int p_nWidth, int p_nHeight, const std::string &p_strFilename, int p_nFramesPerSecond = 25, VideoCodec p_videoCodec = MPEG1); 
			~VideoDevice(void);

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
			std::string GetFilename(void) const;
			void SetFilename(const std::string &p_strFilename);

			VideoCodec GetCodec(void) const;
			void SetCodec(VideoCodec p_videoCodec);

			int GetFrameRate(void) const;
			void SetFrameRate(int p_nFramesPerSecond);

			Image *GetImage(void) const;
		};
	}
}