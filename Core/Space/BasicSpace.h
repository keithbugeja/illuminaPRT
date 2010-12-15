//----------------------------------------------------------------------------------------------
//	Filename:	BasicSpace.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>

#include "Space/Space.h"
#include "Geometry/Intersection.h"

namespace Illumina 
{
	namespace Core
	{
		class BasicSpace 
			: public ISpace
		{
		public:
			void Initialise(void)  { }

			void Build(void) { }
			void Update(void) { }

			bool Intersects(const Ray &p_ray, float p_fTime, Intersection &p_intersection) const
			{
				Ray ray(p_ray);
				bool bHit = false;

				for (int primitiveIdx = 0, count = (int)PrimitiveList.Size(); primitiveIdx < count; primitiveIdx++)
				{
					if (PrimitiveList[primitiveIdx]->Intersect(ray, p_fTime, p_intersection))
					{
						ray.Max = p_intersection.Surface.Distance;
						bHit = true;
					}
				}

				return bHit;
			}

			bool Intersects(const Ray &p_ray, float p_fTime) const
			{
				for (int primitiveIdx = 0, count = (int)PrimitiveList.Size(); primitiveIdx < count; primitiveIdx++)
				{
					if (PrimitiveList[primitiveIdx]->Intersect(p_ray, p_fTime))
						return true;
				}

				return false;
			}
		};
	} 
}