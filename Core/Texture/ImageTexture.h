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
			TextureFiltering m_filtering;
			boost::shared_ptr<Image> m_image;

		protected:
			RGBPixel GetValueNearestNeighbour(const Vector2 &p_uv) const
			{
				// take fractional part of u and v
				float u = Maths::Frac(p_uv.U);
				float v = 1.0f - Maths::Frac(p_uv.V);

				float width = m_image->GetWidth() - 1,
					height = m_image->GetHeight() - 1;

				// transform to texture space
				float iu = width * u;
				float iv = height * v;

				// get discretised coordinates
				float ix = Maths::Min(width, iu > 0.5f ? Maths::Ceil(iu) : Maths::Floor(iu));
				float iy = Maths::Min(height, iv > 0.5f ? Maths::Ceil(iv) : Maths::Floor(iv));

				return m_image->Get(ix, iy);
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
				float ix = Maths::FAbs(Maths::Floor(iu));
				float iy = Maths::FAbs(Maths::Floor(iv));

				RGBPixel value[4];
				value[0] = m_image->Get(ix, iy);
				value[1] = m_image->Get(Maths::Min(ix, m_image->GetWidth() - 1), iy); 
				value[2] = m_image->Get(ix, Maths::Min(iy, m_image->GetHeight() - 1));
				value[3] = m_image->Get(Maths::Min(ix, m_image->GetWidth() - 1), Maths::Min(iy, m_image->GetHeight() - 1)); 

				float fx = Maths::Frac(ix);
				float fy = Maths::Frac(iy);

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
			}

			ImageTexture(const std::string &p_strFilename, IImageIO *p_pImageIO, TextureFiltering p_filtering = Bilinear) 
				: m_filtering(p_filtering)
			{
				m_image = boost::shared_ptr<Image>(p_pImageIO->Load(p_strFilename));
			}

			RGBPixel GetValue(const Vector2 &p_uv, const Vector3 &p_hitPoint) const
			{
				switch (m_filtering)
				{
					case Bilinear:
						return GetValueBilinear(p_uv);
					
					default:
						return GetValueNearestNeighbour(p_uv);
				}
			}
		};
	} 
}