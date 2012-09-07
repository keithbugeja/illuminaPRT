//----------------------------------------------------------------------------------------------
//	Filename:	MemoryMappedTexture.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

#include "Texture/Texture.h"
#include "Texture/Noise.h"
#include "Image/ImageIO.h"

namespace Illumina 
{
	namespace Core
	{
		class MemoryMappedTexture :
			public ITexture
		{
		protected:
			TextureFiltering m_filtering;

			boost::iostreams::mapped_file_source m_imageFile;
			unsigned char *m_imageDataLDR;
			float *m_imageData;
			int m_width, m_height,
				m_widthUnits, m_heightUnits;

		protected:
			inline void GetPixel(int x, int y, RGBPixel &p_pixel) const
			{
				float *pixel = m_imageData + ((y * m_width + x) * 3);
				p_pixel.Set(pixel[0], pixel[1], pixel[2]);
			}

			inline void GetPixelLDR(int x, int y, RGBPixel &p_pixel) const
			{
				static float r = 1.f / 255.f;

				unsigned char *pixel = m_imageDataLDR + ((y * m_width + x) * 3);
				p_pixel.Set((float)pixel[0] * r, (float)pixel[1] * r, (float)pixel[2] * r);
			}

			void GetValueNearestNeighbour(const Vector2 &p_uv, RGBPixel &p_pixel) const
			{
				// take fractional part of u and v
				float u = Maths::Frac(p_uv.U);
				float v = 1.0f - Maths::Frac(p_uv.V);

				GetPixelLDR((int)(m_widthUnits * u), (int)(m_heightUnits * v), p_pixel);
			}

			void MapFile(const std::string &p_strFilename)
			{
				m_imageFile.open(p_strFilename);

				int *imageData = (int*)m_imageFile.data();
				
				m_width		= imageData[0];
				m_height	= imageData[1];

				m_widthUnits = m_width - 1;
				m_heightUnits = m_height - 1;

				m_imageData = (float*)(imageData + 2);
				m_imageDataLDR = (unsigned char*)(imageData + 2);
			}

			void UnmapFile(void)
			{
				m_imageFile.close();
			}

		public:
			static void Make(const Image& p_image, const std::string &p_strOutputFilename)
			{
				std::ofstream imageFile;

				// Open image file writer stream
				imageFile.open(p_strOutputFilename.c_str(), std::ios::binary);

				int width = p_image.GetWidth(),
					height = p_image.GetHeight();

				imageFile.write((const char*)&width, sizeof(int));
				imageFile.write((const char*)&height, sizeof(int));

				for (int i = 0; i < p_image.GetArea(); i++)	{
					imageFile.write((const char*)(p_image[i].Element), sizeof(float) * 3);
				}
				
				imageFile.close();
			}

			static void MakeLDR(const Image& p_image, const std::string &p_strOutputFilename)
			{
				std::ofstream imageFile;

				// Open image file writer stream
				imageFile.open(p_strOutputFilename.c_str(), std::ios::binary);

				int width = p_image.GetWidth(),
					height = p_image.GetHeight();

				imageFile.write((const char*)&width, sizeof(int));
				imageFile.write((const char*)&height, sizeof(int));

				unsigned char rgb[3];

				for (int i = 0; i < p_image.GetArea(); i++)	
				{
					rgb[0] = (unsigned char)(p_image[i].R * 255);
					rgb[1] = (unsigned char)(p_image[i].G * 255);
					rgb[2] = (unsigned char)(p_image[i].B * 255);

					imageFile.write((const char*)(rgb), sizeof(unsigned char) * 3);
				}
				
				imageFile.close();
			}

			MemoryMappedTexture(const std::string &p_strName, const std::string &p_strFilename, TextureFiltering p_filtering = Bilinear) 
				: ITexture(p_strName) 
				, m_filtering(p_filtering)
			{
				MapFile(p_strFilename);
			}

			MemoryMappedTexture(const std::string &p_strFilename, TextureFiltering p_filtering = Bilinear)
				: m_filtering(p_filtering)
			{
				MapFile(p_strFilename);
			}

			~MemoryMappedTexture(void)
			{
				UnmapFile();
			}

			void GetValue(const Vector2 &p_uv, const Vector3 &p_hitPoint, RGBPixel &p_pixel) const
			{
				switch (m_filtering)
				{
					case Bilinear:
						GetValueNearestNeighbour(p_uv, p_pixel);
						break;

					default:
						GetValueNearestNeighbour(p_uv, p_pixel);
						break;
				}
			}
		};
	} 
}