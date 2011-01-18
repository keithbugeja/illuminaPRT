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
		template<class T, class U> 
		class BasicMesh
			: public ITriangleMesh<T, U>
		{
		public:
			//----------------------------------------------------------------------------------------------
			BasicMesh(void) : ITriangleMesh<T, U>() { }
			BasicMesh(const std::string &p_strName) : ITriangleMesh<T, U>(p_strName) { }
			//----------------------------------------------------------------------------------------------
			boost::shared_ptr<ITriangleMesh<T, U>> CreateInstance(void) {
				return boost::shared_ptr<ITriangleMesh<T, U>>(new BasicMesh<T, U>());
			}
			//----------------------------------------------------------------------------------------------
			bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
			{
				bool bIntersect = false;
				Ray ray(p_ray);

				for (int idx = 0, count = (int)ITriangleMesh<T, U>::TriangleList.Size(); idx < count; idx++)
				{
					if (ITriangleMesh<T, U>::TriangleList[idx].Intersects(ray, p_fTime, p_surface))
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
				for (int idx = 0, count = (int)ITriangleMesh<T, U>::TriangleList.Size(); idx < count; idx++)
				{
					if (ITriangleMesh<T, U>::TriangleList[idx].Intersects(p_ray, p_fTime))
						return true;					
				}

				return false;
			}
			//----------------------------------------------------------------------------------------------
		};
	} 
}
