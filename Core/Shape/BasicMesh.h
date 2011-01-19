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
			boost::shared_ptr<ITriangleMesh> CreateInstance(void) {
				return boost::shared_ptr<ITriangleMesh>(new BasicMesh());
			}
			//----------------------------------------------------------------------------------------------
			bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
			{
				bool bIntersect = false;
				Ray ray(p_ray);

				for (int idx = 0, count = (int)ITriangleMesh::TriangleList.Size(); idx < count; idx++)
				{
					if (ITriangleMesh::TriangleList[idx].Intersects(ray, p_fTime, p_surface))
					{
						ray.Max = p_surface.Distance;
						bIntersect = true;
					}
				}

				return bIntersect;
			}
			//----------------------------------------------------------------------------------------------
			bool Intersects(const Ray &p_ray, float p_fTime)
			{
				for (int idx = 0, count = (int)ITriangleMesh::TriangleList.Size(); idx < count; idx++)
				{
					if (ITriangleMesh::TriangleList[idx].Intersects(p_ray, p_fTime))
						return true;					
				}

				return false;
			}
			//----------------------------------------------------------------------------------------------
		};
	} 
}
