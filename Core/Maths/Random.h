//----------------------------------------------------------------------------------------------
//	Filename:	Random.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Provides pseudo-random number generation over a uniform probability distribution
//  using the mersenne twister algorithm. 
//	Reference: http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/MT2002/CODES/mt19937ar.c
//----------------------------------------------------------------------------------------------
#pragma once

// Period parameters
#define MERSENNE_N				624
#define MERSENNE_M				397
#define MERSENNE_MATRIX_A		0x9908b0dfU
#define	MERSENNE_UPPER_MASK		0x80000000U
#define	MERSENNE_LOWER_MASK		0x7fffffffU
#define MERSENNE_INVERSE_RNG	2.3283064370807974e-010

// Tempering parameters
#define MERSENNE_TEMPERING_MASK_B 0x9d2c5680
#define	MERSENNE_TEMPERING_MASK_C 0xefc60000

namespace Illumina 
{
	namespace Core
	{
		class Random
		{
		protected:
			static const unsigned int m_unMagicNumbers[2]; 

			// State vector & index
			unsigned int m_unStateVector[MERSENNE_N];
			short m_sStateVectorIndex;

			// Operating parameters
			unsigned int m_unSeed;

		public:    
			Random(void);
			Random(unsigned int p_unSeed);

			void Reset(void); 
			void Seed(unsigned int p_unSeed);

			unsigned int Next(unsigned int p_unRangeMax);
			unsigned int Next(unsigned int p_unRangeMin, unsigned int p_unRangeMax );
			unsigned int Next(void);
			
			float NextFloat(void);
			
			double NextDouble(void);
		};
	}
}