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
#include "Image/Surface.h"

//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		class Image
			: public RGBSurface
		{
		public:
			Image(int p_nWidth, int p_nHeight);
			Image(int p_nWidth, int p_nHeight, const RGBPixel &p_rgb);
			Image(int p_nWidth, int p_nHeight, RGBPixel *p_pRGBBuffer);

			float* GetImageBuffer(void) const;

			void GammaCorrect(float p_fGamma);

			void MakeTestCard(void);
		};

		typedef boost::shared_ptr<Image> ImagePtr;
	} 
}

#include "Image/Image.inl"