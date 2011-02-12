//----------------------------------------------------------------------------------------------
//	Filename:	ImagePPM.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Portable Pixel Map (PPM) implementation for loading and saving image files.
//----------------------------------------------------------------------------------------------
#pragma once

#include <iostream>
#include <fstream>
#include <sstream>

#include "Image/ImageIO.h"

namespace Illumina 
{
	namespace Core
	{
		class ImagePFM 
			: public IImageIO
		{
		public:
			Image* Load(const std::string &p_strImageFile)
			{
				std::ifstream imageFile;

				// Open image file
				imageFile.open(p_strImageFile.c_str(), std::ios::binary);

				if (!imageFile.is_open())
				{
					std::cerr << "ERROR -- Couldn't open file \'" << p_strImageFile << "\'" << std::endl;
					exit(-1);
				}

				// Read and parse header
				char magicNumber[2], whitespace;
				int	width, height;
				float scale;
				
				RGBPixel colour;
				
				imageFile.get(magicNumber[0]);
				imageFile.get(magicNumber[1]);
				imageFile.get(whitespace);
				imageFile >> std::noskipws >> width;
				imageFile.get(whitespace);
				imageFile >> std::noskipws >> height;
				imageFile.get(whitespace);
				imageFile >> std::noskipws >> scale;
				imageFile.get(whitespace);

				// Create image
				Image* pImage = new Image(width, height);
				Image &image = *pImage;

				// Colour
				if (magicNumber[1] == 'F')
				{
					// Little endian
					if (scale < 0)
					{
						for (int i = 0; i < image.GetLength(); i++)
						{
							imageFile.read( (char*)image[i].Element, sizeof(float) * 3);
						}
					} else {
						std::cerr << "ERROR -- Big endian not implemented!" << std::endl;
						exit(-1);
					}
				}
				// Greyscale
				else if (magicNumber[1] == 'f')
				{
						std::cerr << "ERROR -- Grayscale not implemented!" << std::endl;
						exit(-1);
				}

				// Close image file
				imageFile.close();

				return pImage;
			}

			void Save(const Image &p_image, const std::string &p_strImageFile)
			{
				std::stringstream header;
				std::ofstream imageFile;

				// Create header
				header << "PF" << ' ';
				header << p_image.GetWidth() << ' ';
				header << p_image.GetHeight() << ' ';
				header << -1.0 << ' ';

				// Open image file writer stream
				imageFile.open(p_strImageFile.c_str(), std::ios::binary);

				// Dump header
				imageFile << header.str();

				for (int i = 0; i < p_image.GetLength(); i++)
				{
					imageFile.write((char*)p_image[i].Element, sizeof(float) * 3);
				}
				
				imageFile.close();
			}
		};
	} 
}