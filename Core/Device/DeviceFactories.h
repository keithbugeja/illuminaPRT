//----------------------------------------------------------------------------------------------
//	Filename:	DeviceFactories.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <map>
#include <string>
#include <iostream>

#include "Device/Device.h"
#include "Device/ImageDevice.h"
#include "Device/VideoDevice.h"
#include "Device/RTPDevice.h"
#include "Image/ImagePPM.h"

namespace Illumina
{
	namespace Core
	{
		class RTPDeviceFactory : public Illumina::Core::Factory<Illumina::Core::IDevice>
		{
		public:
			Illumina::Core::IDevice *CreateInstance(void)
			{
				throw new Exception("Method not supported!");
			}

			// Arguments
			// -- Id {String}
			// -- Width {Integer}
			// -- Height {Integer}
			// -- FrameRate {Integer}
			// -- Port {Integer}
			// -- Address {String}
			// -- Format {String}
			Illumina::Core::IDevice *CreateInstance(ArgumentMap &p_argumentMap)
			{
				IVideoStream::VideoCodec videoCodec;

				int width = 640,
					height = 480,
					frameRate = 25,
					port = 6666;

				std::string format = "MPEG2",
					address = "127.0.0.1";

				std::string strId;

				p_argumentMap.GetArgument("Port", port);
				p_argumentMap.GetArgument("Width", width);
				p_argumentMap.GetArgument("Height", height);
				p_argumentMap.GetArgument("Address", address);
				p_argumentMap.GetArgument("Filetype", format);
				p_argumentMap.GetArgument("FrameRate", frameRate);

				if (format == "MPEG2")
					videoCodec = IVideoStream::MPEG2;
				else if (format == "MPEG4")
					videoCodec = IVideoStream::MPEG4;
				else if (format == "H264")
					videoCodec = IVideoStream::H264;
				else if (format == "VP8")
					videoCodec = IVideoStream::VP8;
				else
					videoCodec = IVideoStream::MPEG1;

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, width, height, address, port, frameRate, videoCodec);

				return CreateInstance(width, height, address, port, frameRate, videoCodec);
			}

			Illumina::Core::IDevice *CreateInstance(const std::string &p_strId, 
				int p_nWidth, int p_nHeight, const std::string &p_strAddress, int p_nPort,
				int p_nFrameRate, IVideoStream::VideoCodec p_videoCodec)
			{
				return new RTPDevice(p_strId, p_nWidth, p_nHeight, p_strAddress, p_nPort, p_nFrameRate, p_videoCodec);
			}

			Illumina::Core::IDevice *CreateInstance(int p_nWidth, int p_nHeight, const std::string &p_strAddress, 
				int p_nPort, int p_nFrameRate, IVideoStream::VideoCodec p_videoCodec)

			{
				return new RTPDevice(p_nWidth, p_nHeight, p_strAddress, p_nPort, p_nFrameRate, p_videoCodec);
			}
		};


		class VideoDeviceFactory : public Illumina::Core::Factory<Illumina::Core::IDevice>
		{
		public:
			Illumina::Core::IDevice *CreateInstance(void)
			{
				throw new Exception("Method not supported!");
			}

			// Arguments
			// -- Id {String}
			// -- Width {Integer}
			// -- Height {Integer}
			// -- FrameRate {Integer}
			// -- Format {String}
			// -- Filename {String}
			Illumina::Core::IDevice *CreateInstance(ArgumentMap &p_argumentMap)
			{
				IVideoStream::VideoCodec videoCodec;

				int width = 640,
					height = 480,
					frameRate = 25;

				std::string format = "MPEG1",
					filename = "result.mpg";

				std::string strId;

				p_argumentMap.GetArgument("Width", width);
				p_argumentMap.GetArgument("Height", height);
				p_argumentMap.GetArgument("Filetype", format);
				p_argumentMap.GetArgument("Filename", filename);
				p_argumentMap.GetArgument("FrameRate", frameRate);

				if (format == "MPEG2")
					videoCodec = IVideoStream::MPEG2;
				else if (format == "MPEG4")
					videoCodec = IVideoStream::MPEG4;
				else if (format == "H264")
					videoCodec = IVideoStream::H264;
				else if (format == "VP8")
					videoCodec = IVideoStream::VP8;
				else
					videoCodec = IVideoStream::MPEG1;

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, width, height, frameRate, videoCodec, filename);

				return CreateInstance(width, height, frameRate, videoCodec, filename);
			}

			Illumina::Core::IDevice *CreateInstance(const std::string &p_strId, 
				int p_nWidth, int p_nHeight, int p_nFrameRate, IVideoStream::VideoCodec p_videoCodec, 
				const std::string &p_strFilename)
			{
				return new VideoDevice(p_strId, p_nWidth, p_nHeight, p_strFilename, p_nFrameRate, p_videoCodec);
			}

			Illumina::Core::IDevice *CreateInstance(int p_nWidth, int p_nHeight, 
				int p_nFrameRate, IVideoStream::VideoCodec p_videoCodec, const std::string &p_strFilename)
			{
				return new VideoDevice(p_nWidth, p_nHeight, p_strFilename, p_nFrameRate, p_videoCodec);
			}
		};


		class ImageDeviceFactory : public Illumina::Core::Factory<Illumina::Core::IDevice>
		{
		public:
			Illumina::Core::IDevice *CreateInstance(void)
			{
				throw new Exception("Method not supported!");
			}

			// Arguments
			// -- Id {String}
			// -- Width {Integer}
			// -- Height {Integer}
			// -- Format {String}
			// -- Filename {String}
			Illumina::Core::IDevice *CreateInstance(ArgumentMap &p_argumentMap)
			{
				int width = 640,
					height = 480;

				std::string format = "PPM",
					filename = "result.ppm";

				std::string strId;

				p_argumentMap.GetArgument("Width", width);
				p_argumentMap.GetArgument("Height", height);
				p_argumentMap.GetArgument("Filetype", format);
				p_argumentMap.GetArgument("Filename", filename);

				// So far only PPM is supported
				ImagePPM *pImagePPM = new ImagePPM();

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, width, height, pImagePPM, filename);

				return CreateInstance(width, height, pImagePPM, filename);
			}

			Illumina::Core::IDevice *CreateInstance(const std::string &p_strId, 
				int p_nWidth, int p_nHeight, IImageIO *p_pImageIO, const std::string &p_strFilename)
			{
				return new ImageDevice(p_strId, p_nWidth, p_nHeight, p_pImageIO, p_strFilename, true);
			}

			Illumina::Core::IDevice *CreateInstance(int p_nWidth, int p_nHeight, 
				IImageIO *p_pImageIO, const std::string &p_strFilename)
			{
				return new ImageDevice(p_nWidth, p_nHeight, p_pImageIO, p_strFilename, true);
			}
		};
	}
}