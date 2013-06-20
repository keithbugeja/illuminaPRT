//----------------------------------------------------------------------------------------------
//	Filename:	BasicSpace.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Space/Space.h"
#include "Geometry/BoundingBox.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		class BasicSpace 
			: public ISpace
		{

		protected:
			bool m_bRebuildRequired;
			AxisAlignedBoundingBox m_aabb;

		protected:
			void ComputeBoundingBox(void);

		public:
			BasicSpace(const std::string &p_strName) : ISpace(p_strName) { }
			BasicSpace(void) { }

			IBoundingVolume *GetBoundingVolume(void);

			bool Initialise(void);
			void Shutdown(void);
			bool Build(void);
			bool Update(void);

			bool Intersects(const Ray &p_ray) const;
			bool Intersects(const Ray &p_ray, IPrimitive *p_pExclude) const;
			bool Intersects(const Ray &p_ray, Intersection &p_intersection) const;
			bool Intersects(const Ray &p_ray, Intersection &p_intersection, IPrimitive *p_pExclude) const;
		};
	} 
}