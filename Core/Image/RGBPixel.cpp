//----------------------------------------------------------------------------------------------
//	Filename:	RGB.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Representation of the RGB colour space.
//----------------------------------------------------------------------------------------------
#include "Image/RGBPixel.h"

using namespace Illumina::Core;

int rgbPixelDummyToPreventCompilerWarning = 0;

/* */
RGBPixel const RGBPixel::White	= RGBPixel(1.0f);
RGBPixel const RGBPixel::Black	= RGBPixel(0.0f);
RGBPixel const RGBPixel::Red	= RGBPixel(1.0f, 0.0f, 0.0f);
RGBPixel const RGBPixel::Green	= RGBPixel(0.0f, 1.0f, 0.0f);
RGBPixel const RGBPixel::Blue	= RGBPixel(0.0f, 0.0f, 1.0f);
/* */