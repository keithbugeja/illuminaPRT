//----------------------------------------------------------------------------------------------
//	Filename:	RGB.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Representation of the RGB colour space.
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>

#include "Maths/Maths.h"

namespace Illumina 
{
	namespace Core
	{
		class RGBPixel
		{
		public:
			static const RGBPixel White;
			static const RGBPixel Black;
			static const RGBPixel Red;
			static const RGBPixel Green;
			static const RGBPixel Blue;

			union
			{
				float Element[3];
				struct { float R, G, B; };
			};

		public:
			RGBPixel() { }

			RGBPixel(float p_fValue)
				: R(p_fValue), G(p_fValue), B(p_fValue) { }

			RGBPixel(float p_fRed, float p_fGreen, float p_fBlue)
				: R(p_fRed), G(p_fGreen), B(p_fBlue) { }

			RGBPixel(const RGBPixel &p_rgb)
				: R(p_rgb.R), G(p_rgb.G), B(p_rgb.B) { }

			float operator[](int p_nIndex) const { return Element[p_nIndex]; }
			float& operator[](int p_nIndex) { return Element[p_nIndex]; }

			inline RGBPixel& operator=(const RGBPixel &p_rgb)
			{
				R = p_rgb.R; G = p_rgb.G; B = p_rgb.B;
				return *this;
			}

			inline RGBPixel& operator+=(const RGBPixel &p_rgb) {
				return *this = *this + p_rgb;
			}

			inline RGBPixel& operator-=(const RGBPixel &p_rgb) {
				return *this = *this - p_rgb;
			}
			
			inline RGBPixel& operator*=(const RGBPixel &p_rgb) {
				return *this = *this * p_rgb;
			}

			inline RGBPixel& operator/=(const RGBPixel &p_rgb) {
				return *this = *this / p_rgb;
			}
			
			inline RGBPixel& operator*=(float p_fScale) {
				return *this = *this * p_fScale;
			}

			inline RGBPixel& operator/=(float p_fScale) {
				return *this = *this / p_fScale;
			}

			inline RGBPixel operator*(float p_fScale) const {
				return RGBPixel(R * p_fScale, G * p_fScale, B * p_fScale);
			}
			
			inline RGBPixel operator/(float p_fScale) const 
			{
				BOOST_ASSERT(p_fScale > 0);
				return *this * (1.0f / p_fScale);
			}

			inline RGBPixel operator/(const RGBPixel &p_rgb) const
			{
				BOOST_ASSERT(p_rgb.R > 0 && p_rgb.G > 0 && p_rgb.B > 0);
				return RGBPixel(R / p_rgb.R, G / p_rgb.G, B / p_rgb.B);
			}

			inline RGBPixel operator*(const RGBPixel &p_rgb) const {
				return RGBPixel(R * p_rgb.R, G * p_rgb.G, B * p_rgb.B);
			}

			inline RGBPixel operator+(const RGBPixel &p_rgb) const {
				return RGBPixel(R + p_rgb.R, G + p_rgb.G, B + p_rgb.B);
			}

			inline RGBPixel operator-(const RGBPixel &p_rgb) const {
				return RGBPixel(R - p_rgb.R, G - p_rgb.G, B - p_rgb.B);
			}

			void Set(float p_fRed, float p_fGreen, float p_fBlue) {
				R = p_fRed; G = p_fGreen; B = p_fBlue;
			}

			void Normalize()
			{
				float length = R * R + G * G + B * B;
				
				if (length > 0.0f)
				{
					length = Maths::Sqrt(length);
					R /= length; G /= length; B /= length;
				}
			}

			void Clamp(float p_fMin = 0.0f, float p_fMax = 1.0f)
			{
				if (R < p_fMin) R = p_fMin;
				else if (R > p_fMax) R = p_fMax;

				if (G < p_fMin) G = p_fMin;
				else if (G > p_fMax) G = p_fMax;

				if (B < p_fMin) B = p_fMin;
				else if (B > p_fMax) B = p_fMax;
			}

			std::string ToString(void)
			{
				boost::format formatter;
				std::string strOut = boost::str(boost::format("[%d %d %d]") % R % G % B);
				return strOut;
			}
		};

		inline RGBPixel operator*(float p_fScale, const RGBPixel &p_colour) {
			return RGBPixel(p_colour.R * p_fScale, p_colour.G * p_fScale, p_colour.B * p_fScale);
		}
	} 
}