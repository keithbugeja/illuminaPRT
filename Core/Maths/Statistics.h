//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//	Author			Date		Version		Description
//----------------------------------------------------------------------------------------------
//	Keith Bugeja	01/07/2007	1.0.0		First version
//----------------------------------------------------------------------------------------------
#pragma once

// Core Includes
#include "Maths.h"
#include <boost/math/special_functions.hpp>

// namespace Meson.Common.Math
namespace Illumina
{
	namespace Core
	{
		class Statistics
		{
			public:
				//----------------------------------------------------------------------------------------------
				/// Calculates the arithmetic mean of the given distribution.
				///	\param TReal *p_ptDistribution Distribution array
				/// \param int p_nDistributionSize Size of distribution
				/// \returns Arithmetic mean
				//----------------------------------------------------------------------------------------------
				static float Mean(float *p_pfDistribution, int p_nDistributionSize)
				{
					BOOST_ASSERT(p_pfDistribution != NULL);

					// Distribution size is zero or less
					if (p_nDistributionSize <= 0) 
						return 0;

					float fSum = 0;

					for (int n = 0; n < p_nDistributionSize; n++)
						fSum += p_pfDistribution[n];

					fSum /= p_nDistributionSize;
					return fSum; 
				}

				//----------------------------------------------------------------------------------------------
				/// Calculates the variance of the given distribution.
				///	\param TReal *p_ptDistribution Distribution array
				/// \param int p_nDistributionSize Size of distribution
				/// \returns Variance
				//----------------------------------------------------------------------------------------------
				static float Variance(float *p_pfDistribution, int p_nDistributionSize)
				{
					BOOST_ASSERT(p_pfDistribution != NULL);

					// Distribution size is zero or less
					if ( p_nDistributionSize <= 0 ) 
						return 0;

					float fMean = Mean(p_pfDistribution, p_nDistributionSize),
						  fDeviation = 0;

					for (int n = 0; n < p_nDistributionSize; n++)
						fDeviation += p_pfDistribution[n] * p_pfDistribution[n];

					fDeviation -= fMean * fMean * p_nDistributionSize;
		
					return fDeviation / p_nDistributionSize;
				}

			//----------------------------------------------------------------------------------------------
			/// Calculates the standard deviation of the given distribution.
			///	\param TReal *p_ptDistribution Distribution array
			/// \param int p_nDistributionSize Size of distribution
			/// \returns Standard deviation
			//----------------------------------------------------------------------------------------------
			static float StandardDeviation(float *p_pfDistribution, int p_nDistributionSize) 
			{
				BOOST_ASSERT(p_pfDistribution != NULL);

				// Distribution size is zero or less
				if (p_nDistributionSize <= 0) 
					return 0;

				float fMean = Mean(p_pfDistribution, p_nDistributionSize),
					  fDeviation = 0;

				for (int n = 0; n < p_nDistributionSize; n++)
					fDeviation += p_pfDistribution[n] * p_pfDistribution[n];

				fDeviation -= fMean * fMean * p_nDistributionSize;
		
				return Maths::Sqrt(fDeviation / p_nDistributionSize);
			}

			//----------------------------------------------------------------------------------------------
			/// Calculates the permutations, for the given number of elements, taken N at a time.
			/// \param p_nElements Number of elements
			/// \param p_nSelection Number of elements to take at a time
			/// \returns Permutation count
			//----------------------------------------------------------------------------------------------
			static int Permutations(int p_nElements, int p_nSelection)
			{
				// P( n, r ) = n! / (n-r)!
				BOOST_ASSERT(p_nSelection <= p_nElements);
				BOOST_ASSERT(p_nElements > 0 && p_nSelection > 0);
		
				// TODO: Optimise factorials
				int nNFct = Maths::Factorial(p_nElements),
					nNLessRFct = Maths::Factorial(p_nElements - p_nSelection);

				return (nNFct / nNLessRFct);
			}
	
			//----------------------------------------------------------------------------------------------
			/// Calculates the combinations, for the given number of elements, taken N at a time.
			/// \param p_nElements Number of elements
			/// \param p_nSelection Number of elements to take at a time
			/// \returns Combination count
			//----------------------------------------------------------------------------------------------
			static int Combinations(int p_nElements, int p_nSelection)
			{
				// C(n, r) = n! / ((n-r)!r!)
				BOOST_ASSERT(p_nSelection <= p_nElements);
				BOOST_ASSERT(p_nElements > 0 && p_nSelection > 0);
		
				// TODO: Optimise factorials
				int nNFct = Maths::Factorial(p_nElements),
					nNLessRFct = Maths::Factorial(p_nElements - p_nSelection),
					nRFct = Maths::Factorial(p_nSelection);

				return (nNFct / (nNLessRFct * nRFct));
			}

			//----------------------------------------------------------------------------------------------
			//----------------------------------------------------------------------------------------------
			static float InverseErf(float p_fX)
			{
				// Inverse Error Function, source: http://functions.wolfram.com/GammaBetaErf/InverseErf/06/01/
				//
				//					   Ck	     Sqrt(Pi)
				// erf^-1(x) = SUM( -------- * ( -------- * x) ^ 2k+1, k=0 to Inf )
				//                  (2k + 1)         2
				//					
				// C0 = 1
				//			 Cm * Ck - 1 - m 
				// Ck = SUM( ---------------, m=0 to k-1)
				//			 (m + 1)(2m + 1)
				//

				// Using Peter J. Acklam's algorithm:
				// http://home.online.no/~pjacklam/notes/invnorm/

				static const long double A1 = -3.969683028665376e+01,
										 A2 =  2.209460984245205e+02,
										 A3 = -2.759285104469687e+02,
										 A4 =  1.383577518672690e+02,
										 A5 = -3.066479806614716e+01,
										 A6 =  2.506628277459239e+00;

				static const long double B1 = -5.447609879822406e+01,
										 B2 =  1.615858368580409e+02,
										 B3 = -1.556989798598866e+02,
										 B4 =  6.680131188771972e+01,
										 B5 = -1.328068155288572e+01;

				static const long double C1 = -7.784894002430293e-03,
										 C2 = -3.223964580411365e-01,
										 C3 = -2.400758277161838e+00,
										 C4 = -2.549732539343734e+00,
										 C5 =  4.374664141464968e+00,
										 C6 =  2.938163982698783e+00;

				static const long double D1 =  7.784695709041462e-03,
										 D2 =  3.224671290700398e-01,
										 D3 =  2.445134137142996e+00,
										 D4 =  3.754408661907416e+00;

				static const long double ldfXLow  = 0.02425,
										 ldfXHigh = 0.97575;

				long double q, r;

				// 0 < X < Low
				if (p_fX > 0 && p_fX < ldfXLow)
				{
					q = Maths::Sqrt(-2 * Maths::Ln(p_fX));
					return (float)(((((C1 * q + C2) * q + C3) * q + C4) * q + C5) * q + C6) / ((((D1 * q + D2) * q + D3) * q + D4) * q + 1);
				}
		
				// Low <= X <= High
				if (p_fX >= ldfXLow && p_fX <= ldfXHigh)
				{
					q = p_fX - 0.5;
					r = q * q;
					return (float)(((((A1 * r + A2) * r + A3) * r + A4) * r + A5) * r + A6) * q /(((((B1 * r + B2) * r + B3) * r + B4) * r + B5) * r + 1);
				}
		
				// High < X < 1
				if (p_fX > ldfXHigh && p_fX < 1)
				{
					q = Maths::Sqrt(-2 * Maths::Ln(1 - p_fX));
					return (float) -(((((C1 * q + C2) * q + C3) * q + C4) * q + C5) * q + C6) / ((((D1 * q + D2) * q + D3) * q + D4) * q + 1);
				} 
		
				// 0 <= X
				if (p_fX < Maths::Epsilon)
					return Maths::Minimum;

				// 1 <= X
				return Maths::Maximum;
			}

			//----------------------------------------------------------------------------------------------
			//----------------------------------------------------------------------------------------------
			inline static float Erf(float p_fX, float p_fTolerance = 1e-5f, int p_nMaxTerms = 32)
			{
				return boost::math::erf(p_fX);
			}

			//----------------------------------------------------------------------------------------------
			//----------------------------------------------------------------------------------------------
			inline static float Erfc(float p_fX, float p_fTolerance = 1e-5f, int p_nMaxTerms = 32)
			{
				return boost::math::erfc(p_fX);
			}

			//----------------------------------------------------------------------------------------------
			/// Evaluates the Gaussian probability density function.
			//----------------------------------------------------------------------------------------------
			static float GaussianPDF(float p_fX, float p_fMean = 0, float p_fStandardDeviation = 1)
			{
				// f(x ; m,s) =          1				  (x - m)^2
				//				------------------ exp( - --------- )
				//				s * sqrt( 2 * Pi )           2s^2

				float fLeft = 1.f / (p_fStandardDeviation * Maths::SqrtPiTwo);
				float fNum = Maths::Sqr(p_fX - p_fMean);
				float fDen = 2.f * p_fStandardDeviation * p_fStandardDeviation;
				return fLeft * Maths::Exp(-(fNum / fDen));
			}

			static float GaussianPDFApprox(float p_fX)
			{
				return (1 + Maths::Cos(p_fX)) * Maths::InvPiTwo;
			}
		};
	}
}
