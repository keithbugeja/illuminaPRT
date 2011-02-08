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
#include "Image/ImagePPM.h"

namespace Illumina
{
	namespace Core
	{		
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