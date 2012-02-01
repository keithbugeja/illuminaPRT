//----------------------------------------------------------------------------------------------
//	Filename:	Vector.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Implementation for Vectors in R3
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"

#include "Maths/Maths.h"
#include "Threading/List.h"

//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		class Vector3
		{
		public:
			/* Common vector configurations */
			static const Vector3 Ones;
			static const Vector3 Zero;
			static const Vector3 UnitXNeg;
			static const Vector3 UnitXPos;
			static const Vector3 UnitYNeg;
			static const Vector3 UnitYPos;
			static const Vector3 UnitZNeg;
			static const Vector3 UnitZPos;

			/* Vector elements */
			ALIGN_16 union
			{
				struct	{ float X, Y, Z, W; };
				float	Element[4];
			};

		public:
			/* Default vector constructor */
			Vector3();

			/* Constructor which initialises vector elements to a single value */
			Vector3(float p_fValue);

			/* Constructor which initialises elements */
			Vector3(float p_x, float p_y, float p_z);

			/* Copy constructor */
			Vector3(const Vector3 &p_vector);

			/* Overloading indexing operator */
			inline float operator[](int p_nIndex) const;
			inline float& operator[](int p_nIndex);

			inline void Set(float p_x, float p_y, float p_z);

			inline Vector3& operator=(const Vector3 &p_vector);
			inline bool operator==(const Vector3 &p_vector) const;
			inline bool operator!=(const Vector3& p_vector) const;

			inline Vector3 operator*(float p_fValue) const;
			inline Vector3 operator/(float p_fValue) const;
			inline Vector3 operator*(const Vector3 &p_vector) const;
			inline Vector3 operator+(const Vector3 &p_vector) const;
			inline Vector3 operator-(const Vector3 &p_vector) const;
			inline Vector3 operator-(void) const;

			inline Vector3& operator*=(float p_fValue);
			inline Vector3& operator*=(const Vector3 &p_vector);
			inline Vector3& operator/=(float p_fValue);
			inline Vector3& operator+=(const Vector3 &p_vector);
			inline Vector3& operator-=(const Vector3 &p_vector);

			inline bool IsOnes() const;
			inline bool IsZero() const;

			inline float MaxComponent() const;
			inline float MinComponent() const;
			inline float MaxAbsComponent() const;
			inline float MinAbsComponent() const;
			inline int ArgMaxAbsComponent() const;
			inline int ArgMinAbsComponent() const;
			inline int ArgMaxComponent() const;
			inline int ArgMinComponent() const;

			inline void Inverse(void);
			inline void Normalize(void);
			inline float Length(void) const;
			inline float LengthSquared(void) const;
			inline float Dot(const Vector3 &p_vector) const;
			inline float AbsDot(const Vector3 &p_vector) const;
			inline Vector3 Cross(const Vector3 &p_vector) const;

			static inline void Add(const Vector3 &p_vector1, const Vector3 &p_vector2, Vector3 &p_out);
			static inline void Subtract(const Vector3 &p_vector1, const Vector3 &p_vector2, Vector3 &p_out);
			static inline float Distance(const Vector3& p_point1, const Vector3& p_point2);
			static inline float DistanceSquared(const Vector3& p_point1, const Vector3& p_point2);
			static inline float Dot(const Vector3 &p_vector1, const Vector3 &p_vector2);
			static inline float AbsDot(const Vector3 &p_vector1, const Vector3 &p_vector2);
			static inline float TripleProduct(const Vector3 &p_vector1, const Vector3 &p_vector2, const Vector3 &p_vector3);
			static inline void Cross(const Vector3 &p_vector1, const Vector3 &p_vector2, Vector3 &p_out);
			static inline Vector3 Cross(const Vector3 &p_vector1, const Vector3 &p_vector2);
			static inline Vector3 Normalize(const Vector3 &p_vector);
			static inline void Normalize(const Vector3 &p_vector, Vector3 &p_out);
			static inline Vector3 Inverse(const Vector3 &p_vector);
			static inline void Inverse(const Vector3 &p_vector, Vector3 &p_out);

			static inline Vector3 Max(const Vector3 &p_vector1, const Vector3 &p_vector2);
			static inline Vector3 Min(const Vector3 &p_vector1, const Vector3 &p_vector2);
			static inline void Max(const Vector3 &p_vector1, const Vector3 &p_vector2, Vector3 &p_out);
			static inline void Min(const Vector3 &p_vector1, const Vector3 &p_vector2, Vector3 &p_out);

			static inline void Reflect(const Vector3 &p_vector, const Vector3 &p_normal, Vector3 &p_out);

			std::string ToString(void) const;
		};

		//----------------------------------------------------------------------------------------------
		inline Vector3 operator*(float p_fValue, const Vector3 &p_vector) {
			return Vector3(p_fValue * p_vector.X, p_fValue * p_vector.Y, p_fValue * p_vector.Z);
		}

		/* Type definition for a vector list */
		typedef List<Vector3> Vector3List;
	} 
}

#include "Geometry/Vector3.inl"