//----------------------------------------------------------------------------------------------
//	Filename:	ImageIO.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Defines the IImageIO interface. Implementations which load and save files in
//	different file formats must implement this interface.
//----------------------------------------------------------------------------------------------
#pragma once

#include "Image/Image.h"

namespace Illumina 
{
	namespace Core
	{
		class IImageIO
		{
		public:
			virtual Image* Load(const std::string &p_strImageFile) = 0;
			virtual void Save(const Image &p_image, const std::string &p_strImageFile) = 0;
		};
	} 
}