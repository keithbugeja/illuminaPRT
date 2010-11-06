//----------------------------------------------------------------------------------------------
//	Filename:	Image.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	A simple image class to store an array of RGB colours that can be accessed
//  via one or two-dimensional indexing. 
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>

#include "Image/RGBPixel.h"

namespace Illumina 
{
	namespace Core
	{
		class Image
		{
		protected:
			int m_nWidth,
				m_nHeight;

			RGBPixel *m_bitmap;

		protected:
			inline int IndexOf(int p_x, int p_y) 
			{
				BOOST_ASSERT(p_x + p_y * m_nWidth >= 0 &&
					p_x + p_y * m_nWidth < m_nHeight * m_nWidth);
			
				return p_x + p_y * m_nWidth;
			}

		public:
			inline int GetWidth() const { return m_nWidth; }
			inline int GetHeight() const { return m_nHeight; }
			inline int GetLength() const { return m_nWidth * m_nHeight; }
			
			inline RGBPixel operator[](int p_nIndex) const {
				BOOST_ASSERT(p_nIndex >= 0 && p_nIndex < m_nWidth * m_nHeight);
				return RGBPixel(m_bitmap[p_nIndex]);
			}

			inline RGBPixel& operator[](int p_nIndex) {
				BOOST_ASSERT(p_nIndex >= 0 && p_nIndex < m_nWidth * m_nHeight);
				return m_bitmap[p_nIndex];
			}

		public:
			Image(int p_nWidth, int p_nHeight)
				: m_nWidth(p_nWidth)
				, m_nHeight(p_nHeight)
			{
				m_bitmap = new RGBPixel[m_nWidth * m_nHeight];
				
				for (int i = 0; i < m_nWidth * m_nHeight; i++)
					m_bitmap[i].Set(0.0f, 0.0f, 0.0f);
			}

			Image(int p_nWidth, int p_nHeight, const RGBPixel &p_rgb)
				: m_nWidth(p_nWidth)
				, m_nHeight(p_nHeight)
			{
				m_bitmap = new RGBPixel[m_nWidth * m_nHeight];

				for (int i = 0; i < m_nWidth * m_nHeight; i++)
					m_bitmap[i] = p_rgb;
			}

			~Image(void) {
				delete[] m_bitmap;
			}

			/* inline */ void Set(int p_x, int p_y, const RGBPixel &p_colour) {
				m_bitmap[IndexOf(p_x, p_y)] = p_colour;
			} 

			inline RGBPixel Get(int p_x, int p_y) {
				return RGBPixel(m_bitmap[IndexOf(p_x, p_y)]);
			}

			void GammaCorrect(float p_fGamma)
			{
				float fPower = 1.0f / p_fGamma;

				for (int i = 0; i < m_nWidth * m_nHeight; i++)
				{
					m_bitmap[i].Set(Maths::Pow(m_bitmap[i].R, fPower),
						Maths::Pow(m_bitmap[i].G, fPower),
						Maths::Pow(m_bitmap[i].B, fPower));
				}
			}
		};

		typedef boost::shared_ptr<Image> ImagePtr;
	} 
}