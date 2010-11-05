//----------------------------------------------------------------------------------------------
//	Filename:	Matrix4x4.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Implementation for a 4x4 Matrix
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>

#include "System/Platform.h"
#include "Geometry/Matrix3x3.h"

namespace Illumina 
{
	namespace Core
	{
		class Matrix4x4
		{
		public:
			/* Matrix property constants */
			static const int Rows	 = 4;
			static const int Columns = 4;
			static const int Order	 = Rows * Columns;

			/* Common matrix configurations */
			static const Matrix4x4 Identity;
			static const Matrix4x4 Zero;

			/* Matrix cells */
			ALIGN_16 union
			{
				float Elements[Order];
				struct 
				{
					float _00, _01, _02, _03,
						  _10, _11, _12, _13,
						  _20, _21, _22, _23,
						  _30, _31, _32, _33;
				};
			};

		public:
			/* Default Matrix constructor */
			Matrix4x4(void) {}
	
			/* Constructor which initialises structure to identity or zero matrix */
			explicit Matrix4x4(bool p_bIdentity) {
				p_bIdentity ? MakeIdentity() : MakeZero();
			}

			/* Initialises matrix from float array */
			explicit Matrix4x4(const float p_fElements[Order]) {
				memcpy( Elements, p_fElements, sizeof(float) * Order );
			}

			/* Constructor for copying a 3x3 matrix */
			explicit Matrix4x4(const Matrix3x3 &p_matrix) {
				*this = p_matrix;
			}

			/* Copy constructor */
			Matrix4x4(const Matrix4x4 &p_matrix) {
				memcpy( Elements, p_matrix.Elements, sizeof(float) * Order );
			}

			/* Constructs a rotation matrix about an arbitrary angle */
			explicit Matrix4x4(const Vector3 &p_axis, float p_fAngle) {
				MakeRotation( p_axis, p_fAngle );
			}

			/* Tests whether matrix is singular (no inverse) */
			inline bool IsSingular(void) const {
				return Maths::Equals(Determinant(), 0.0f);
			}

			/* Tests whether matrix is identity */
			inline bool IsIdentity(void) const
			{
				return ( Maths::Equals(_00, 1.0f) &&
						 Maths::Equals(_11, 1.0f) &&
						 Maths::Equals(_22, 1.0f) &&
						 Maths::Equals(_33, 1.0f) &&
						 Maths::Equals(_01, 0.0f) &&
						 Maths::Equals(_02, 0.0f) &&
						 Maths::Equals(_03, 0.0f) &&
						 Maths::Equals(_10, 0.0f) &&
						 Maths::Equals(_12, 0.0f) &&
						 Maths::Equals(_13, 0.0f) &&
						 Maths::Equals(_20, 0.0f) &&
						 Maths::Equals(_21, 0.0f) &&
						 Maths::Equals(_23, 0.0f) &&
						 Maths::Equals(_30, 0.0f) &&
						 Maths::Equals(_31, 0.0f) &&
						 Maths::Equals(_32, 0.0f));
			}

			/* Tests whether structure is a zero matrix */
			inline bool IsZero(void) const
			{
				return (Maths::Equals(_00, 0.0f) &&
						Maths::Equals(_11, 0.0f) &&
						Maths::Equals(_22, 0.0f) &&
						Maths::Equals(_33, 0.0f) &&
						Maths::Equals(_01, 0.0f) &&
						Maths::Equals(_02, 0.0f) &&
						Maths::Equals(_03, 0.0f) &&
						Maths::Equals(_10, 0.0f) &&
						Maths::Equals(_12, 0.0f) &&
						Maths::Equals(_13, 0.0f) &&
						Maths::Equals(_20, 0.0f) &&
						Maths::Equals(_21, 0.0f) &&
						Maths::Equals(_23, 0.0f) &&
						Maths::Equals(_30, 0.0f) &&
						Maths::Equals(_31, 0.0f) &&
						Maths::Equals(_32, 0.0f));
			}

			/* Clears matrix cells setting it to a zero matrix */
			inline void MakeZero(void) {
				_00 = _01 = _02 = _03 = _10 = _11 = _12 = _13 = _20 = _21 = _22 = _23 = _30 = _31 = _32 = _33 = 0.0f;
			}

			/* Turns matrix into an identity matrix */
			void MakeIdentity(void)
			{
				_01 = _02 = _03 = _10 = _12 = _13 = _20 = _21 = _23 = _30 = _31 = _32 = 0.0f;
				_00 = _11 = _22 = _33 = 1.0f;
			}

			/* Turns matrix into a translation matrix with diagonals = 1.0f */
			void MakeTranslation(const Vector3 &p_translation) 
			{
				_01 = _02 = _10 = _12 = _20 = _21 = _30 = _31 = _32 = 0.0f;
				_00 = _11 = _22 = _33 = 1.0f;

				_03 = p_translation.X; _13 = p_translation.Y; _23 = p_translation.Z;
			}

			/* Turns matrix into a scaling matrix */
			void MakeScaling(const Vector3 &p_scale)
			{
				_01 = _02 = _03 = _10 = _12 = _13 = _20 = _21 = _23 = _30 = _31 = _32 = 0.0f;
				_33 = 1.0f;

				_00 = p_scale.X; _11 = p_scale.Y; _22 = p_scale.Z;
			}

			/* Turns matrix into a rotation matrix about an arbitrary angle */
			void MakeRotation(const Vector3 &p_axis, float p_fAngle)
			{
				SetRotation(p_axis, p_fAngle);
				
				_03 = _13 = _23 = _30 = _31 = _32 = 0.0f; 
				_33 = 1.0f;
			}

			/* Extracts a rotation axis and angle from matrix */
			void ExtractRotation(Vector3 &p_outAxis, float &p_outAngle) const
			{
				// Graphics Gems I, Converting between Matrix and Axis-Amount representations. pg 466
				float tCosTheta = 0.5f * (_00 + _11 + _22 - 1.0f);
				float tSinTheta = Maths::Sqrt(1.0f - tCosTheta * tCosTheta);

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

			/* Extracts translation components from matrix */
			void ExtractTranslation(Vector3 &p_outTranslation) const
			{
				p_outTranslation.X = _03;
				p_outTranslation.Y = _13;
				p_outTranslation.Z = _23;
			}

			/* Extracts scaling components from matrix */
			void ExtractScaling(Vector3 &p_outScale) const
			{
				p_outScale.X = _00;
				p_outScale.Y = _11;
				p_outScale.Z = _22;
			}			

			/* Set translation components without modifying other cells */
			void SetTranslation(const Vector3 &p_translation) {
				_03 = p_translation.X; _13 = p_translation.Y; _23 = p_translation.Z;
			}

			/* Set scaling components without modifying other cells */
			void SetScaling(const Vector3 &p_scale) {
				_00 = p_scale.X; _11 = p_scale.Y; _22 = p_scale.Z;
			}

			/* Set rotation (inner 3x3 sub-matrix) about an arbitrary axis */
			void SetRotation(const Vector3 &p_axis, float p_fAngle)
			{
				float tCos = Maths::Cos( p_fAngle ),
					  tSin = Maths::Sin( p_fAngle ),
					  tOneMinusCos = 1.0f - tCos;
				
				float tXY = p_axis.X * p_axis.Y * tOneMinusCos,
					  tXZ = p_axis.X * p_axis.Z * tOneMinusCos,
					  tYZ = p_axis.Y * p_axis.Z * tOneMinusCos,
					  tSinX = p_axis.X * tSin,
					  tSinY = p_axis.Y * tSin,
					  tSinZ = p_axis.Z * tSin;

				_00 = p_axis.X * p_axis.X * tOneMinusCos + tCos;
				_01 = tXY - tSinZ;
				_02 = tXZ + tSinY;

				_10 = tXY + tSinZ;
				_11 = p_axis.Y * p_axis.Y * tOneMinusCos + tCos;
				_12 = tYZ - tSinX;

				_20 = tXZ - tSinY;
				_21 = tYZ + tSinX;
				_22 = p_axis.Z * p_axis.Z * tOneMinusCos + tCos;
			}
	
			/* Returns matrix determinant */
			inline float Determinant(void) const
			{
				//     _00 * ( _11*(_22*_33-_23*_32) - _12*(_21*_33-_23*_31) + _13*(_21*_32-_22*_31)) -
				//	   _01 * ( _10*(_22*_33-_23*_32) - _12*(_20*_33-_23*_30) + _13*(_20*_32-_22*_30)) +
				//	   _02 * ( _10*(_21*_33-_23*_31) - _11*(_20*_33-_23*_30) + _13*(_20*_31-_21*_30)) -
				//	   _03 * ( _10*(_21*_32-_22*_31) - _11*(_20*_32-_22*_30) + _12*(_20*_31-_21*_30));

				// Cofactors
				float _22x44_23x43 = _22*_33 - _23*_32,
					  _21x44_23x42 = _21*_33 - _23*_31,
					  _21x43_22x42 = _21*_32 - _22*_31,
					  _20x44_23x41 = _20*_33 - _23*_30,
					  _20x43_22x41 = _20*_32 - _22*_30,
					  _20x42_21x41 = _20*_31 - _21*_30;

				return _00 * ( _11*(_22x44_23x43) - _12*(_21x44_23x42) + _13*(_21x43_22x42) ) -
					   _01 * ( _10*(_22x44_23x43) - _12*(_20x44_23x41) + _13*(_20x43_22x41) ) +
					   _02 * ( _10*(_21x44_23x42) - _11*(_20x44_23x41) + _13*(_20x42_21x41) ) -
					   _03 * ( _10*(_21x43_22x42) - _11*(_20x43_22x41) + _12*(_20x42_21x41) );
			}
	

			/* Computes matrix conjugate */
			void Negate(void) {
				Matrix4x4::Negate(*this, *this);
			}

			/* Compute matrix transpose */
			void Transpose(void)
			{
				std::swap(_10, _01);
				std::swap(_20, _02);
				std::swap(_30, _03);
				std::swap(_21, _12);
				std::swap(_31, _13);
				std::swap(_32, _23);
			}

			/* Compute matrix inverse */
			void Invert(void) 
			{
				// Macro returning determinant of 3x3 matrix
				#define DETERMINANT3X3(a, b, c, d, e, f, g, h, i) (a*(e*i-f*h) - b*(d*i-f*g) + c*(d*h-e*g))

				float mat[16], tInvDeterminant = 1.0f / Determinant();

				// b(i,j) = pow(-1, i+j) * det(A(j,i))/det(A)
				mat[0]  =  DETERMINANT3X3(_11,_12,_13,_21,_22,_23,_31,_32,_33) * tInvDeterminant;
				mat[1]  = -DETERMINANT3X3(_01,_02,_03,_21,_22,_23,_31,_32,_33) * tInvDeterminant;
				mat[2]  =  DETERMINANT3X3(_01,_02,_03,_11,_12,_13,_31,_32,_33) * tInvDeterminant;
				mat[3]  = -DETERMINANT3X3(_01,_02,_03,_11,_12,_13,_21,_22,_23) * tInvDeterminant;

				mat[4]  = -DETERMINANT3X3(_10,_12,_13,_20,_22,_23,_30,_32,_33) * tInvDeterminant;
				mat[5]  =  DETERMINANT3X3(_00,_02,_03,_20,_22,_23,_30,_32,_33) * tInvDeterminant;
				mat[6]  = -DETERMINANT3X3(_00,_02,_03,_10,_12,_13,_30,_32,_33) * tInvDeterminant;
				mat[7]  =  DETERMINANT3X3(_00,_02,_03,_10,_12,_13,_20,_22,_23) * tInvDeterminant;

				mat[8]  =  DETERMINANT3X3(_10,_11,_13,_20,_21,_23,_30,_31,_33) * tInvDeterminant;
				mat[9]  = -DETERMINANT3X3(_00,_01,_03,_20,_21,_23,_30,_31,_33) * tInvDeterminant;
				mat[10] =  DETERMINANT3X3(_00,_01,_03,_10,_11,_13,_30,_31,_33) * tInvDeterminant;
				mat[11] = -DETERMINANT3X3(_00,_01,_03,_10,_11,_13,_20,_21,_23) * tInvDeterminant;

				mat[12] = -DETERMINANT3X3(_10,_11,_12,_20,_21,_22,_30,_31,_32) * tInvDeterminant;
				mat[13] =  DETERMINANT3X3(_00,_01,_02,_20,_21,_22,_30,_31,_32) * tInvDeterminant;
				mat[14] = -DETERMINANT3X3(_00,_01,_02,_10,_11,_12,_30,_31,_32) * tInvDeterminant;
				mat[15] =  DETERMINANT3X3(_00,_01,_02,_10,_11,_12,_20,_21,_22) * tInvDeterminant;

				memcpy( Elements, mat, Order * sizeof(float) );

				// Undefine determinant macro
				#undef DETERMINANT3X3
			}

			/* Overloads functor operator to return element at (row, column) */
			float operator()(int p_nRow, int p_nColumn) const {
				return Elements[ Index(p_nRow, p_nColumn) ];
			}

			/* Overloads functor operator to return element reference at (row, column) */
			float& operator()(int p_nRow, int p_nColumn) {
				return Elements[ Index(p_nRow, p_nColumn) ];
			}

			/* Overloads the addition operator */
			Matrix4x4 operator+(const Matrix4x4 &p_matrix) const
			{
				Matrix4x4 result;
				Matrix4x4::Add(*this, p_matrix, result);
				return result;
			}

			/* Overloads the subtraction operator */
			Matrix4x4 operator-(const Matrix4x4 &p_matrix) const
			{
				Matrix4x4 result;
				Matrix4x4::Subtract(*this, p_matrix, result);
				return result;
			}

			/* Multiplication operator with a scalar value */
			Matrix4x4 operator*(float p_fValue) const
			{
				Matrix4x4 result;

				for (int n=0; n<Order; n++)
					result.Elements[n] = Elements[n] * p_fValue;

				return result;
			}

			/* Multiplication operator with a scalar value */
			friend Matrix4x4 operator*(float p_fValue, const Matrix4x4 &p_matrix)
			{
				Matrix4x4 result;

				for (int n=0; n<Order; n++)
					result.Elements[n] = p_matrix.Elements[n] * p_fValue;

				return result;
			}

			/* Division operator with a scalar value */
			Matrix4x4 operator/(float p_fValue) const
			{
				Matrix4x4 result;

				if ( !Maths::Equals(p_fValue, 0.0f) )
				{
					float fRcp = 1.0f / p_fValue;

					for (int n = 0; n < Order; n++)
						result.Elements[n] = Elements[n] * fRcp;
				}
				else
				{
					for (int n = 0; n < Order; n++)
						result.Elements[n] = Maths::Maximum;
				}

				return result;
			}

			Matrix4x4& operator+=(const Matrix4x4 &p_matrix)
			{
				for (int n=0; n<Order; n++)
					Elements[n] += p_matrix.Elements[n];

				return *this;
			}

			Matrix4x4& operator-=(const Matrix4x4 &p_matrix)
			{
				for (int n=0; n<Order; n++)
					Elements[n] -= p_matrix.Elements[n];

				return *this;
			}

			Matrix4x4& operator*=(float p_fValue)
			{
				for (int n=0; n<Order; n++)
					Elements[n] *= p_fValue;

				return *this;
			}

			Matrix4x4& operator/=(float p_fValue)
			{
				if ( !Maths::Equals(p_fValue, 0.0f) )
				{
					float tInvValue = 1.0f / p_fValue;

					for (int n=0; n<Order; n++)
						Elements[n] *= tInvValue;
				}
				else
				{
					for (int n=0; n<Order; n++)
						Elements[n] = Maths::Maximum;
				}

				return *this;
			}
	
			/* Product operation with a vector */
			Vector3 operator*(const Vector3 &p_vector) const
			{
				return Vector3( _00 * p_vector.X + _01 * p_vector.Y + _02 * p_vector.Z + _03,
								_10 * p_vector.X + _11 * p_vector.Y + _12 * p_vector.Z + _13,
								_20 * p_vector.X + _21 * p_vector.Y + _22 * p_vector.Z + _23);
			}

			/* Product operation with a matrix */
			Matrix4x4 operator*(const Matrix4x4 &p_matrix) const
			{
				Matrix4x4 result;
				Matrix4x4::Product(*this, p_matrix, result);
				return result;
			}
			
			/* Unary negation operator */
			Matrix4x4 operator-(void) const 
			{
				Matrix4x4 result;
				Matrix4x4::Negate(*this, result);
				return result;
			}

			/* Assignment operator */
			Matrix4x4& operator=(const Matrix3x3 &p_matrix)
			{
				_00 = p_matrix._00;
				_01 = p_matrix._01;
				_02 = p_matrix._02;
				_10 = p_matrix._10;
				_11 = p_matrix._11;
				_12 = p_matrix._12;
				_20 = p_matrix._20;
				_21 = p_matrix._21;
				_22 = p_matrix._22;
				
				_03 = _13 = _23 = _30 = _31 = _32 = 0.0f;
				_33 = 1.0f;

				return *this;
			}

			/* Assignment operator */
			Matrix4x4& operator=(const Matrix4x4 &p_matrix)
			{
				memcpy(Elements, p_matrix.Elements, sizeof(float) * Order);
				return *this;
			}

			/* Equality operator */
			bool operator==(const Matrix4x4 &p_matrix) const
			{
				for (int nIdx = 0; nIdx < Order; nIdx++)
					if ( !Maths::Equals( Elements[nIdx], p_matrix.Elements[nIdx] ) )
						return false;

				return true;
			}
	
			/* Inequality operator */
			bool operator!=(const Matrix4x4 &p_matrix) const
			{
				for (int nIdx = 0; nIdx < Order; nIdx++)
					if ( !Maths::Equals( Elements[nIdx], p_matrix.Elements[nIdx] ) )
						return true;

				return false;
			}

			/* Cast operator to a float* */
			operator float*() const {
				return (float*)&Elements;
			}
			
			/* Factory method which creates a rotation matrix about an arbitrary axis */
			inline static Matrix4x4 CreateRotation(const Vector3 &p_axis, float p_fAngle) {
				return Matrix4x4(p_axis, p_fAngle);
			}

			/* Factory method which creates a scaling matrix */
			inline static Matrix4x4 CreateScaling(const Vector3 &p_scale)
			{
				Matrix4x4 result;
				result.MakeScaling(p_scale);
				return result;
			}

			/* Factory method which creates a translation matrix */
			inline static Matrix4x4 CreateTranslation(const Vector3 &p_translation) 
			{
				Matrix4x4 result;
				result.MakeTranslation(p_translation);
				return result;
			}

			/* Addition of two matrices */
			inline static void Add(const Matrix4x4 &p_matrix1, const Matrix4x4 &p_matrix2, Matrix4x4 &p_out)
			{
				#if defined(SSE_ENABLED)
					__m128 v1, v2, v3;

					for (int j = 0; j < Order; j+=4)
					{
						v1 = _mm_load_ps(&p_matrix1.Elements[j]);
						v2 = _mm_load_ps(&p_matrix2.Elements[j]);
						v3 = _mm_add_ps(v1, v2);
						_mm_store_ps(&p_out.Elements[j], v3);
					}
				#else
					for (int j = 0; j < Order; j++)
						p_out.Elements[j] = p_matrix1.Elements[j] + p_matrix2.Elements[j];
				#endif
			}

			/* Subtraction between two matrices */
			inline static void Subtract(const Matrix4x4 &p_matrix1, const Matrix4x4 &p_matrix2, Matrix4x4 &p_out)
			{
				#if defined(SSE_ENABLED)
					__m128 v1, v2, v3;

					for (int j = 0; j < Order; j+=4)
					{
						v1 = _mm_load_ps(&p_matrix1.Elements[j]);
						v2 = _mm_load_ps(&p_matrix2.Elements[j]);
						v3 = _mm_sub_ps(v1, v2);
						_mm_store_ps(&p_out.Elements[j], v3);
					}
				#else
					for (int j = 0; j < Order; j++)
						p_out.Elements[j] = p_matrix1.Elements[j] - p_matrix2.Elements[j];
				#endif
			}

			/* Produce of two matrices */
			inline static void Product(const Matrix4x4 &p_matrix1, const Matrix4x4 &p_matrix2, Matrix4x4 &p_out)
			{
				#if defined(SSE_ENABLED)
					__m128 A, B1, B2, B3, B4, T1, T2, R;

					B1 = _mm_load_ps(&p_matrix2.Elements[0]);
					B2 = _mm_load_ps(&p_matrix2.Elements[4]);
					B3 = _mm_load_ps(&p_matrix2.Elements[8]);
					B4 = _mm_load_ps(&p_matrix2.Elements[12]);

					_MM_TRANSPOSE4_PS(B1, B2, B3, B4);

					for (int j = 0; j < Order; j+=4)
					{
						A = _mm_load_ps(&p_matrix1.Elements[j]);
						
						T1 = _mm_mul_ps(A, B1);
						T2 = _mm_mul_ps(A, B2);

						R = _mm_hadd_ps(T1, T2);

						T1 = _mm_mul_ps(A, B3);
						T2 = _mm_mul_ps(A, B4);

						A = _mm_hadd_ps(T1, T2);

						T1 = _mm_hadd_ps(R, A);

						_mm_store_ps(&p_out.Elements[j], T1);
					}
				#else
					p_out._00 = p_matrix1._00 * p_matrix2._00 + p_matrix1._01 * p_matrix2._10 + p_matrix1._02 * p_matrix2._20 + p_matrix1._03 * p_matrix2._30;
					p_out._01 = p_matrix1._00 * p_matrix2._01 + p_matrix1._01 * p_matrix2._11 + p_matrix1._02 * p_matrix2._21 + p_matrix1._03 * p_matrix2._31;
					p_out._02 = p_matrix1._00 * p_matrix2._02 + p_matrix1._01 * p_matrix2._12 + p_matrix1._02 * p_matrix2._22 + p_matrix1._03 * p_matrix2._32;
					p_out._03 = p_matrix1._00 * p_matrix2._03 + p_matrix1._01 * p_matrix2._13 + p_matrix1._02 * p_matrix2._23 + p_matrix1._03 * p_matrix2._33;

					p_out._10 = p_matrix1._10 * p_matrix2._00 + p_matrix1._11 * p_matrix2._10 + p_matrix1._12 * p_matrix2._20 + p_matrix1._13 * p_matrix2._30;
					p_out._11 = p_matrix1._10 * p_matrix2._01 + p_matrix1._11 * p_matrix2._11 + p_matrix1._12 * p_matrix2._21 + p_matrix1._13 * p_matrix2._31;
					p_out._12 = p_matrix1._10 * p_matrix2._02 + p_matrix1._11 * p_matrix2._12 + p_matrix1._12 * p_matrix2._22 + p_matrix1._13 * p_matrix2._32;
					p_out._13 = p_matrix1._10 * p_matrix2._03 + p_matrix1._11 * p_matrix2._13 + p_matrix1._12 * p_matrix2._23 + p_matrix1._13 * p_matrix2._33;

					p_out._20 = p_matrix1._20 * p_matrix2._00 + p_matrix1._21 * p_matrix2._10 + p_matrix1._22 * p_matrix2._20 + p_matrix1._23 * p_matrix2._30;
					p_out._21 = p_matrix1._20 * p_matrix2._01 + p_matrix1._21 * p_matrix2._11 + p_matrix1._22 * p_matrix2._21 + p_matrix1._23 * p_matrix2._31;
					p_out._22 = p_matrix1._20 * p_matrix2._02 + p_matrix1._21 * p_matrix2._12 + p_matrix1._22 * p_matrix2._22 + p_matrix1._23 * p_matrix2._32;
					p_out._23 = p_matrix1._20 * p_matrix2._03 + p_matrix1._21 * p_matrix2._13 + p_matrix1._22 * p_matrix2._23 + p_matrix1._23 * p_matrix2._33;

					p_out._30 = p_matrix1._30 * p_matrix2._00 + p_matrix1._31 * p_matrix2._10 + p_matrix1._32 * p_matrix2._20 + p_matrix1._33 * p_matrix2._30;
					p_out._31 = p_matrix1._30 * p_matrix2._01 + p_matrix1._31 * p_matrix2._11 + p_matrix1._32 * p_matrix2._21 + p_matrix1._33 * p_matrix2._31;
					p_out._32 = p_matrix1._30 * p_matrix2._02 + p_matrix1._31 * p_matrix2._12 + p_matrix1._32 * p_matrix2._22 + p_matrix1._33 * p_matrix2._32;
					p_out._33 = p_matrix1._30 * p_matrix2._03 + p_matrix1._31 * p_matrix2._13 + p_matrix1._32 * p_matrix2._23 + p_matrix1._33 * p_matrix2._33;
				#endif
			}

			/* Transposition of two matrices */
			inline static void Transpose(const Matrix4x4 &p_matrix, Matrix4x4 &p_out)
			{
				#if defined(SSE_ENABLED)
					__m128 v1, v2, v3, v4;

					v1 = _mm_load_ps(&p_matrix.Elements[0]);
					v2 = _mm_load_ps(&p_matrix.Elements[4]);
					v3 = _mm_load_ps(&p_matrix.Elements[8]);
					v4 = _mm_load_ps(&p_matrix.Elements[12]);

					_MM_TRANSPOSE4_PS(v1, v2, v3, v4);

					_mm_store_ps(&p_out.Elements[0], v1);
					_mm_store_ps(&p_out.Elements[4], v2);
					_mm_store_ps(&p_out.Elements[8], v3);
					_mm_store_ps(&p_out.Elements[12], v4);
				#else
					p_out._00 = p_matrix._00; p_out._01 = p_matrix._10; p_out._02 = p_matrix._20; p_out._03 = p_matrix._30;
					p_out._10 = p_matrix._01; p_out._11 = p_matrix._11; p_out._12 = p_matrix._21; p_out._13 = p_matrix._31;
					p_out._20 = p_matrix._02; p_out._21 = p_matrix._12; p_out._22 = p_matrix._22; p_out._23 = p_matrix._32;
					p_out._30 = p_matrix._03; p_out._31 = p_matrix._13; p_out._32 = p_matrix._23; p_out._33 = p_matrix._33;
				#endif
			}
			
			/* Negation */
			inline static void Negate(const Matrix4x4 &p_matrix, Matrix4x4 &p_out) 
			{
				p_out._00 = -p_matrix._00;
				p_out._10 = -p_matrix._10;
				p_out._20 = -p_matrix._20;
				p_out._30 = -p_matrix._30;

				p_out._01 = -p_matrix._01;
				p_out._11 = -p_matrix._11;
				p_out._21 = -p_matrix._21;
				p_out._31 = -p_matrix._31;

				p_out._02 = -p_matrix._02;
				p_out._12 = -p_matrix._12;
				p_out._22 = -p_matrix._22;
				p_out._32 = -p_matrix._32;

				p_out._03 = -p_matrix._03;
				p_out._13 = -p_matrix._13;
				p_out._23 = -p_matrix._23;
				p_out._33 = -p_matrix._33;
			}

			/* Inverse operation */
			static void Invert(const Matrix4x4 &p_matrix, Matrix4x4 &p_out)
			{
				// Macro returning determinant of 3x3 matrix
				#define DETERMINANT3X3(a, b, c, d, e, f, g, h, i) (a*(e*i-f*h) - b*(d*i-f*g) + c*(d*h-e*g))

				float tInvDeterminant = 1.0f / p_matrix.Determinant();

				// b(i,j) = pow(-1, i+j) * det(A(j,i))/det(A)
				p_out._00 =  DETERMINANT3X3(p_matrix._11,p_matrix._12,p_matrix._13,p_matrix._21,p_matrix._22,p_matrix._23,p_matrix._31,p_matrix._32,p_matrix._33) * tInvDeterminant;
				p_out._01 = -DETERMINANT3X3(p_matrix._01,p_matrix._02,p_matrix._03,p_matrix._21,p_matrix._22,p_matrix._23,p_matrix._31,p_matrix._32,p_matrix._33) * tInvDeterminant;
				p_out._02 =  DETERMINANT3X3(p_matrix._01,p_matrix._02,p_matrix._03,p_matrix._11,p_matrix._12,p_matrix._13,p_matrix._31,p_matrix._32,p_matrix._33) * tInvDeterminant;
				p_out._03 = -DETERMINANT3X3(p_matrix._01,p_matrix._02,p_matrix._03,p_matrix._11,p_matrix._12,p_matrix._13,p_matrix._21,p_matrix._22,p_matrix._23) * tInvDeterminant;

				p_out._10 = -DETERMINANT3X3(p_matrix._10,p_matrix._12,p_matrix._13,p_matrix._20,p_matrix._22,p_matrix._23,p_matrix._30,p_matrix._32,p_matrix._33) * tInvDeterminant;
				p_out._11 =  DETERMINANT3X3(p_matrix._00,p_matrix._02,p_matrix._03,p_matrix._20,p_matrix._22,p_matrix._23,p_matrix._30,p_matrix._32,p_matrix._33) * tInvDeterminant;
				p_out._12 = -DETERMINANT3X3(p_matrix._00,p_matrix._02,p_matrix._03,p_matrix._10,p_matrix._12,p_matrix._13,p_matrix._30,p_matrix._32,p_matrix._33) * tInvDeterminant;
				p_out._13 =  DETERMINANT3X3(p_matrix._00,p_matrix._02,p_matrix._03,p_matrix._10,p_matrix._12,p_matrix._13,p_matrix._20,p_matrix._22,p_matrix._23) * tInvDeterminant;

				p_out._20 =  DETERMINANT3X3(p_matrix._10,p_matrix._11,p_matrix._13,p_matrix._20,p_matrix._21,p_matrix._23,p_matrix._30,p_matrix._31,p_matrix._33) * tInvDeterminant;
				p_out._21 = -DETERMINANT3X3(p_matrix._00,p_matrix._01,p_matrix._03,p_matrix._20,p_matrix._21,p_matrix._23,p_matrix._30,p_matrix._31,p_matrix._33) * tInvDeterminant;
				p_out._22 =  DETERMINANT3X3(p_matrix._00,p_matrix._01,p_matrix._03,p_matrix._10,p_matrix._11,p_matrix._13,p_matrix._30,p_matrix._31,p_matrix._33) * tInvDeterminant;
				p_out._23 = -DETERMINANT3X3(p_matrix._00,p_matrix._01,p_matrix._03,p_matrix._10,p_matrix._11,p_matrix._13,p_matrix._20,p_matrix._21,p_matrix._23) * tInvDeterminant;

				p_out._30 = -DETERMINANT3X3(p_matrix._10,p_matrix._11,p_matrix._12,p_matrix._20,p_matrix._21,p_matrix._22,p_matrix._30,p_matrix._31,p_matrix._32) * tInvDeterminant;
				p_out._31 =  DETERMINANT3X3(p_matrix._00,p_matrix._01,p_matrix._02,p_matrix._20,p_matrix._21,p_matrix._22,p_matrix._30,p_matrix._31,p_matrix._32) * tInvDeterminant;
				p_out._32 = -DETERMINANT3X3(p_matrix._00,p_matrix._01,p_matrix._02,p_matrix._10,p_matrix._11,p_matrix._12,p_matrix._30,p_matrix._31,p_matrix._32) * tInvDeterminant;
				p_out._33 =  DETERMINANT3X3(p_matrix._00,p_matrix._01,p_matrix._02,p_matrix._10,p_matrix._11,p_matrix._12,p_matrix._20,p_matrix._21,p_matrix._22) * tInvDeterminant;

				// Undefine determinant macro
				#undef DETERMINANT3X3
			}
			
			/* Returns a string representation of matrix instance */
			std::string ToString(void)
			{
				boost::format formatter;
				
				std::string strOut = 
					boost::str(boost::format("[%d %d %d %d | %d %d %d %d | %d %d %d %d | %d %d %d %d]") 
						% _00 % _01 % _02 % _03 
						% _10 % _11 % _12 % _13
						% _20 % _21 % _22 % _23
						% _30 % _31 % _32 % _33);

				return strOut;
			}

		private:
			inline int Index(int p_nRow, int p_nColumn) const
			{
				BOOST_ASSERT(p_nRow >= 0 && p_nRow < Rows && p_nColumn >= 0 && p_nColumn < Columns);
				return p_nRow * Columns + p_nColumn;
			}
		};
	}
}


