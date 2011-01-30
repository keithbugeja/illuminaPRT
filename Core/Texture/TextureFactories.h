//----------------------------------------------------------------------------------------------
//	Filename:	TextureManager.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <map>
#include <string>
#include <iostream>

#include "Image/ImagePPM.h"

#include "Texture/Texture.h"
#include "Texture/SimpleTexture.h"
#include "Texture/NoiseTexture.h"
#include "Texture/MarbleTexture.h"
#include "Texture/ImageTexture.h"

namespace Illumina
{
	namespace Core
	{
		class ImageTextureFactory : public Illumina::Core::Factory<Illumina::Core::ITexture>
		{
		public:
			Illumina::Core::ITexture *CreateInstance(void)
			{
				throw new Exception("ImageTextureFactory cannot create ImageTexture instance without a specified filename!");
			}

			Illumina::Core::ITexture *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strName, 
					strFilename,
					strFiletype;

				if (p_argumentMap.GetArgument("Name", strName) && 
					p_argumentMap.GetArgument("Filename", strFilename) && 
					p_argumentMap.GetArgument("Filetype", strFiletype))
				{
					return CreateInstance(strName, strFilename, strFiletype);
				}

				throw new Exception("Invalid arguments to ImageTextureFactory!");
			}

			Illumina::Core::ITexture *CreateInstance(const std::string &p_strName, const std::string &p_strFilename, const std::string &p_strFiletype)
			{
				if (p_strFiletype.find("PPM") != std::string::npos)
				{
					ImagePPM imagePPM;
					return new ImageTexture(p_strName, p_strFilename, (IImageIO*)&imagePPM);
				}

				throw new Exception("Unable to create ImageTexture instance!");
			}
		};

		class MarbleTextureFactory : public Illumina::Core::Factory<Illumina::Core::ITexture>
		{
		public:
			Illumina::Core::ITexture *CreateInstance(void)
			{
				return new MarbleTexture(4.0f);
			}

			Illumina::Core::ITexture *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strName;

				int nOctaves;

				float fStripes,
					fScale;

				RGBPixel rgb[3]; 

				if (p_argumentMap.GetArgument("Name", strName) && 
					p_argumentMap.GetArgument("Stripes", fStripes) && 
					p_argumentMap.GetArgument("Scale", fScale) &&
					p_argumentMap.GetArgument("Octaves", nOctaves))
				{
					if (p_argumentMap.GetArgument("RGBLow", rgb[0]) &&
						p_argumentMap.GetArgument("RGBMedium", rgb[1]) &&
						p_argumentMap.GetArgument("RGBHigh", rgb[2]))
					{
						return CreateInstance(strName, rgb[0], rgb[1], rgb[2], fStripes, fScale, nOctaves);
					}

					return CreateInstance(strName, fStripes, fScale, nOctaves);
				}

				throw new Exception("Invalid arguments to MarbleTextureFactory!");
			}

			Illumina::Core::ITexture *CreateInstance(const std::string &p_strName, 
				const RGBPixel &p_rgbLow, const RGBPixel &p_rgbMedium, const RGBPixel &p_rgbHigh,
				float p_fStripesPerUnit, float p_fScale, int p_nOctaves)
			{
				return new MarbleTexture(p_strName, p_rgbLow, p_rgbMedium, p_rgbHigh, p_fStripesPerUnit, p_fScale, p_nOctaves);
			}

			Illumina::Core::ITexture *CreateInstance(const std::string &p_strName, 
				float p_fStripesPerUnit, float p_fScale, int p_nOctaves)
			{
				return new MarbleTexture(p_strName, p_fStripesPerUnit, p_fScale, p_nOctaves);
			}
		};

		class NoiseTextureFactory : public Illumina::Core::Factory<Illumina::Core::ITexture>
		{
		public:
			Illumina::Core::ITexture *CreateInstance(void)
			{
				return new NoiseTexture();
			}

			Illumina::Core::ITexture *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strName;
				RGBPixel rgb[2]; 
				float fScale;

				if (p_argumentMap.GetArgument("Name", strName) && 
					p_argumentMap.GetArgument("Scale", fScale))
				{
					if (p_argumentMap.GetArgument("RGBLow", rgb[0]) &&
						p_argumentMap.GetArgument("RGBHigh", rgb[1]))
					{
						return CreateInstance(strName, rgb[0], rgb[1], fScale);
					}

					return CreateInstance(strName, fScale);
				}

				throw new Exception("Invalid arguments to NoiseTextureFactory!");
			}

			Illumina::Core::ITexture *CreateInstance(const std::string &p_strName, 
				const RGBPixel &p_rgbLow, const RGBPixel &p_rgbHigh, float p_fScale)
			{
				return new NoiseTexture(p_strName, p_rgbLow, p_rgbHigh, p_fScale);
			}

			Illumina::Core::ITexture *CreateInstance(const std::string &p_strName, float p_fScale)
			{
				return new NoiseTexture(p_strName, p_fScale);
			}
		};
	}
}