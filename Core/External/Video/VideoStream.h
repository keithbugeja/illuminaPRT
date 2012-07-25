//----------------------------------------------------------------------------------------------
//	Filename:	VideoStream.h
//	Author:		Keith Bugeja
//	Date:		20/07/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include "Spectrum/Spectrum.h"
#include "Image/Image.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		// IVideoStream
		//----------------------------------------------------------------------------------------------
		class IVideoStream
		{
		public:
			enum VideoCodec
			{
				Dummy,
				MPEG1,
				MPEG2,
				MPEG4,
				H264,
				VP8
			};

			enum StreamType
			{
				Display,
				Network,
				Disk
			};

		protected:
			virtual void ConvertImageToFrame(Image *p_pImage, void *p_pFrame);
		
		public:
			static std::string GetFourCC(VideoCodec p_videoCodec);
			static VideoCodec GetCodec(const std::string &p_strFourCC);

		public:
			~IVideoStream(void) { }

			virtual bool Initialise(int p_nWidth, int p_nHeight, int p_nFramesPerSecond = 25, int p_nBitRate = 400000, VideoCodec p_videoCodec = IVideoStream::MPEG1) = 0;
			virtual void Stream(Image *p_pImage) = 0;
			virtual void Shutdown(void) = 0;

			virtual StreamType GetStreamType(void) const = 0;
		};

		//----------------------------------------------------------------------------------------------
		// VideoStreamState
		//----------------------------------------------------------------------------------------------
		struct VideoStreamState;

		//----------------------------------------------------------------------------------------------
		// NetworkVideoStream
		//----------------------------------------------------------------------------------------------
		class DisplayVideoStream
			: public IVideoStream
		{
		protected:
			VideoStreamState *m_pVideoStreamState;

		public:
			DisplayVideoStream(void);
			~DisplayVideoStream(void);

		public:
			bool Initialise(int p_nWidth, int p_nHeight, int p_nFramesPerSecond, int p_nBitRate, VideoCodec p_videoCodec);
			void Stream(Image *p_pImage);
			void Shutdown(void);

			StreamType GetStreamType(void) const;
		};

		//----------------------------------------------------------------------------------------------
		// NetworkVideoStream
		//----------------------------------------------------------------------------------------------
		class NetworkVideoStream
			: public IVideoStream
		{
		protected:
			VideoStreamState *m_pVideoStreamState;
			
			int m_nPort;
			std::string m_strAddress;

		public:
			NetworkVideoStream(const std::string &p_strNetworkAddress, int p_nPortNumber);
			NetworkVideoStream(void);
			~NetworkVideoStream(void);

			std::string GetNetworkAddress(void) const;
			void SetNetworkAddress(const std::string &p_strNetworkAddress);

			int GetPortNumber(void) const;
			void SetPortNumber(int p_nPortNumber);

		public:
			bool Initialise(int p_nWidth, int p_nHeight, int p_nFramesPerSecond, int p_nBitRate, VideoCodec p_videoCodec);
			void Stream(Image *p_pImage);
			void Shutdown(void);

			StreamType GetStreamType(void) const;
		};

		//----------------------------------------------------------------------------------------------
		// FileVideoStream
		//----------------------------------------------------------------------------------------------
		class FileVideoStream
			: public IVideoStream
		{
		protected:
			VideoStreamState *m_pVideoStreamState;

			FILE *m_videoFile;			
			std::string m_strFilename;

		public:
			FileVideoStream(const std::string &p_strFilename);
			FileVideoStream(void);
			~FileVideoStream(void);

			std::string GetFilename(void) const;
			void SetFilename(const std::string &p_strFilename);

		public:
			bool Initialise(int p_nWidth, int p_nHeight, int p_nFramesPerSecond, int p_nBitRate, VideoCodec p_videoCodec);
			void Stream(Image *p_pImage);
			void Shutdown(void);

			StreamType GetStreamType(void) const;
		};
	}
}