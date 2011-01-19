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
			ImageTexture(const std::string &p_strName, const std::string &p_strFilename, IImageIO *p_pImageIO) 
				: ITexture(p_strName) 
			{
				m_image = boost::shared_ptr<Image>(p_pImageIO->Load(p_strFilename));
			}

			ImageTexture(const std::string &p_strFilename, IImageIO *p_pImageIO) {
				m_image = boost::shared_ptr<Image>(p_pImageIO->Load(p_strFilename));
			}

			RGBPixel GetValue(const Vector2 &p_uv, const Vector3 &p_hitPoint) const
			{
				float u, v;

				u = p_uv.U >= 0	? p_uv.U - (int)p_uv.U 
								: 1.0f + (p_uv.U - (int)p_uv.U);

				v = p_uv.V >= 0	? p_uv.V - (int)p_uv.V
								: 1.0f + (p_uv.V - (int)p_uv.V);

				//u = Maths::Frac(u);
				//v = Maths::Frac(v);

				//u = Maths::Max(0.0f, Maths::Min(1.0f, u));
				//v = Maths::Max(0.0f, Maths::Min(1.0f, v));

				u *= (m_image->GetWidth() - 1);
				v *= (m_image->GetHeight() - 1);

				int iu = Maths::FAbs(((int)u) % m_image->GetWidth()),
					iv = Maths::FAbs(((int)v) % m_image->GetHeight());

				// Add interpolation w. smoothing function
				return m_image->Get(iu, iv);
			}
		};
	} 
}