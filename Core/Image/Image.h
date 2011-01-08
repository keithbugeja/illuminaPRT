//----------------------------------------------------------------------------------------------
//	Filename:	Image.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	A simple image class to store an array of RGB colours that can be accessed
//  via one or two-dimensional indexing. 
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "Image/RGBPixel.h"

//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		class Image
		{
		protected:
			int m_nWidth,
				m_nHeight;

			bool m_bIsOwner;

			RGBPixel *m_bitmap;

		protected:
			inline int IndexOf(int p_x, int p_y);

		public:
			inline int GetWidth(void) const;
			inline int GetHeight(void) const;
			inline int GetLength(void) const;
			
			inline RGBPixel operator[](int p_nIndex) const;
			inline RGBPixel& operator[](int p_nIndex);

			Image(int p_nWidth, int p_nHeight);
			Image(int p_nWidth, int p_nHeight, const RGBPixel &p_rgb);
			Image(int p_nWidth, int p_nHeight, RGBPixel *p_pRGBBuffer);
			~Image(void);

			void Set(int p_x, int p_y, const RGBPixel &p_colour);
			RGBPixel Get(int p_x, int p_y);

			void GammaCorrect(float p_fGamma);

			float* GetImageBuffer(void) const;
		};

		typedef boost::shared_ptr<Image> ImagePtr;
	} 
}

#include "Image/Image.inl"