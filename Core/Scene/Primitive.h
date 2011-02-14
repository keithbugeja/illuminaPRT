//----------------------------------------------------------------------------------------------
//	Filename:	Primitive.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		class IPrimitive
		{
		public:
			virtual bool IsBounded(void) const = 0;
			virtual boost::shared_ptr<IBoundingVolume> GetWorldBounds(void) const  = 0;

			virtual Vector3 SamplePoint(float p_u, float p_v, Vector3 &p_normal) = 0;
			virtual Vector3 SamplePoint(const Vector3 &p_viewPoint, float p_u, float p_v, Vector3 &p_normal) = 0;

			virtual bool Intersect(const Ray &p_ray, float p_fTime, Intersection &p_intersection) = 0;
			virtual bool Intersect(const Ray &p_ray, float p_fTime) = 0;

			virtual std::string ToString(void) const = 0;
		};
	} 
}