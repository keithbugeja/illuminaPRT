//----------------------------------------------------------------------------------------------
//	Filename:	Vector2.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Implementation for Vectors in R2
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>

#include "Maths/Maths.h"
#include "Threading/List.h"

namespace Illumina 
{
	namespace Core
	{
		class Vector2
		{
		public:
			static const Vector2 Zero;
			static const Vector2 UnitXNeg;
			static const Vector2 UnitXPos;
			static const Vector2 UnitYNeg;
			static const Vector2 UnitYPos;

			union
			{
				float Element[2];
				struct { float X, Y; };
				struct { float U, V; };
			};

		public:
			Vector2() {}

			Vector2(float p_fValue)
				: X(p_fValue), Y(p_fValue) {}

			Vector2(float p_x, float p_y)
				: X(p_x), Y(p_y) {}

			Vector2(const Vector2 &p_vector)
				: X(p_vector.X), Y(p_vector.Y) {}

			float operator[](int p_nIndex) const { return Element[p_nIndex]; }
			float& operator[](int p_nIndex) { return Element[p_nIndex]; }

			inline void Set(float p_x, float p_y) {
				X = p_x; Y = p_y;
			}
	
			Vector2& operator=(const Vector2 &p_vector)
			{
				X = p_vector.X;
				Y = p_vector.Y;
				
				return *this;
			}

			inline bool operator==(const Vector2 &p_vector) const
			{
				if (X != p_vector.X) return false;
				if (Y != p_vector.Y) return false;

				return true;
			}

			inline bool operator!=(const Vector2& p_vector) const {
				return !(*this == p_vector);
			}

			inline Vector2 operator*(float p_fValue) const {
				return Vector2(p_fValue * X, p_fValue * Y);
			}

			inline Vector2 operator/(float p_fValue) const {
				BOOST_ASSERT(p_fValue != 0.0f);
				return Vector2(*this * (1.0f / p_fValue));
			}

			inline Vector2 operator*(const Vector2 &p_vector) const {
				return Vector2(p_vector.X * X, p_vector.Y * Y);
			}

			inline Vector2 operator+(const Vector2 &p_vector) const {
				return Vector2(X + p_vector.X, Y + p_vector.Y);
			}

			inline Vector2 operator-(const Vector2 &p_vector) const {
				return Vector2(X - p_vector.X, Y - p_vector.Y);
			}

			inline Vector2 operator-(void) const {
				return Vector2(-X, -Y);
			}

			inline Vector2& operator*=(float p_fValue) {
				return *this = *this * p_fValue;
			}

			inline Vector2& operator*=(const Vector2 &p_vector) {
				return *this = *this * p_vector;
			}

			inline Vector2& operator/=(float p_fValue) {
				return *this = *this / p_fValue;
			}

			inline Vector2& operator+=(const Vector2 &p_vector) {
				return *this = *this + p_vector;
			}
			
			inline Vector2& operator-=(const Vector2 &p_vector) {
				return *this = *this - p_vector;
			}

			inline float MaxComponent() const {
				return Maths::Max(X, Y);
			}

			inline float MinComponent() const {
				return Maths::Min(X, Y);
			}

			inline float MaxAbsComponent() const {
				return Maths::Max(Maths::FAbs(X), Maths::FAbs(Y));
			}

			inline float MinAbsComponent() const
			{
				return Maths::Min(Maths::FAbs(X), Maths::FAbs(Y));
			}

			static Vector2 Max(const Vector2 &p_vector1, const Vector2 &p_vector2) 
			{
				return Vector2(Maths::Max(p_vector1.X, p_vector2.X),
					Maths::Max(p_vector1.Y, p_vector2.Y));
			}

			static Vector2 Min(const Vector2 &p_vector1, const Vector2 &p_vector2) 
			{
				return Vector2(Maths::Min(p_vector1.X, p_vector2.X),
					Maths::Min(p_vector1.Y, p_vector2.Y));
			}

			inline float Length(void) const {
				return Maths::Sqrt(X * X + Y * Y);
			}

			inline float LengthSquared(void) const {
				return X * X + Y * Y;
			}

			inline void Normalize(void) {
				*this = Vector2::Normalize(*this);
			}

			inline float Dot(const Vector2 &p_vector) const {
				return Vector2::Dot(*this, p_vector);
			}

			inline float AbsDot(const Vector2 &p_vector) const {
				return Vector2::AbsDot(*this, p_vector);
			}

			static float Dot(const Vector2 &p_vector1, const Vector2 &p_vector2) {
				return p_vector1.X * p_vector2.X + p_vector1.Y * p_vector2.Y;
			}

			static float AbsDot(const Vector2 &p_vector1, const Vector2 &p_vector2) {
				return Maths::FAbs(p_vector1.X * p_vector2.X +
					p_vector1.Y * p_vector2.Y);
			}

			static Vector2 Normalize(const Vector2 &p_vector) {
				return p_vector / Maths::Sqrt(p_vector.Length());
			}

			static float DistanceSquared(const Vector2 &p_point1, const Vector2 &p_point2) {
				return (p_point2 - p_point1).LengthSquared();
			}

			static float Distance(const Vector2 &p_point1, const Vector2 &p_point2) {
				return (p_point2 - p_point1).Length();
			}

			std::string ToString(void)
			{
				std::string strOut = boost::str(boost::format("[%d %d]") % X % Y);
				return strOut;
			}
		};

		inline Vector2 operator*(float p_fValue, const Vector2 &p_vector) {
			return Vector2(p_fValue * p_vector.X, p_fValue * p_vector.Y);
		}

		//typedef std::vector<Vector2> Vector2List;
		typedef List<Vector2> Vector2List;
	}
}