//----------------------------------------------------------------------------------------------
//	Filename:	Texture.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/shared_ptr.hpp>

#include "System/IlluminaPRT.h"
#include "System/FactoryManager.h"

#include "Object/Object.h"

#include "Geometry/Vector2.h"
#include "Geometry/Vector3.h"
#include "Image/RGBPixel.h"

//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		// TextureFiltering
		//----------------------------------------------------------------------------------------------
		enum TextureFiltering
		{
			NearestNeighbour,
			Bilinear
		};

		//----------------------------------------------------------------------------------------------
		// Texture Access Mode
		//----------------------------------------------------------------------------------------------
		enum TextureAccessMode
		{
			ReadOnly,
			WriteOnly,
			ReadWrite
		};

		//----------------------------------------------------------------------------------------------
		// ITexture : Abstract base class for texture objects. 
		//----------------------------------------------------------------------------------------------
		class ITexture
			: public Object
		{
		public:
			ITexture(const std::string &p_strName) : Object(p_strName) { }
			ITexture(void) : Object() { }

			virtual RGBPixel GetValue(const Vector2 &p_uv, const Vector3 &p_hitPoint) const { RGBPixel pixel; GetValue(p_uv, p_hitPoint, pixel); return pixel; }
			virtual RGBPixel GetValue(const Vector2 &p_uv) const { return GetValue(p_uv, Vector3::Zero); }
			virtual RGBPixel GetValue(const Vector3 &p_hitPoint) const { return GetValue(Vector2::Zero, p_hitPoint); }
			
			virtual void GetValue(const Vector2 &p_uv, const Vector3 &p_hitPoint, RGBPixel &p_pixel) const = 0;
			virtual void GetValue(const Vector2 &p_uv, RGBPixel &p_pixel) const { GetValue(p_uv, Vector3::Zero, p_pixel); };
			virtual void GetValue(const Vector3 &p_hitpoint, RGBPixel &p_pixel) const { GetValue(Vector2::Zero, p_hitpoint, p_pixel); };
			
            std::string ToString(void) const { return "ITexture"; }
		};

		typedef boost::shared_ptr<ITexture> TexturePtr; 

		//----------------------------------------------------------------------------------------------
		// TextureManager : All Texture factories must register with object.
		//----------------------------------------------------------------------------------------------
		typedef FactoryManager<ITexture> TextureManager;
	} 
}