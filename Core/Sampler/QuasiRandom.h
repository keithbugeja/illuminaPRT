//----------------------------------------------------------------------------------------------
//	Filename:	LowDiscrepancySampler.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <cstring>

#include "Maths/Random.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		// Quasi-random sequence generator
		//----------------------------------------------------------------------------------------------
		class QuasiRandomSequence 
		{
		public:
			//----------------------------------------------------------------------------------------------
			inline static float Halton(unsigned int index, unsigned int base)
			{
				float result = 0;
				float f = 1.0 / base;
				int i = index;
				while (i > 0) 
				{
					result = result + f * (i % base);
					i = (int)Maths::Floor((float)i / base);
					f = f / base;
				}

				return result;
			}
			//----------------------------------------------------------------------------------------------
			inline static double VanDerCorput(unsigned int bits, unsigned int r = 0)
			{
				bits = (bits << 16) | (bits >> 16);
				bits = ((bits & 0x00ff00ff) << 8) | ((bits & 0xff00ff00) >> 8);
				bits = ((bits & 0x0f0f0f0f) << 4) | ((bits & 0xf0f0f0f0) >> 4);
				bits = ((bits & 0x33333333) << 2) | ((bits & 0xcccccccc) >> 2);
				bits = ((bits & 0x55555555) << 1) | ((bits & 0xaaaaaaaa) >> 1);
				bits ^= r;

				return (double) bits / (double) 0x100000000L;
			}
			//----------------------------------------------------------------------------------------------
			inline static double Sobol2(unsigned int i, unsigned int r = 0)
			{
				for (unsigned int v = 1 << 31; i ; i >>= 1, v ^= v >> 1)
					if (i & 1) r ^= v;
			
				return (double) r / (double) 0x100000000L;
			}
			//----------------------------------------------------------------------------------------------
			inline static double RI_LP(unsigned int i, unsigned int r = 0)
			{
				for (unsigned int v = 1 << 31; i; i >>= 1, v |= v >> 1)
					if (i & 1) r ^= v;

				return (double) r / (double) 0x100000000L;
			}
			//----------------------------------------------------------------------------------------------
		};
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
		// Sequence Generators
		//----------------------------------------------------------------------------------------------
		class ISequenceGenerator
		{
		public:
			virtual float operator()(void) = 0;
			virtual float operator()(int) = 0;
		};
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
		template<int TSeed>
		class HaltonSequenceGenerator 
			: public ISequenceGenerator
		{
		private:
			unsigned int m_nSequenceId;
		
		public:
			HaltonSequenceGenerator(void) : m_nSequenceId(0) { }
			
			float operator()(void) { 
				return QuasiRandomSequence::Halton(m_nSequenceId++, TSeed); 
			}
			
			float operator()(int p_nSequenceId) { 
				return QuasiRandomSequence::Halton(p_nSequenceId, TSeed); 
			}
		};
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
		template<int TSeed>
		class SobolSequenceGenerator
			: public ISequenceGenerator
		{
		private:
			unsigned int m_nSequenceId;
		
		public:
			SobolSequenceGenerator(void) : m_nSequenceId(0) { }
			
			float operator()(void) { 
				return (float)QuasiRandomSequence::Sobol2(m_nSequenceId++, TSeed); 
			}

			float operator()(int p_nSequenceId) { 
				return QuasiRandomSequence::Sobol2(p_nSequenceId, TSeed); 
			}
		};
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
		template<int TSeed>
		class VanDerCorputSequenceGenerator
			: public ISequenceGenerator
		{
		private:
			unsigned int m_nSequenceId;
		
		public:
			VanDerCorputSequenceGenerator(void) : m_nSequenceId(0) { }
			
			float operator()(void) { 
				return (float)QuasiRandomSequence::VanDerCorput(m_nSequenceId++, TSeed); 
			}

			float operator()(int p_nSequenceId) { 
				return QuasiRandomSequence::VanDerCorput(p_nSequenceId, TSeed); 
			}
		};
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
		template<int TSeed>
		class RandomSequenceGenerator
			: public ISequenceGenerator
		{
		private:
			Random m_random;
		
		public:
			RandomSequenceGenerator(void) : m_random(TSeed) { }

			float operator()(void) { 
				return m_random.NextFloat(); 
			}

			float operator()(int p_nSequenceId) { 
				return m_random.NextFloat(); 
			}
		};
		//----------------------------------------------------------------------------------------------
	} 
}