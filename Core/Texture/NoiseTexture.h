//----------------------------------------------------------------------------------------------
//	Filename:	Texture.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Texture/Texture.h"
#include "Texture/Noise.h"

namespace Illumina
{
	namespace Core
	{
		class NoiseTexture 
			: public ITexture
		{
		protected:
			float m_fScale;
			Noise m_noise;
			RGBPixel m_c0, m_c1;

		public:
			NoiseTexture(const std::string &p_strName, float p_fScale = 1.0f)
				: ITexture(p_strName)
			{
				m_fScale = p_fScale;
				m_c0 = RGBPixel(0.8f, 0.0f, 0.0f);
				m_c1 = RGBPixel(0.0f, 0.0f, 0.8f);
			}

			NoiseTexture(const std::string &p_strName, const RGBPixel &p_c0, const RGBPixel &p_c1, float p_fScale = 1.0f)
				: ITexture(p_strName)
				, m_fScale(p_fScale)
				, m_c0(p_c0)
				, m_c1(p_c1)
			{ }

			NoiseTexture(float p_fScale = 1.0f)
			{
				m_fScale = p_fScale;
				m_c0 = RGBPixel(0.8f, 0.0f, 0.0f);
				m_c1 = RGBPixel(0.0f, 0.0f, 0.8f);
			}

			NoiseTexture(const RGBPixel &p_c0, const RGBPixel &p_c1, float p_fScale = 1.0f)
				: m_fScale(p_fScale)
				, m_c0(p_c0)
				, m_c1(p_c1)
			{ }

			void GetValue(const Vector2& p_uv, const Vector3 &p_hitPoint, RGBPixel &p_pixel) const
			{
				float t = (1.0f + m_noise.Perlin(p_hitPoint * m_fScale)) / 2.0f;
				p_pixel = t * m_c0 + (1.0f - t) * m_c1;
			}
		};
	}
}
