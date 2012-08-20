//----------------------------------------------------------------------------------------------
//	Filename:	Syrface.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "Image/RGBPixel.h"

//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		template <class TPixelType>
		class TSurface
		{
		protected:
			int m_nWidth,
				m_nHeight;

			bool m_bIsOwner;

			TPixelType *m_bitmap;

		protected:
			inline int IndexOf(int p_x, int p_y);

		public:
			inline int GetWidth(void) const;
			inline int GetHeight(void) const;
			inline int GetArea(void) const;
			
			inline TPixelType operator[](int p_nIndex) const;
			inline TPixelType& operator[](int p_nIndex);

			TSurface(int p_nWidth, int p_nHeight);
			TSurface(int p_nWidth, int p_nHeight, const TPixelType &p_rgb);
			TSurface(int p_nWidth, int p_nHeight, TPixelType *p_pRGBBuffer);
			~TSurface(void);

			inline void Set(int p_x, int p_y, const TPixelType &p_colour);
			inline void Get(int p_x, int p_y, TPixelType &p_colour);
			inline TPixelType Get(int p_x, int p_y);

			TPixelType* GetScanline(int p_nScanline) const;
			TPixelType* GetSurfaceBuffer(void) const;
		};

		typedef TSurface<RGBPixel4F> RGBSurface4F;
		typedef TSurface<RGBPixel1I> RGBSurface1I;
		typedef TSurface<RGBPixel> RGBSurface;
	}
}

#include "Image/Surface.inl"