//----------------------------------------------------------------------------------------------
//	Filename:	BasicSpace.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Space/Space.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		class BasicSpace 
			: public ISpace
		{
		public:
			BasicSpace(const std::string &p_strId);
			BasicSpace(void);

			bool Initialise(void);
			void Shutdown(void);
			bool Build(void);
			bool Update(void);

			bool Intersects(const Ray &p_ray, float p_fTime) const;
			bool Intersects(const Ray &p_ray, float p_fTime, IPrimitive *p_pExclude) const;
			bool Intersects(const Ray &p_ray, float p_fTime, Intersection &p_intersection) const;
			bool Intersects(const Ray &p_ray, float p_fTime, Intersection &p_intersection, IPrimitive *p_pExclude) const;
		};
	} 
}