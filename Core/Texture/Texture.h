//----------------------------------------------------------------------------------------------
//	Filename:	Texture.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>

#include "Object/Object.h"
#include "Geometry/Vector2.h"
#include "Geometry/Vector3.h"
#include "Image/RGBPixel.h"

namespace Illumina 
{
	namespace Core
	{
		class ITexture
			: public Object
		{
		public:
			ITexture(void) : Object() { }
			ITexture(const std::string &p_strName) : Object(p_strName) { }

			virtual RGBPixel GetValue(const Vector2 &p_uv, const Vector3 &p_hitPoint) const = 0;
			virtual std::string ToString(void) { return "ITexture"; }
		};

		typedef boost::shared_ptr<ITexture> TexturePtr; 
	} 
}