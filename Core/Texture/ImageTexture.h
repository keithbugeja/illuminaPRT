//----------------------------------------------------------------------------------------------
//	Filename:	Texture.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>

#include "Texture/Texture.h"
#include "Texture/Noise.h"
#include "Image/ImageIO.h"

namespace Illumina 
{
	namespace Core
	{
		class ImageTexture :
			public ITexture
		{
		protected:
			// Filtering and image buffer
			TextureFiltering m_filtering;
			boost::shared_ptr<Image> m_image;

			// Image dimensions (zero inclusive)
			int m_widthUnits, 
				m_heightUnits;

		protected:
			// NN
			void GetValueNearestNeighbour(const Vector2 &p_uv, RGBPixel &p_pixel) const
			{
				float u = Maths::Frac(p_uv.U);
				float v = 1.f - Maths::Frac(p_uv.V);

				m_image->Get((int)(m_widthUnits * u), (int)(m_heightUnits * v), p_pixel);
			}
			
			RGBPixel GetValueNearestNeighbour(const Vector2 &p_uv) const
			{
				float u = Maths::Frac(p_uv.U);
				float v = 1.f - Maths::Frac(p_uv.V);

				return m_image->Get((int)(m_widthUnits * u), (int)(m_heightUnits * v));
			}

			// Bilinear (NN for now... to change)
			void GetValueBilinear(const Vector2 &p_uv, RGBPixel &p_pixel) const
			{
				float u = Maths::Frac(p_uv.U);
				float v = 1.f - Maths::Frac(p_uv.V);

				m_image->Get((int)(m_widthUnits * u), (int)(m_heightUnits * v), p_pixel);
			}

			RGBPixel GetValueBilinear(const Vector2 &p_uv) const
			{
				// take fractional part of u and v
				float u = Maths::Frac(p_uv.U);
				float v = 1.0f - Maths::Frac(p_uv.V);

				// transform to texture space
				float iu = (m_image->GetWidth() - 1) * u;
				float iv = (m_image->GetHeight() - 1) * v;

				// get discretised coordinates
				int ix = (int)Maths::FAbs(Maths::Floor(iu));
				int iy = (int)Maths::FAbs(Maths::Floor(iv));

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
			}

		public:
			ImageTexture(const std::string &p_strName, const std::string &p_strFilename, IImageIO *p_pImageIO, TextureFiltering p_filtering = Bilinear) 
				: ITexture(p_strName) 
				, m_filtering(p_filtering)
			{
				m_image = boost::shared_ptr<Image>(p_pImageIO->Load(p_strFilename));

				m_widthUnits = m_image->GetWidth() - 1;
				m_heightUnits = m_image->GetHeight() - 1;
			}

			ImageTexture(const std::string &p_strFilename, IImageIO *p_pImageIO, TextureFiltering p_filtering = Bilinear) 
				: m_filtering(p_filtering)
			{
				m_image = boost::shared_ptr<Image>(p_pImageIO->Load(p_strFilename));

				m_widthUnits = m_image->GetWidth() - 1;
				m_heightUnits = m_image->GetHeight() - 1;
			}

			void GetValue(const Vector2 &p_uv, const Vector3 &p_hitpoint, RGBPixel &p_pixel) const
			{
				switch (m_filtering)
				{
					case Bilinear:
						GetValueBilinear(p_uv, p_pixel);
						break;

					default:
						GetValueNearestNeighbour(p_uv, p_pixel);
						break;
				}
			}
		};
	} 
}