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
			boost::iostreams::mapped_file  m_imageFile;
			unsigned char *m_imageDataLDR;
			float *m_imageData;
			int m_width, m_height;

		protected:
			RGBPixel GetPixel(int x, int y) const
			{
				float *pixel = m_imageData + ((y * m_width + x) * 3);
				return RGBPixel(pixel[0], pixel[1], pixel[2]);
			}

			RGBPixel GetPixelLDR(int x, int y) const
			{
				float r = 1.f / 255.f;

				unsigned char *pixel = m_imageDataLDR + ((y * m_width + x) * 3);
				return RGBPixel((float)pixel[0] * r, (float)pixel[1] * r, (float)pixel[2] * r);
			}

			RGBPixel GetValueNearestNeighbour(const Vector2 &p_uv) const
			{
				// take fractional part of u and v
				float u = Maths::Frac(p_uv.U);
				float v = 1.0f - Maths::Frac(p_uv.V);

				float width = m_width - 1,
					height = m_height - 1;

				// transform to texture space
				float iu = width * u;
				float iv = height * v;

				// get discretised coordinates
				int ix = Maths::Min(width, iu > 0.5f ? Maths::Ceil(iu) : Maths::Floor(iu));
				int iy = Maths::Min(height, iv > 0.5f ? Maths::Ceil(iv) : Maths::Floor(iv));

				return GetPixelLDR(ix, iy);
			}

			RGBPixel GetValueBilinear(const Vector2 &p_uv) const
			{
				/*
				// take fractional part of u and v
				float u = Maths::Frac(p_uv.U);
				float v = 1.0f - Maths::Frac(p_uv.V);

				// transform to texture space
				float iu = (m_image->GetWidth() - 1) * u;
				float iv = (m_image->GetHeight() - 1) * v;

				// get discretised coordinates
				float ix = Maths::FAbs(Maths::Floor(iu));
				float iy = Maths::FAbs(Maths::Floor(iv));

				RGBPixel value[4];
				value[0] = m_image->Get(ix, iy);
				value[1] = m_image->Get(Maths::Max(0, Maths::Min(ix, m_image->GetWidth() - 1)), iy); 
				value[2] = m_image->Get(ix, Maths::Max(0, Maths::Min(iy, m_image->GetHeight() - 1)));
				value[3] = m_image->Get(Maths::Max(0, Maths::Min(ix, m_image->GetWidth() - 1)), Maths::Max(0, Maths::Min(iy, m_image->GetHeight() - 1))); 

				float fx = Maths::Frac(iu);
				float fy = Maths::Frac(iv);

				float w0 = (1.0f - fx) * (1.0f - fy),
					  w1 = fx * (1.0f - fy),
					  w2 = (1.0f - fx) * fy,
					  w3 = fx * fy;

				return value[0] * w0 + value[1] * w1 + value[2] * w2 + value[3] * w3;
				*/
			}

			void MapFile(const std::string &p_strFilename)
			{
				m_imageFile.open(p_strFilename);

				int *imageData = (int*)m_imageFile.const_data();
				
				m_width		= imageData[0];
				m_height	= imageData[1];

				m_imageData = (float*)(imageData + 2);
				m_imageDataLDR = (unsigned char*)(imageData + 2);
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

			RGBPixel GetValue(const Vector2 &p_uv, const Vector3 &p_hitPoint) const
			{
				switch (m_filtering)
				{
					case Bilinear:
						return GetValueNearestNeighbour(p_uv);
						// return GetValueBilinear(p_uv);
					
					default:
						return GetValueNearestNeighbour(p_uv);
				}
			}
		};
	} 
}