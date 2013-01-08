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
#include "Device/BufferedImageDevice.h"
#include "Device/DisplayDevice.h"
#include "Device/VideoDevice.h"
#include "Device/RTPDevice.h"
#include "Image/ImagePPM.h"

namespace Illumina
{
	namespace Core
	{
		class DisplayDeviceFactory : public Illumina::Core::Factory<Illumina::Core::IDevice>
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
			Illumina::Core::IDevice *CreateInstance(ArgumentMap &p_argumentMap)
			{
				int width = 640,
					height = 480;

				std::string strId;

				// Read arguments
				p_argumentMap.GetArgument("Width", width);
				p_argumentMap.GetArgument("Height", height);

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, width, height);

				return CreateInstance(width, height);
			}

			Illumina::Core::IDevice *CreateInstance(const std::string &p_strId, int p_nWidth, int p_nHeight)
			{
				return new DisplayDevice(p_strId, p_nWidth, p_nHeight);
			}

			Illumina::Core::IDevice *CreateInstance(int p_nWidth, int p_nHeight)
			{
				return new DisplayDevice(p_nWidth, p_nHeight);
			}
		};

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
			// -- Format {String}
			// -- BitRate {Integer}
			// -- FrameRate {Integer}
			// -- Port {Integer}
			// -- Address {String}
			Illumina::Core::IDevice *CreateInstance(ArgumentMap &p_argumentMap)
			{
				IVideoStream::VideoCodec videoCodec;

				int width = 640,
					height = 480,
					bitRate = 128,
					frameRate = 25,
					port = 10000;

				std::string format = "mp2v",
					address = "127.0.0.1";

				std::string strId;

				// Read arguments
				p_argumentMap.GetArgument("Port", port);
				p_argumentMap.GetArgument("Width", width);
				p_argumentMap.GetArgument("Height", height);
				p_argumentMap.GetArgument("Format", format);
				p_argumentMap.GetArgument("Address", address);
				p_argumentMap.GetArgument("BitRate", bitRate);
				p_argumentMap.GetArgument("FrameRate", frameRate);

				// Get video codec from FourCC
				videoCodec = IVideoStream::GetCodec(format);

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, width, height, address, port, frameRate, bitRate, videoCodec);

				return CreateInstance(width, height, address, port, frameRate, bitRate, videoCodec);
			}

			Illumina::Core::IDevice *CreateInstance(const std::string &p_strId, 
				int p_nWidth, int p_nHeight, const std::string &p_strAddress, int p_nPort,
				int p_nFrameRate, int p_nBitRate, IVideoStream::VideoCodec p_videoCodec)
			{
				return new RTPDevice(p_strId, p_nWidth, p_nHeight, p_strAddress, p_nPort, p_nFrameRate, p_nBitRate, p_videoCodec);
			}

			Illumina::Core::IDevice *CreateInstance(int p_nWidth, int p_nHeight, const std::string &p_strAddress, 
				int p_nPort, int p_nFrameRate, int p_nBitRate, IVideoStream::VideoCodec p_videoCodec)
			{
				return new RTPDevice(p_nWidth, p_nHeight, p_strAddress, p_nPort, p_nFrameRate, p_nBitRate, p_videoCodec);
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
			// -- BitRate {Integer}
			// -- FrameRate {Integer}
			// -- Format {String}
			// -- Filename {String}
			Illumina::Core::IDevice *CreateInstance(ArgumentMap &p_argumentMap)
			{
				IVideoStream::VideoCodec videoCodec;

				int width = 640,
					height = 480,
					frameRate = 25,
					bitRate = 128;

				std::string format = "mp1v",
					filename = "result.mpg";

				std::string strId;

				// Read argumenrt map
				p_argumentMap.GetArgument("Width", width);
				p_argumentMap.GetArgument("Height", height);
				p_argumentMap.GetArgument("Format", format);
				p_argumentMap.GetArgument("BitRate", bitRate);
				p_argumentMap.GetArgument("Filename", filename);
				p_argumentMap.GetArgument("FrameRate", frameRate);

				// Get codec from FourCC
				videoCodec = IVideoStream::GetCodec(format);

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


		class BufferedImageDeviceFactory : public Illumina::Core::Factory<Illumina::Core::IDevice>
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
			// -- BufferSize {Integer}
			Illumina::Core::IDevice *CreateInstance(ArgumentMap &p_argumentMap)
			{
				int width = 640,
					height = 480,
					buffersize = 32;

				std::string format = "PPM",
					filename = "result.ppm";

				std::string strId;

				p_argumentMap.GetArgument("Width", width);
				p_argumentMap.GetArgument("Height", height);
				p_argumentMap.GetArgument("Format", format);
				p_argumentMap.GetArgument("Filename", filename);
				p_argumentMap.GetArgument("BufferSize", buffersize);

				// So far only PPM is supported
				// TODO: Destroy object when ready
				ImagePPM *pImagePPM = new ImagePPM();

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, width, height, pImagePPM, filename, buffersize);

				return CreateInstance(width, height, pImagePPM, filename, buffersize);
			}

			Illumina::Core::IDevice *CreateInstance(const std::string &p_strId, 
				int p_nWidth, int p_nHeight, IImageIO *p_pImageIO, const std::string &p_strFilename, int p_nBufferSize)
			{
				return new BufferedImageDevice(p_strId, p_nWidth, p_nHeight, p_pImageIO, p_strFilename, p_nBufferSize, true);
			}

			Illumina::Core::IDevice *CreateInstance(int p_nWidth, int p_nHeight, 
				IImageIO *p_pImageIO, const std::string &p_strFilename, int p_nBufferSize)
			{
				return new BufferedImageDevice(p_nWidth, p_nHeight, p_pImageIO, p_strFilename, p_nBufferSize, true);
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
			// -- Timestamp {Integer}
			Illumina::Core::IDevice *CreateInstance(ArgumentMap &p_argumentMap)
			{
				int width = 640,
					height = 480,
					timestamp = 0;

				std::string format = "PPM",
					filename = "result.ppm";

				std::string strId;

				p_argumentMap.GetArgument("Width", width);
				p_argumentMap.GetArgument("Height", height);
				p_argumentMap.GetArgument("Format", format);
				p_argumentMap.GetArgument("Filename", filename);
				p_argumentMap.GetArgument("Timestamp", timestamp);

				// So far only PPM is supported
				// TODO: Destroy object when ready
				ImagePPM *pImagePPM = new ImagePPM();

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, width, height, timestamp, pImagePPM, filename);

				return CreateInstance(width, height, timestamp, pImagePPM, filename);
			}

			Illumina::Core::IDevice *CreateInstance(const std::string &p_strId, 
				int p_nWidth, int p_nHeight, int p_nTimeStamp, IImageIO *p_pImageIO, const std::string &p_strFilename)
			{
				return new ImageDevice(p_strId, p_nWidth, p_nHeight, p_nTimeStamp != 0, p_pImageIO, p_strFilename, true);
			}

			Illumina::Core::IDevice *CreateInstance(int p_nWidth, int p_nHeight, 
				int p_nTimeStamp, IImageIO *p_pImageIO, const std::string &p_strFilename)
			{
				return new ImageDevice(p_nWidth, p_nHeight, p_nTimeStamp != 0, p_pImageIO, p_strFilename, true);
			}
		};
	}
}