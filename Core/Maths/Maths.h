//----------------------------------------------------------------------------------------------
//	Filename:	Maths.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Hub for various mathematical functions in single precision floating point
//----------------------------------------------------------------------------------------------
#pragma once

#include <limits>
#include <cmath>

#include "System/IlluminaPRT.h"

namespace Illumina
{
	namespace Core
	{
		class Maths
		{
		public:
			template <class T>
			static T Min(const T &p_value1, const T &p_value2) {
				return (p_value1 < p_value2) ? p_value1 : p_value2;
			}

			template <class T>
			static T Max(const T &p_value1, const T& p_value2) {
				return (p_value1 > p_value2) ? p_value1 : p_value2;
			}

			template<class T>
			inline static T FAbs(const T& p_value) {
				return (p_value < 0) ? -p_value : p_value;
			}

			inline static float Frac(float p_fValue) {
				return fabs(p_fValue - floor(p_fValue));
			}

			inline static float Ceil(float p_fValue) {
				return ceil(p_fValue);
			}

			inline static float Floor(float p_fValue) {
				return floor(p_fValue);
			}

			inline static float Abs(float p_fValue) {
				return fabs(p_fValue);
			}

			inline static float FAbs(float p_fValue) {
				return (p_fValue < 0.0f) ? -p_fValue : p_fValue;
			}

			inline static int ISgn(float p_fValue)
			{
				if (p_fValue == 0.0f) return 0;
				return (p_fValue < 0.0f) ? -1: 1;
			}

			inline static float Pow(float p_fBase, float p_fExp) {
				return pow( p_fBase, p_fExp );
			}

			inline static float Sqr(float p_fValue) {
				return p_fValue * p_fValue;
			}

			inline static float Sqrt(const float p_fValue) 
			{
				return sqrt(p_fValue);
			}

			inline static float Sin(float p_fAngle) {
				return sin(p_fAngle);
			}

			inline static float Asin(float p_fValue) {
				return asin(p_fValue);
			}

			inline static float Cos(float p_fAngle) {
				return cos(p_fAngle);
			}

			inline static float Acos(float p_fValue) {
				return acos(p_fValue);
			}

			inline static float Tan(float p_fAngle) {
				return tan(p_fAngle);
			}

			inline static float Atan(float p_fValue) {
				return atan(p_fValue);
			}

			inline static float Atan(float p_fYValue, float p_fXValue) {
				return atan2(p_fYValue, p_fXValue);
			}

			static float Exp( float p_fValue ) {
				return exp(p_fValue);
			}

			static float Ln(float p_fValue) {
				return log(p_fValue);
			}

			static float Ld(float p_fValue) {
				return (float)(log(p_fValue) / log(2.0));
			}

			static float Log(float p_fValue) {
				return log10(p_fValue);
			}

			static float LogBase(float p_fValue, float p_fBase) {
				return log(p_fValue) / log(p_fBase);
			}

			inline static float RadToDeg(float p_fAngle) {
				return p_fAngle * _RadToDeg;
			}

			inline static float DegToRad(float p_fAngle) {
				return p_fAngle * _DegToRad;
			}

			/*
			inline static float Min(float p_fValue1, float p_fValue2) {
				return (p_fValue1 < p_fValue2) ? p_fValue1 : p_fValue2;
			}
			*/

			inline static float Clamp(float p_fValue, float p_fMin, float p_fMax )
			{
				if (p_fValue < p_fMin) return p_fMin;
				if (p_fValue > p_fMax) return p_fMax;
				return p_fValue;
			}

			/* 
			inline static float Max(float p_fValue1, float p_fValue2) {
				return (p_fValue1 > p_fValue2) ? p_fValue1 : p_fValue2;
			} 
			*/

			inline static bool Equals(float p_fValue1, float p_fValue2, float p_fTolerance = Epsilon) {
				return (Abs(p_fValue1 - p_fValue2) <= p_fTolerance);
			}

			inline static float Lerp(float u0, float u1, float x) {
				return u0 - Hermite(x) * (u0 - u1);
			}

			inline static float Hermite(float t) {
				return 3*t*t - 2*t*t*t;
			}

			inline static int Factorial(int p_nValue)
			{
				int nSum = 1;

				if (p_nValue > 0)
					for (int nTerm = 2; nTerm <= p_nValue; nSum *= nTerm, nTerm++);
		
				return nSum;
			}

			template<class type> inline static void Swap(type &p_fValue1, type &p_fValue2)
			{
				type temp = p_fValue1;
				p_fValue1 = p_fValue2;
				p_fValue2 = temp;
			}

			static const float E;
			static const float Pi;
			static const float PiTwo;
			static const float PiHalf;
			static const float InvPi;
			static const float InvPiTwo;
			static const float SqrtPi;
			static const float SqrtPiTwo;
			static const float InvSqrtPi;
			static const float InvSqrtPiTwo;
			static const float Epsilon;
			static const float Infinity;
			static const float Minimum;
			static const float Maximum;
			static const float Tolerance;

		private:
			static const float _DegToRad;
			static const float _RadToDeg;
		};
	}
}
