//----------------------------------------------------------------------------------------------
//	Filename:	Matrix3x3.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Implementation for a 3x3 Matrix
//----------------------------------------------------------------------------------------------
#pragma once

#include <string.h>
#include <boost/format.hpp>

#include "System/Platform.h"
#include "Geometry/Vector3.h"

namespace Illumina
{
	namespace Core
	{
		class Matrix3x3
		{
		public:
			/* Matrix property constants */
			static const int Rows	 = 3;
			static const int Columns = 3;
			static const int Order	 = Rows * Columns;

			/* Common matrix configurations */
			static const Matrix3x3 Identity;
			static const Matrix3x3 Zero;

			/* Matrix cells */
			ALIGN_16 union
			{
				float Elements[Order];
				struct
				{
					float _00, _01, _02,
						  _10, _11, _12,
						  _20, _21, _22;
				};
			};

		public:
			/* Default Matrix constructor */
			Matrix3x3(void) {}

			/* Constructor which initialises structure to identity or zero matrix */
			explicit Matrix3x3(bool p_bIdentity) {
				p_bIdentity ? MakeIdentity() : MakeZero();
			}

			/* Initialises matrix from float array */
			explicit Matrix3x3(const float p_fElements[Order]) {
				memcpy( Elements, p_fElements, sizeof(float) * Order );
			}

			/* Copy constructor */
			Matrix3x3(const Matrix3x3& p_matrix) {
				memcpy( Elements, p_matrix.Elements, sizeof(float) * Order );
			}

			/* Makes a rotation matrix about an arbitrary angle */
			explicit Matrix3x3(const Vector3& p_axis, float p_fAngle) {
				MakeRotation( p_axis, p_fAngle );
			}

			/* Constructs a matrix from three vectors */
			explicit Matrix3x3(const Vector3& p_vector1, const Vector3& p_vector2, const Vector3& p_vector3, bool p_bRowMajor = true)
			{
				if (p_bRowMajor)
				{
					SetRow(0, p_vector1);
					SetRow(1, p_vector2);
					SetRow(2, p_vector3);
				}
				else
				{
					SetColumn(0, p_vector1);
					SetColumn(1, p_vector2);
					SetColumn(2, p_vector3);
				}
			}

			/* Tests whether matrix is singular (no inverse) */
			inline bool IsSingular(void) const {
				return Maths::Equals(Determinant(), 0.0f);
			}

			/* Tests whether matrix is identity */
			inline bool IsIdentity(void) const
			{
				return ( Maths::Equals( _00, 1.0f ) &&
						 Maths::Equals( _11, 1.0f ) &&
						 Maths::Equals( _22, 1.0f ) &&
						 Maths::Equals( _01, 0.0f ) &&
						 Maths::Equals( _02, 0.0f ) &&
						 Maths::Equals( _10, 0.0f ) &&
						 Maths::Equals( _12, 0.0f ) &&
						 Maths::Equals( _20, 0.0f ) &&
						 Maths::Equals( _21, 0.0f ) );
			}

			/* Tests whether structure is a zero matrix */
			inline bool IsZero(void) const
			{
				return ( Maths::Equals( _00, 0.0f ) &&
						 Maths::Equals( _11, 0.0f ) &&
						 Maths::Equals( _22, 0.0f ) &&
						 Maths::Equals( _01, 0.0f ) &&
						 Maths::Equals( _02, 0.0f ) &&
						 Maths::Equals( _10, 0.0f ) &&
						 Maths::Equals( _12, 0.0f ) &&
						 Maths::Equals( _20, 0.0f ) &&
						 Maths::Equals( _21, 0.0f ) );
			}

			/* Clears matrix cells setting it to a zero matrix */
			inline void MakeZero(void) {
				_01 = _02 = _10 = _12 = _20 = _21 = _00 = _11 = _22 = 0.0f;
			}

			/* Turns matrix into an identity matrix */
			void MakeIdentity(void)
			{
				_01 = _02 = _10 = _12 = _20 = _21 = 0.0f;
				_00 = _11 = _22 = 1.0f;
			}

			/* Clears matrix and sets the main diagonal component to	*/
			/* the supplied vector elements								*/
			void MakeDiagonal(const Vector3& p_diagonal)
			{
				_01 = _02 = _10 = _12 = _20 = _21 = 0.0f;

				_00 = p_diagonal.X;
				_11 = p_diagonal.Y;
				_22 = p_diagonal.Z;
			}

			/* Turns matrix into a rotation matrix about an arbitrary axis */
			void MakeRotation(const Vector3& p_axis, float p_fAngle)
			{
				float fCos = Maths::Cos( p_fAngle ),
					  fSin = Maths::Sin( p_fAngle ),
					  fOneMinusCos = 1.0f - fCos;

				float fXY = p_axis.X * p_axis.Y * fOneMinusCos,
					  fXZ = p_axis.X * p_axis.Z * fOneMinusCos,
					  fYZ = p_axis.Y * p_axis.Z * fOneMinusCos,
					  fSinX = p_axis.X * fSin,
					  fSinY = p_axis.Y * fSin,
					  fSinZ = p_axis.Z * fSin;

				_00 = p_axis.X * p_axis.X * fOneMinusCos + fCos;
				_01 = fXY - fSinZ;
				_02 = fXZ + fSinY;

				_10 = fXY + fSinZ;
				_11 = p_axis.Y * p_axis.Y * fOneMinusCos + fCos;
				_12 = fYZ - fSinX;

				_20 = fXZ - fSinY;
				_21 = fYZ + fSinX;
				_22 = p_axis.Z * p_axis.Z * fOneMinusCos + fCos;
			}

			/* Extracts a rotation axis and angle from matrix */
			void ExtractRotation(Vector3 &p_outAxis, float &p_outAngle) const
			{
				// Graphics Gems I, Converting between Matrix and Axis-Amount representations. pg 466
				float tCosTheta = 0.5f * (_00 + _11 + _22 - 1.0f);
				float tSinTheta = Maths::Sqrt((float)1.0 - tCosTheta * tCosTheta);

				if ( Maths::Abs(tSinTheta) > Maths::Epsilon )
				{
					tSinTheta = (float) 0.5 / tSinTheta;

					p_outAxis.X = ( _21 - _12 ) * tSinTheta;
					p_outAxis.Y = ( _02 - _20 ) * tSinTheta;
					p_outAxis.Z = ( _10 - _01 ) * tSinTheta;
					p_outAngle  = Maths::Acos( tCosTheta );
				}
				else
				{
					p_outAxis = Vector3::Zero;
					p_outAngle = 0;
				}
			}

			/* Extracts scaling components from matrix */
			void ExtractScaling(Vector3 &p_scale) const
			{
				p_scale.X = _00;
				p_scale.Y = _11;
				p_scale.Z = _22;
			}

			/* Set scaling components without modifying other elements */
			void SetScaling(const Vector3& p_scale)
			{
				_00 = p_scale.X;
				_11 = p_scale.Y;
				_22 = p_scale.Z;
			}

			/* Sets the specified column cells to the values specified by the given vector */
			void SetColumn(int p_nColumnIndex, const Vector3& p_column)
			{
				BOOST_ASSERT(p_nColumnIndex >= 0 && p_nColumnIndex < Columns);

				switch(p_nColumnIndex)
				{
					case 0:
						_00 = p_column.X; _10 = p_column.Y; _20 = p_column.Z;
						return;

					case 1:
						_01 = p_column.X; _11 = p_column.Y; _21 = p_column.Z;
						return;

					case 2:
						_02 = p_column.X; _12 = p_column.Y; _22 = p_column.Z;
						return;
				}
			}

			/* Returns the specified matrix column as a vector */
			Vector3 GetColumn(int p_nColumnIndex) const
			{
				BOOST_ASSERT(p_nColumnIndex >= 0 && p_nColumnIndex < Columns);

				switch(p_nColumnIndex)
				{
					case 0: return Vector3(_00, _10, _20);
					case 1: return Vector3(_01, _11, _21);
					case 2: return Vector3(_02, _12, _22);
				}

				return Vector3::Zero;
			}

			/* Sets the specified row cells to the values specified by the given vector */
			void SetRow(int p_nRowIndex, const Vector3& p_row)
			{
				BOOST_ASSERT(p_nRowIndex >= 0 && p_nRowIndex < Rows);

				switch(p_nRowIndex)
				{
					case 0:
						_00 = p_row.X; _01 = p_row.Y; _02 = p_row.Z;
						return;

					case 1:
						_10 = p_row.X; _11 = p_row.Y; _12 = p_row.Z;
						return;

					case 2:
						_20 = p_row.X; _21 = p_row.Y; _22 = p_row.Z;
						return;
				}
			}

			/* Returns the specified matrix row as a vector */
			Vector3 GetRow(int p_nRowIndex) const
			{
				BOOST_ASSERT(p_nRowIndex >= 0 && p_nRowIndex < Rows);

				switch( p_nRowIndex )
				{
					case 0: return Vector3(_00, _01, _02);
					case 1: return Vector3(_10, _11, _12);
					case 2: return Vector3(_20, _21, _22);
				}

				return Vector3::Zero;
			}

			/* Computes the determinant of the matrix */
			float Determinant(void) const
			{
				return _00 * (_11*_22 - _21*_12) -
					   _01 * (_10*_22 - _20*_12) +
					   _02 * (_10*_21 - _20*_11);
			}

			/* Negates the matrix such that A = -A */
			void Negate(void)
			{
				for (int n = 0; n < Order; n++)
					Elements[n] = -Elements[n];
			}

			/* Computes the matrix transpose */
			void Transpose(void)
			{
				std::swap(_10, _01);
				std::swap(_20, _02);
				std::swap(_21, _12);
			}

			/* Computes the matrix inverse */
			void Invert(void)
			{
				float fDeterminant = Determinant();

				// Find inverse of determinant
				fDeterminant = 1.0f / fDeterminant;

				// Find inverse of each cell
				float f00 =  (_11*_22 - _12*_21) * fDeterminant,
					  f01 = -(_01*_22 - _21*_02) * fDeterminant,
					  f02 =  (_01*_12 - _11*_02) * fDeterminant,
					  f10 = -(_10*_22 - _12*_20) * fDeterminant,
					  f11 =  (_00*_22 - _20*_02) * fDeterminant,
					  f12 = -(_00*_12 - _10*_02) * fDeterminant,
					  f20 =  (_10*_21 - _10*_11) * fDeterminant,
					  f21 = -(_00*_21 - _20*_01) * fDeterminant,
					  f22 =  (_00*_11 - _01*_10) * fDeterminant;

				// Copy inverse to matrix cells
				_00 = f00; _01 = f01; _02 = f02;
				_10 = f10; _11 = f11; _12 = f12;
				_20 = f20; _21 = f21; _22 = f22;
			}

			/* Overloads functor operator to return element at (row, column) */
			float operator()(int p_nRowIndex, int p_nColumnIndex) const {
				return Elements[Index(p_nRowIndex, p_nColumnIndex)];
			}

			/* Overloads functor operator to return element reference at (row, column) */
			float& operator()(int p_nRowIndex, int p_nColumnIndex) {
				return Elements[Index(p_nRowIndex, p_nColumnIndex)];
			}

			/* Overloads the addition operator */
			Matrix3x3 operator+(const Matrix3x3& p_matrix) const
			{
				Matrix3x3 result;
				Matrix3x3::Add(*this, p_matrix, result);
				return result;
			}

			/* Overloads the subtraction operator */
			Matrix3x3 operator-(const Matrix3x3& p_matrix) const
			{
				Matrix3x3 result;
				Matrix3x3::Subtract(*this, p_matrix, result);
				return result;
			}

			/* Multiplication operator with a scalar value */
			Matrix3x3 operator*(float p_fValue) const
			{
				Matrix3x3 result;

				for (int n=0; n<Order; n++)
					result.Elements[n] = Elements[n] * p_fValue;

				return result;
			}

			/* Multiplication operator with a scalar value */
			friend Matrix3x3 operator*(float p_fValue, const Matrix3x3& p_matrix)
			{
				Matrix3x3 result;

				for (int n=0; n<Order; n++)
					result.Elements[n] = p_matrix.Elements[n] * p_fValue;

				return result;
			}

			/* Division operator with a scalar value */
			Matrix3x3 operator/(float p_fValue) const
			{
				Matrix3x3 result;

				if (!Maths::Equals(p_fValue, 0.0f))
				{
					float fRcp = 1.0f / p_fValue;

					for (int n=0; n<Order; n++)
						result.Elements[n] = Elements[n] * fRcp;
				}
				else
				{
					for (int n=0; n<Order; n++)
						result.Elements[n] = Maths::Maximum;
				}

				return result;
			}

			Matrix3x3& operator+=(const Matrix3x3 &p_matrix)
			{
				for (int n=0; n<Order; n++)
					Elements[n] += p_matrix.Elements[n];

				return *this;
			}

			Matrix3x3& operator-=(const Matrix3x3 &p_matrix)
			{
				for (int n=0; n<Order; n++)
					Elements[n] -= p_matrix.Elements[n];

				return *this;
			}

			Matrix3x3& operator*=(float p_fValue)
			{
				for (int n=0; n<Order; n++)
					Elements[n] *= p_fValue;

				return *this;
			}

			Matrix3x3& operator/=(float p_fValue)
			{
				if (!Maths::Equals(p_fValue, 0.0f))
				{
					float fRcp = 1.0f / p_fValue;

					for (int n=0; n<Order; n++)
						Elements[n] *= fRcp;
				}
				else
				{
					for (int n=0; n<Order; n++)
						Elements[n] = Maths::Maximum;
				}

				return *this;
			}

			/* Product operation with a vector */
			Vector3 operator*(const Vector3& p_vector) const
			{
				return Vector3(_00 * p_vector.X + _01 * p_vector.Y + _02 * p_vector.Z,
							   _10 * p_vector.X + _11 * p_vector.Y + _12 * p_vector.Z,
							   _20 * p_vector.X + _21 * p_vector.Y + _22 * p_vector.Z);
			}

			/* Product operation with a matrix */
			Matrix3x3 operator*(const Matrix3x3& p_matrix) const
			{
				Matrix3x3 result;

				result._00 = _00 * p_matrix._00 + _01 * p_matrix._10 + _02 * p_matrix._20;
				result._01 = _00 * p_matrix._01 + _01 * p_matrix._11 + _02 * p_matrix._21;
				result._02 = _00 * p_matrix._02 + _01 * p_matrix._12 + _02 * p_matrix._22;

				result._10 = _10 * p_matrix._00 + _11 * p_matrix._10 + _12 * p_matrix._20;
				result._11 = _10 * p_matrix._01 + _11 * p_matrix._11 + _12 * p_matrix._21;
				result._12 = _10 * p_matrix._02 + _11 * p_matrix._12 + _12 * p_matrix._22;

				result._20 = _20 * p_matrix._00 + _21 * p_matrix._10 + _22 * p_matrix._20;
				result._21 = _20 * p_matrix._01 + _21 * p_matrix._11 + _22 * p_matrix._21;
				result._22 = _20 * p_matrix._02 + _21 * p_matrix._12 + _22 * p_matrix._22;

				return result;
			}

			/* Unary negation operator */
			Matrix3x3 operator-(void) const
			{
				Matrix3x3 negateThis;
				Matrix3x3::Negate(*this, negateThis);
				return negateThis;
			}

			/* Assignment operator */
			Matrix3x3& operator=(const Matrix3x3& p_matrix)
			{
				memcpy(Elements, p_matrix.Elements, sizeof(float) * Order);
				return *this;
			}

			/* Equality operator */
			bool operator==(const Matrix3x3& p_matrix) const
			{
				for (int n = 0; n < Order; n++)
					if (!Maths::Equals(Elements[n], p_matrix.Elements[n]))
						return false;

				return true;
			}

			/* Inequality operator */
			bool operator!=(const Matrix3x3& p_matrix) const
			{
				for (int n = 0; n < Order; n++)
					if (!Maths::Equals(Elements[n], p_matrix.Elements[n]))
						return true;

				return false;
			}

			/* Cast operator to a float* */
			operator float*() const {
				return (float*)&Elements;
			}

			/* Factory method which creates a rotation matrix about an arbitrary axis */
			inline static Matrix3x3 CreateRotation(const Vector3& p_axis, float p_fAngle) {
				return Matrix3x3(p_axis, p_fAngle);
			}

			/* Factory method which creates a scaling matrix */
			inline static Matrix3x3 CreateScaling(const Vector3& p_scale)
			{
				Matrix3x3 result;
				result.MakeDiagonal(p_scale);
				return result;
			}

			/* Addition of two matrices */
			inline static void Add(const Matrix3x3 &p_matrix1, const Matrix3x3 &p_matrix2, Matrix3x3 &p_out)
			{
				#if defined(SSE_ENABLED)
					__m128 v1, v2, v3;

					v1 = _mm_load_ps(&p_matrix1.Elements[0]);
					v2 = _mm_load_ps(&p_matrix2.Elements[0]);
					v3 = _mm_add_ps(v1, v2);
					_mm_store_ps(&p_out.Elements[0], v3);

					v1 = _mm_load_ps(&p_matrix1.Elements[4]);
					v2 = _mm_load_ps(&p_matrix2.Elements[4]);
					v3 = _mm_add_ps(v1, v2);
					_mm_store_ps(&p_out.Elements[4], v3);

					p_out._22 = p_matrix1._22 + p_matrix2._22;
				#else
					for (int j = 0; j < Order; j++)
						p_out.Elements[j] = p_matrix1.Elements[j] + p_matrix2.Elements[j];
				#endif
			}

			/* Subtraction between two matrices */
			inline static void Subtract(const Matrix3x3 &p_matrix1, const Matrix3x3 &p_matrix2, Matrix3x3 &p_out)
			{
				#if defined(SSE_ENABLED)
					__m128 v1, v2, v3;

					v1 = _mm_load_ps(&p_matrix1.Elements[0]);
					v2 = _mm_load_ps(&p_matrix2.Elements[0]);
					v3 = _mm_sub_ps(v1, v2);
					_mm_store_ps(&p_out.Elements[0], v3);

					v1 = _mm_load_ps(&p_matrix1.Elements[4]);
					v2 = _mm_load_ps(&p_matrix2.Elements[4]);
					v3 = _mm_sub_ps(v1, v2);
					_mm_store_ps(&p_out.Elements[4], v3);

					p_out._22 = p_matrix1._22 - p_matrix2._22;
				#else
					for (int j = 0; j < Order; j++)
						p_out.Elements[j] = p_matrix1.Elements[j] - p_matrix2.Elements[j];
				#endif
			}

			/* Product of two matrices */
			inline static void Product(const Matrix3x3 &p_matrix1, const Matrix3x3 &p_matrix2, Matrix3x3 &p_out)
			{
				p_out._00 = p_matrix1._00 * p_matrix2._00 + p_matrix1._01 * p_matrix2._10 + p_matrix1._02 * p_matrix2._20;
				p_out._01 = p_matrix1._00 * p_matrix2._01 + p_matrix1._01 * p_matrix2._11 + p_matrix1._02 * p_matrix2._21;
				p_out._02 = p_matrix1._00 * p_matrix2._02 + p_matrix1._01 * p_matrix2._12 + p_matrix1._02 * p_matrix2._22;

				p_out._10 = p_matrix1._10 * p_matrix2._00 + p_matrix1._11 * p_matrix2._10 + p_matrix1._12 * p_matrix2._20;
				p_out._11 = p_matrix1._10 * p_matrix2._01 + p_matrix1._11 * p_matrix2._11 + p_matrix1._12 * p_matrix2._21;
				p_out._12 = p_matrix1._10 * p_matrix2._02 + p_matrix1._11 * p_matrix2._12 + p_matrix1._12 * p_matrix2._22;

				p_out._20 = p_matrix1._20 * p_matrix2._00 + p_matrix1._21 * p_matrix2._10 + p_matrix1._22 * p_matrix2._20;
				p_out._21 = p_matrix1._20 * p_matrix2._01 + p_matrix1._21 * p_matrix2._11 + p_matrix1._22 * p_matrix2._21;
				p_out._22 = p_matrix1._20 * p_matrix2._02 + p_matrix1._21 * p_matrix2._12 + p_matrix1._22 * p_matrix2._22;
			}

			/* Product of a matrix and a vector */
			inline static void Product(const Matrix3x3 &p_matrix, const Vector3 &p_vector, Vector3 &p_out)
			{
				p_out.Set(p_matrix._00 * p_vector.X + p_matrix._01 * p_vector.Y + p_matrix._02 * p_vector.Z,
						  p_matrix._10 * p_vector.X + p_matrix._11 * p_vector.Y + p_matrix._12 * p_vector.Z,
						  p_matrix._20 * p_vector.X + p_matrix._21 * p_vector.Y + p_matrix._22 * p_vector.Z);
			}

			/* Transposition of two matrices */
			inline static void Transpose(const Matrix3x3 &p_matrix, Matrix3x3 &p_out)
			{
				p_out._00 = p_matrix._00;
				p_out._10 = p_matrix._01;
				p_out._20 = p_matrix._02;

				p_out._01 = p_matrix._10;
				p_out._11 = p_matrix._11;
				p_out._21 = p_matrix._12;

				p_out._02 = p_matrix._20;
				p_out._12 = p_matrix._21;
				p_out._22 = p_matrix._22;
			}

			/* Negation */
			inline static void Negate(const Matrix3x3 &p_matrix, Matrix3x3 &p_out)
			{
				p_out._00 = -p_matrix._00;
				p_out._10 = -p_matrix._10;
				p_out._20 = -p_matrix._20;

				p_out._01 = -p_matrix._01;
				p_out._11 = -p_matrix._11;
				p_out._21 = -p_matrix._21;

				p_out._02 = -p_matrix._02;
				p_out._12 = -p_matrix._12;
				p_out._22 = -p_matrix._22;
			}

			/* Inverse operation */
			inline static void Invert(const Matrix3x3 &p_matrix, Matrix3x3 &p_out)
			{
				float fDeterminant = p_matrix.Determinant();

				// Find inverse of determinant
				fDeterminant = 1.0f / fDeterminant;

				// Find inverse of each cell
				p_out._00 =  (p_matrix._11 * p_matrix._22 - p_matrix._12 * p_matrix._21) * fDeterminant;
				p_out._01 = -(p_matrix._01 * p_matrix._22 - p_matrix._21 * p_matrix._02) * fDeterminant;
				p_out._02 =  (p_matrix._01 * p_matrix._12 - p_matrix._11 * p_matrix._02) * fDeterminant;
				p_out._10 = -(p_matrix._10 * p_matrix._22 - p_matrix._12 * p_matrix._20) * fDeterminant;
				p_out._11 =  (p_matrix._00 * p_matrix._22 - p_matrix._20 * p_matrix._02) * fDeterminant;
				p_out._12 = -(p_matrix._00 * p_matrix._12 - p_matrix._10 * p_matrix._02) * fDeterminant;
				p_out._20 =  (p_matrix._10 * p_matrix._21 - p_matrix._10 * p_matrix._11) * fDeterminant;
				p_out._21 = -(p_matrix._00 * p_matrix._21 - p_matrix._20 * p_matrix._01) * fDeterminant;
				p_out._22 =  (p_matrix._00 * p_matrix._11 - p_matrix._01 * p_matrix._10) * fDeterminant;
			}

			/* Returns a string representation of matrix instance */
			std::string ToString(void)
			{
				boost::format formatter;

				std::string strOut =
					boost::str(boost::format("[%d %d %d | %d %d %d | %d %d %d]") % _00 % _01 % _02 % _10 % _11 % _12 % _20 % _21 % _22);
				return strOut;
			}

		private:
			inline int Index(int p_nRowIndex, int p_nColumnIndex) const
			{
				BOOST_ASSERT( p_nRowIndex >= 0 && p_nRowIndex < Rows && p_nColumnIndex >= 0 && p_nColumnIndex < Columns);
				return p_nRowIndex * Columns + p_nColumnIndex;
			}
		};
	}
}