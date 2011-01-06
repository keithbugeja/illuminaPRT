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
		class ImagePPM 
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
				int	width, height, colours;
				
				RGBPixel colour;
				
				imageFile.get(magicNumber[0]);
				imageFile.get(magicNumber[1]);
				imageFile.get(whitespace);
				imageFile >> std::noskipws >> width;
				imageFile.get(whitespace);
				imageFile >> std::noskipws >> height;
				imageFile.get(whitespace);
				imageFile >> std::noskipws >> colours;
				imageFile.get(whitespace);

				std::cout << "Image header : [" << width << " x " << height << " x " << colours << "]" << std::endl;

				// Create image
				Image* pImage = new Image(width, height);
				Image &image = *pImage;

				for (int i = 0; i < image.GetLength(); i++)
				{
					image[i].R = (unsigned char)imageFile.get();
					image[i].G = (unsigned char)imageFile.get();
					image[i].B = (unsigned char)imageFile.get();

					image[i]/=255.0f;
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
				header << "P6" << ' ';
				header << p_image.GetWidth() << ' ';
				header << p_image.GetHeight() << ' ';
				header << 255 << ' ';

				// Open image file writer stream
				imageFile.open(p_strImageFile.c_str(), std::ios::binary);

				// Dump header
				imageFile << header.str();

				for (int i = 0; i < p_image.GetLength(); i++)
				{
					imageFile.put((unsigned char)Maths::Min(255, 256 * p_image[i].R));
					imageFile.put((unsigned char)Maths::Min(255, 256 * p_image[i].G));
					imageFile.put((unsigned char)Maths::Min(255, 256 * p_image[i].B));
				}
				
				imageFile.close();
			}
		};
	} 
}