//----------------------------------------------------------------------------------------------
//	Filename:	AggregateMesh.h
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
			//static AtomicInt64 m_intersectionCount;

		public:
			long long GetIntersectionCount() { return 0; /*m_intersectionCount;*/ }

			BasicMesh(void) {}

			boost::shared_ptr<ITriangleMesh<T, U>> CreateInstance(void) {
				return boost::shared_ptr<ITriangleMesh<T, U>>(new BasicMesh<T, U>());
			}

			bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
			{
				//int intersectionCount = 0;

				bool bIntersect = false;
				Ray ray(p_ray);

				for (int idx = 0, count = (int)TriangleList.Size(); idx < count; idx++)
				{
					//intersectionCount++;

					if (TriangleList[idx].Intersects(ray, p_fTime, p_surface))
					{
						ray.Max = p_surface.Distance;
						bIntersect = true;
					}
				}

				//m_intersectionCount += intersectionCount;
				return bIntersect;
			}

			bool Intersects(const Ray &p_ray, float p_fTime)
			{
				for (int idx = 0, count = (int)TriangleList.Size(); idx < count; idx++)
				{
					if (TriangleList[idx].Intersects(p_ray, p_fTime))
						return true;					
				}

				return false;
			}
		};

		//template<class T, class U> AtomicInt64 BasicMesh<T,U>::m_intersectionCount(0);
	} 
}
