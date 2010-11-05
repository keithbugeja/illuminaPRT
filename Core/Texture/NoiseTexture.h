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
		{
		protected:
			float m_fScale;
			RGBPixel	m_c0, m_c1;
			Noise m_noise;

		public:
			NoiseTexture(float p_fScale = 1.0f)
			{
				m_fScale = p_fScale;
				m_c0 = RGBPixel(0.8f, 0.0f, 0.0f);
				m_c1 = RGBPixel(0.0f, 0.0f, 0.8f);
			}

			NoiseTexture(const RGBPixel &p_c0, const RGBPixel &p_c1, float p_fScale = 1.0f)
				: m_c0(p_c0)
				, m_c1(p_c1)
				, m_fScale(p_fScale)
			{ }

			RGBPixel GetValue(const Vector2& p_uv, const Vector3 &p_hitPoint) const
			{
				float t = (1.0f + m_noise.Perlin(p_hitPoint * m_fScale)) / 2.0f;
				return t * m_c0 + (1.0f - t) * m_c1;
			}
		};
	} 
}