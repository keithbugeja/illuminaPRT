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
		//----------------------------------------------------------------------------------------------
		// templated RGBPixel
		//----------------------------------------------------------------------------------------------
		template <typename TComponentType, int TMin, int TMax> 
		class TRGBPixel
		{
		public:
			static const TRGBPixel<TComponentType, TMin, TMax> White;
			static const TRGBPixel<TComponentType, TMin, TMax> Black;
			static const TRGBPixel<TComponentType, TMin, TMax> Red;
			static const TRGBPixel<TComponentType, TMin, TMax> Green;
			static const TRGBPixel<TComponentType, TMin, TMax> Blue;

			union
			{
				TComponentType Element[3];
				struct { TComponentType R, G, B; };
			};

		public:
			//----------------------------------------------------------------------------------------------
			// Constructors
			//----------------------------------------------------------------------------------------------
			TRGBPixel(void) { }

			TRGBPixel(TComponentType p_value)
				: R(p_value), G(p_value), B(p_value) { }

			TRGBPixel(TComponentType p_red, TComponentType p_green, TComponentType p_blue)
				: R(p_red), G(p_green), B(p_blue) { }

			TRGBPixel(const TRGBPixel<TComponentType, TMin, TMax> &p_rgb)
				: R(p_rgb.R), G(p_rgb.G), B(p_rgb.B) { }

			//----------------------------------------------------------------------------------------------
			// Misc
			//----------------------------------------------------------------------------------------------
			void Clear(void) { 
				R = G = B = TMin;
			}

			//----------------------------------------------------------------------------------------------
			// Index operators
			//----------------------------------------------------------------------------------------------
			TComponentType operator[](int p_nIndex) const { return Element[p_nIndex]; }
			TComponentType& operator[](int p_nIndex) { return Element[p_nIndex]; }

			//----------------------------------------------------------------------------------------------
			// Arithmetic operators
			//----------------------------------------------------------------------------------------------
			inline TRGBPixel<TComponentType, TMin, TMax>& operator=(const TRGBPixel<TComponentType, TMin, TMax> &p_rgb)
			{
				R = p_rgb.R; G = p_rgb.G; B = p_rgb.B;
				return *this;
			}

			inline TRGBPixel<TComponentType, TMin, TMax>& operator+=(const TRGBPixel<TComponentType, TMin, TMax> &p_rgb) {
				return *this = *this + p_rgb;
			}

			inline TRGBPixel<TComponentType, TMin, TMax>& operator-=(const TRGBPixel<TComponentType, TMin, TMax> &p_rgb) {
				return *this = *this - p_rgb;
			}
			
			inline TRGBPixel<TComponentType, TMin, TMax>& operator*=(const TRGBPixel<TComponentType, TMin, TMax> &p_rgb) {
				return *this = *this * p_rgb;
			}

			inline TRGBPixel<TComponentType, TMin, TMax>& operator/=(const TRGBPixel<TComponentType, TMin, TMax> &p_rgb) {
				return *this = *this / p_rgb;
			}
			
			inline TRGBPixel<TComponentType, TMin, TMax> operator/(const TRGBPixel<TComponentType, TMin, TMax> &p_rgb) const {
				BOOST_ASSERT(p_rgb.R > 0 && p_rgb.G > 0 && p_rgb.B > 0);
				return TRGBPixel<TComponentType, TMin, TMax>(R / p_rgb.R, G / p_rgb.G, B / p_rgb.B);
			}

			inline TRGBPixel<TComponentType, TMin, TMax> operator*(const TRGBPixel<TComponentType, TMin, TMax> &p_rgb) const {
				return TRGBPixel<TComponentType, TMin, TMax>(R * p_rgb.R, G * p_rgb.G, B * p_rgb.B);
			}

			inline TRGBPixel<TComponentType, TMin, TMax> operator+(const TRGBPixel<TComponentType, TMin, TMax> &p_rgb) const {
				return TRGBPixel<TComponentType, TMin, TMax>(R + p_rgb.R, G + p_rgb.G, B + p_rgb.B);
			}

			inline TRGBPixel<TComponentType, TMin, TMax> operator-(const TRGBPixel<TComponentType, TMin, TMax> &p_rgb) const {
				return TRGBPixel<TComponentType, TMin, TMax>(R - p_rgb.R, G - p_rgb.G, B - p_rgb.B);
			}

			void Set(TComponentType p_red, TComponentType p_green, TComponentType p_blue) {
				R = p_red; G = p_green; B = p_blue;
			}

			inline TRGBPixel<TComponentType, TMin, TMax>& operator*=(TComponentType p_scale) {
				return *this = *this * p_scale;
			}

			inline TRGBPixel<TComponentType, TMin, TMax>& operator/=(TComponentType p_scale) {
				return *this = *this / p_scale;
			}

			inline TRGBPixel<TComponentType, TMin, TMax> operator*(TComponentType p_scale) const {
				return TRGBPixel<TComponentType, TMin, TMax>(R * p_scale, G * p_scale, B * p_scale);
			}
			
			inline TRGBPixel<TComponentType, TMin, TMax> operator/(TComponentType p_scale) const 
			{
				BOOST_ASSERT(p_scale > 0);
				return TRGBPixel<TComponentType, TMin, TMax>(R / p_scale, G / p_scale, B / p_scale);
			}

			// Clamp
			void Clamp(TComponentType p_min = TMin, TComponentType p_max = TMax)
			{
				if (R < p_min) R = p_min;
				else if (R > p_max) R = p_max;

				if (G < p_min) G = p_min;
				else if (G > p_max) G = p_max;

				if (B < p_min) B = p_min;
				else if (B > p_max) B = p_max;
			}

			TComponentType Luminance(void) const 
			{
				return (unsigned char)(0.2126 * (double)R + 0.7152 * (double)G + 0.0722 * (double)B);
			}

			virtual void Normalize(void) 
			{
				double length = R * R + G * G + B * B;
				
				if (length > 0.0)
				{
					length = ((TComponentType)TMax) / Maths::Sqrt(length);
					
					R = (TComponentType)(length * R); 
					G = (TComponentType)(length * G); 
					B = (TComponentType)(length * B); 
				}
			}

			std::string ToString(void)
			{
				boost::format formatter;
				std::string strOut = boost::str(boost::format("[%d %d %d]") % (float)R % (float)G % (float)B);
				return strOut;
			}
		};

		template <typename TComponentType, int TMin, int TMax>
		inline TRGBPixel<TComponentType, TMin, TMax> operator*(TComponentType p_scale, const TRGBPixel<TComponentType, TMin, TMax> &p_colour) {
			return TRGBPixel<TComponentType, TMin, TMax>(p_colour.R * p_scale, p_colour.G * p_scale, p_colour.B * p_scale); 
		}

		/*
		template <class TComponent, int TMin, int TMax> TRGBPixel<TComponent, TMin, TMax> const TRGBPixel<TComponent, TMin, TMax>::White;
		template <class TComponent, int TMin, int TMax> TRGBPixel<TComponent, TMin, TMax> const TRGBPixel<TComponent, TMin, TMax>::Black;
		template <class TComponent, int TMin, int TMax> TRGBPixel<TComponent, TMin, TMax> const TRGBPixel<TComponent, TMin, TMax>::Red;
		template <class TComponent, int TMin, int TMax> TRGBPixel<TComponent, TMin, TMax> const TRGBPixel<TComponent, TMin, TMax>::Green;
		template <class TComponent, int TMin, int TMax> TRGBPixel<TComponent, TMin, TMax> const TRGBPixel<TComponent, TMin, TMax>::Blue;
		*/

		//----------------------------------------------------------------------------------------------
		// RGBPixel, 1 byte per colour channel (0 - 255)
		//----------------------------------------------------------------------------------------------
		typedef TRGBPixel<unsigned char, 0, 255> RGBPixel1I;
		
		/*
		const RGBPixel1I RGBPixel1I::White = RGBPixel1I(255);
		const RGBPixel1I RGBPixel1I::Black = RGBPixel1I(0);
		const RGBPixel1I RGBPixel1I::Red = RGBPixel1I(255, 0, 0);
		const RGBPixel1I RGBPixel1I::Green = RGBPixel1I(0, 255, 0);
		const RGBPixel1I RGBPixel1I::Blue = RGBPixel1I(0, 0, 255);
		*/

		//----------------------------------------------------------------------------------------------
		// RGBPixel, 4 bytes per colour channel (0.f - 1.0f)
		//----------------------------------------------------------------------------------------------
		typedef TRGBPixel<float, 0, 1> RGBPixel4F;
		
		/*
		const RGBPixel4F RGBPixel4F::White = RGBPixel4F(1.f);
		const RGBPixel4F RGBPixel4F::Black = RGBPixel4F(0.f);
		const RGBPixel4F RGBPixel4F::Red = RGBPixel4F(1.f, 0.f, 0.f);
		const RGBPixel4F RGBPixel4F::Green = RGBPixel4F(0.f, 1.f, 0.f);
		const RGBPixel4F RGBPixel4F::Blue = RGBPixel4F(0.f, 0.f, 1.f);
		*/

		//----------------------------------------------------------------------------------------------
		// Common aliases
		//----------------------------------------------------------------------------------------------
		typedef RGBPixel1I RGBBytePixel;
		typedef RGBPixel4F RGBPixel;
	} 
}