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

namespace Illumina 
{
	namespace Core
	{
		class BasicSpace 
			: public ISpace
		{
		public:
			void Initialise(void) 
			{
				for (int primitiveIdx = 0, count = (int)PrimitiveList.Size(); primitiveIdx < count; primitiveIdx++)
				{
				}
			}

			void Build(void) { }
			void Update(void) { }

			bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface& p_surface, float& p_fTestDensity) 
			{
				p_fTestDensity = 0.0f;

				return Intersects(p_ray, p_fTime, p_surface);
			}

			bool Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface) const
			{
				Ray ray(p_ray);
				bool bHit = false;

				for (int primitiveIdx = 0, count = (int)PrimitiveList.Size(); primitiveIdx < count; primitiveIdx++)
				{
					if (PrimitiveList[primitiveIdx]->Intersect(ray, p_fTime, p_surface))
					{
						ray.Max = p_surface.Distance;
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