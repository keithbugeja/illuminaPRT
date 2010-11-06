//----------------------------------------------------------------------------------------------
//	Filename:	CollectionPrimitive.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>

#include "Staging/Aggregate.h"
//#include "Accelerator/Octree.h"

namespace Illumina 
{
	namespace Core
	{
		/*
		class OctreeAggregate : public Aggregate
		{
			Octree<IPrimitive*> *m_pOctree;

		public:
			OctreeAggregate()
				: m_pOctree(NULL)
			{ }

			~OctreeAggregate()
			{
				if (m_pOctree != NULL)
				{
					delete m_pOctree;
					m_pOctree = NULL;
				}
			}

			void Prepare(void) 
			{
				if (m_pOctree != NULL)
				{
					AxisAlignedBoundingBox boundingBox;
					boundingBox.Invalidate();
					
					for (int idx = 0, count = (int)PrimitiveList.Size(); idx < count; idx++)
					{
						BoundingVolumePtr boundingVolume = PrimitiveList[idx]->GetWorldBounds();
						boundingBox.Union(*(boundingVolume.get()));
					}

					m_pOctree = new Octree<IPrimitive*>(boundingBox);

					for (int idx = 0, count = (int)PrimitiveList.Size(); idx < count; idx++)
					{
						BoundingVolumePtr boundingVolume = PrimitiveList[idx]->GetWorldBounds();
						IBoundingVolume* pVolume = boundingVolume.get();
						m_pOctree->Add(PrimitiveList[idx], pVolume->GetCentre(), pVolume->GetRadius());
					}
				}
			};


			bool Intersect(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface) const
			{
				Ray ray(p_ray);
				List<IPrimitive*> primitiveList;
				m_pOctree->Traverse(ray, primitiveList);
				
				bool bHit = false;

				for (int idx = 0, count = (int)primitiveList.Size(); idx < count; idx++)
				{
					if (primitiveList[idx]->Intersect(ray, p_fTime, p_surface))
					{
						ray.Max = p_surface.Distance;
						bHit = true;
					}
				}

				return bHit;
			}

			bool Intersect(const Ray &p_ray, float p_fTime) const
			{
				Ray ray(p_ray);
				List<IPrimitive*> primitiveList;
				m_pOctree->Traverse(ray, primitiveList);

				for (int idx = 0, count = (int)primitiveList.Size(); idx < count; idx++)
				{
					if (primitiveList[idx]->Intersect(ray, p_fTime))
						return true;
				}
			}

			std::string ToString(void) const { return "CollectionPrimitive"; }
		};
		*/
	} 
}