//----------------------------------------------------------------------------------------------
//	Filename:	BasicMesh.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/shared_ptr.hpp>
#include "Shape/TriangleMesh.h"

namespace Illumina 
{
	namespace Core
	{
		class BasicMesh
			: public ITriangleMesh
		{
		public:
			//----------------------------------------------------------------------------------------------
			BasicMesh(void) : ITriangleMesh() { }
			BasicMesh(const std::string &p_strName) : ITriangleMesh(p_strName) { }
			//----------------------------------------------------------------------------------------------
			boost::shared_ptr<ITriangleMesh> CreateInstance(void);
			//----------------------------------------------------------------------------------------------
			bool Intersects(const Ray &p_ray, DifferentialSurface &p_surface);
			bool Intersects(const Ray &p_ray);
			//----------------------------------------------------------------------------------------------
		};
	} 
}
