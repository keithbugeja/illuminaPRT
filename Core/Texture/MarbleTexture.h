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
		class MarbleTexture
			: public ITexture
		{
		protected:
			RGBPixel m_c0, m_c1, m_c2;

			Noise m_noise;

			float m_fFrequency, 
				m_fScale;

			int m_nOctaves;

		public:
			MarbleTexture(const std::string &p_strName, float p_fStripesPerUnit, float p_fScale = 5.0f, int p_nOctaves = 8)
				: ITexture(p_strName)
			{
				m_fFrequency = Maths::Pi * p_fStripesPerUnit;
				m_fScale = p_fScale;
				m_nOctaves = p_nOctaves;
				m_c0 = RGBPixel(0.8f, 0.8f, 0.8f);
				m_c1 = RGBPixel(0.4f, 0.2f, 0.1f);
				m_c2 = RGBPixel(0.06f, 0.04f, 0.02f);
			}

			MarbleTexture(const std::string &p_strName, const RGBPixel &p_c0, const RGBPixel &p_c1, const RGBPixel &p_c2, 
				float p_fStripesPerUnit, float p_fScale = 5.0f, int p_nOctaves = 8)
				: ITexture(p_strName)
			{
				m_fFrequency = Maths::Pi * p_fStripesPerUnit;
				m_fScale = p_fScale;
				m_nOctaves = p_nOctaves;
				m_c0 = p_c0;
				m_c1 = p_c1;
				m_c2 = p_c2;
			}

			MarbleTexture(float p_fStripesPerUnit, float p_fScale = 5.0f, int p_nOctaves = 8)
			{
				m_fFrequency = Maths::Pi * p_fStripesPerUnit;
				m_fScale = p_fScale;
				m_nOctaves = p_nOctaves;
				m_c0 = RGBPixel(0.8f, 0.8f, 0.8f);
				m_c1 = RGBPixel(0.4f, 0.2f, 0.1f);
				m_c2 = RGBPixel(0.06f, 0.04f, 0.02f);
			}

			MarbleTexture(const RGBPixel &p_c0, const RGBPixel &p_c1, const RGBPixel &p_c2, 
				float p_fStripesPerUnit, float p_fScale = 5.0f, int p_nOctaves = 8)
			{
				m_fFrequency = Maths::Pi * p_fStripesPerUnit;
				m_fScale = p_fScale;
				m_nOctaves = p_nOctaves;
				m_c0 = p_c0;
				m_c1 = p_c1;
				m_c2 = p_c2;
			}

			void GetValue(const Vector2 &p_uv, const Vector3 &p_hitPoint, RGBPixel &p_pixel) const
			{
				float temp = m_fScale * m_noise.Turbulence(m_fFrequency * p_hitPoint, m_nOctaves);
				float t = 2.0F * Maths::FAbs(Maths::Sin(m_fFrequency * p_hitPoint.X + temp));

				if (t < 1.0f)
					p_pixel = (m_c1 * t + (1.0f - t) * m_c2);
				else
				{
					t -= 1.0f;
					p_pixel = (m_c0 * t + (1.0f - t) * m_c1);
				}
			}
		};
	} 
}