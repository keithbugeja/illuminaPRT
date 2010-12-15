//----------------------------------------------------------------------------------------------
//	Filename:	Plane.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"

#include "Maths/Maths.h"
#include "Geometry/Vector3.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class Plane
		{
		public:
			enum Side
			{
				Side_Neither,
				Side_Negative,
				Side_Positive,
				Side_Both,

				Side_Count
			};

		public:
			Plane(void);
			Plane(const Plane &p_plane);
			Plane(const Vector3 &p_normal, float p_fDistance);
			Plane(const Vector3 &p_normal, const Vector3 &p_point);
			Plane(const Vector3 &p_point1, const Vector3 &p_point2, const Vector3 &p_point3);

			inline Vector3 GetReflectionVector(const Vector3 &p_vector) const;
			inline void GetDisplacementVector(const Vector3 &p_vector, Vector3 &p_out) const;
			inline Vector3 GetDisplacementVector(const Vector3 &p_vector) const;
			inline void GetReflectionVector(const Vector3 &p_vector, Vector3 &p_out) const;
			inline float GetDisplacement(const Vector3& p_vector) const;
			inline float GetDistance(const Vector3 &p_vector) const;

			Side GetSide(const Vector3 &p_vector) const;
			Side GetSide(const Vector3 &p_midPoint, const Vector3 &p_halfVector) const;

			inline void Normalize(void);

			bool operator==(const Plane &p_plane) const;
			bool operator!=(const Plane &p_plane) const;

			std::string ToString(void) const;

		public:
			float	Distance;
			Vector3 Normal;
		};
	}
}

#include "Geometry/Plane.inl"
