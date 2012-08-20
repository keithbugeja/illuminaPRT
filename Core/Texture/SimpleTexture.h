//----------------------------------------------------------------------------------------------
//	Filename:	Texture.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Texture/Texture.h"

namespace Illumina 
{
	namespace Core
	{
		class SimpleTexture
		{
		protected:
			RGBPixel m_colour;
		
		public:
			SimpleTexture(RGBPixel p_colour)
				: m_colour(p_colour)
			{ }

			void GetValue(const Vector2& p_uv, const Vector3 &p_hitPoint, RGBPixel &p_pixel) const {
				p_pixel = m_colour;
			}
		};
	} 
}