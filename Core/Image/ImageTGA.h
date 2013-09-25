//----------------------------------------------------------------------------------------------
//	Filename:	ImageTGA.h
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
		class ImageTGA 
			: public IImageIO
		{
		public:
			Image* Load(const std::string &p_strImageFile)
			{
				std::ifstream imageFile;

				// Open image file
				imageFile.open(p_strImageFile.c_str(), std::ios::in | std::ios::binary);

				if (!imageFile.is_open())
				{
					// Output to stderr
					std::cerr << "ERROR -- Couldn't open file \'" << 
						p_strImageFile << "\'" << std::endl;

					// Use testcard instead
					Image *pImage = new Image(64, 64);
					pImage->MakeTestCard();
					return pImage;
				}

				// width, height and depth of image
				unsigned short width, 
					height;

				unsigned char depth;

				// Seek block
				imageFile.seekg(12, std::ios::beg);

				// Read width, height and depth
				imageFile.read((char *)&width, sizeof(unsigned short));
				imageFile.read((char *)&height, sizeof(unsigned short));
				depth = imageFile.get();

				// std::cout << "Image : [" << p_strImageFile << "] is " << width << " x " << height << " x " << (int)depth << std::endl;

				// Seek image data
				imageFile.seekg(18, std::ios::beg);

				// Create image
				Image* pImage = new Image(width, height);
				Image &image = *pImage;

				int mode = depth >> 3;

				switch(mode)
				{
					case 3:
					{
						for (int i = 0; i < image.GetArea(); i++)
						{
							image[i].B = (unsigned char)imageFile.get();
							image[i].G = (unsigned char)imageFile.get();
							image[i].R = (unsigned char)imageFile.get();

							image[i]/=255.0f;
						}

						break;
					}

					case 4:
					{
						for (int i = 0; i < image.GetArea(); i++)
						{
							image[i].B = (unsigned char)imageFile.get();
							image[i].G = (unsigned char)imageFile.get();
							image[i].R = (unsigned char)imageFile.get();

							image[i]/=255.0f;
						}

						break;
					}
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

				for (int i = 0; i < p_image.GetArea(); i++)
				{
					imageFile.put((unsigned char)Maths::Min(255, (int)(256 * p_image[i].R)));
					imageFile.put((unsigned char)Maths::Min(255, (int)(256 * p_image[i].G)));
					imageFile.put((unsigned char)Maths::Min(255, (int)(256 * p_image[i].B)));
				}
				
				imageFile.close();
			}
		};
	} 
}