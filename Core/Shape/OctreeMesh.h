//----------------------------------------------------------------------------------------------
//	Filename:	AggregateMesh.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/shared_ptr.hpp>

#include "Shape/TriangleMesh.h"
#include "Accelerator/Octree.h"

namespace Illumina 
{
	namespace Core
	{
		template<class T, class U> 
		class OctreeMesh
			: public ITriangleMesh<T, U>
		{
		protected:
			Octree<T*> *m_triangleOctree;
		public:
			OctreeMesh(void) 
				: m_triangleOctree(NULL)
			{}

			boost::shared_ptr<ITriangleMesh<T, U>> CreateInstance(void) {
				return boost::shared_ptr<ITriangleMesh<T, U>>(new OctreeMesh<T, U>());
			}

			bool Compile(void) 
			{
				if (m_triangleOctree != NULL)
					delete m_triangleOctree;

				ITriangleMesh::ComputeBoundingVolume();
				m_triangleOctree = new Octree<T*>(m_boundingBox);

				for (int idx = 0, count = TriangleList.Size(); idx < count; idx++)
				{
					T &triangle = TriangleList[idx];
					triangle.ComputeBoundingVolume();
					IBoundingVolume* pVolume = triangle.GetBoundingVolume();
					m_triangleOctree->Add(&triangle, pVolume->GetCentre(), pVolume->GetRadius());
				}

				return true;
			}

			bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
			{
				bool bIntersect = false;

				List<T*> intersectionList;
				Ray ray(p_ray);

				m_triangleOctree->Traverse(ray, intersectionList);

				for (int idx = 0; idx < intersectionList.Size(); idx++)
				{
					const T* pTri = intersectionList[idx];

					if (pTri->Intersect(ray, p_fTime, p_surface))
					{
						ray.Max = p_surface.Distance;
						bIntersect = true;
					}
				}

				return bIntersect;
			}

			bool Intersects(const Ray &p_ray, float p_fTime)
			{
				bool bIntersect = false;

				List<T*> intersectionList;
				Ray ray(p_ray);

				m_triangleOctree->Traverse(ray, intersectionList);

				return intersectionList.Size() > 0;
			}
		};
	} 
}
