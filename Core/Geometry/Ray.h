//----------------------------------------------------------------------------------------------
//	Filename:	Ray.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Geometry/Vector3.h"
#include "Geometry/Transform.h"

//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		class Ray
		{
		public:
			Vector3 Origin;
			Vector3 Direction;
		
			Vector3 DirectionInverseCache;
		
		public:
			float Min, Max;

		public:
			Ray(void);
			Ray(const Vector3 &p_origin, const Vector3 &p_direction);
			Ray(const Vector3 &p_origin, const Vector3 &p_direction, float p_fMin, float p_fMax = Maths::Maximum);
			Ray(const Ray &p_ray);

			inline Ray operator-(void) const;

			inline void Set(const Vector3 &p_origin, const Vector3 &p_direction, float p_fMin = 0.0f, float p_fMax = Maths::Maximum);
			inline void SetDirection(const Vector3 &p_direction);

			inline Vector3 PointAlongRay(float p_fDistance) const;
			inline void PointAlongRay(float p_fDistance, Vector3 &p_out);

			inline Ray Apply(const Transformation &p_transformation) const;
			inline Ray ApplyInverse(const Transformation &p_transformation) const;

			inline Ray& operator=(const Ray &p_ray);
			
			static void Apply(const Transformation &p_transformation, const Ray &p_ray, Ray &p_out);
			static void ApplyInverse(const Transformation &p_transformation, const Ray &p_ray, Ray &p_out);

			std::string ToString(void) const;
		};
	} 
}

#include "Geometry/Ray.inl"