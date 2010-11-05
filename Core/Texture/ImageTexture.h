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
			boost::shared_ptr<Image> m_image;

		public:
			ImageTexture(std::string p_strFilename, IImageIO &p_imageIO) {
				m_image = boost::shared_ptr<Image>(p_imageIO.Load(p_strFilename));
			}

			RGBPixel GetValue(const Vector2 &p_uv, const Vector3 &p_hitPoint) const
			{
				float u, v;

				u = p_uv.U >= 0	? p_uv.U - (int)p_uv.U 
								: 1.0f + (p_uv.U - (int)p_uv.U);

				v = p_uv.V >= 0	? p_uv.V - (int)p_uv.V
								: 1.0f + (p_uv.V - (int)p_uv.V);

				//u = Maths::Max(0.0f, Maths::Min(1.0f, u));
				//v = Maths::Max(0.0f, Maths::Min(1.0f, v));

				u *= (m_image->GetWidth() - 1);
				v *= (m_image->GetHeight() - 1);

				int iu = (int)u,
					iv = (int)v;

				// Add interpolation w. smoothing function
				return m_image->Get(iu, iv);
			}
		};
	} 
}