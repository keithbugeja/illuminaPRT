//----------------------------------------------------------------------------------------------
//	Filename:	Interval.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Implementation for Intervals
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>

#include "Maths/Maths.h"
#include "Threading/List.h"

namespace Illumina 
{
	namespace Core
	{
		class Interval
		{
		public:
			union
			{
				float Element[2];
				struct { float Min, Max; };
			};

		public:
			Interval() {}
			Interval(float p_fValue) 
				: Min(p_fValue), Max(p_fValue) {}
			Interval(float p_fMin, float p_fMax) 
				: Min(p_fMin), Max(p_fMax) {}
			Interval(const Interval& p_interval) 
				: Min(p_interval.Min), Max(p_interval.Max) {}

			float operator[](int p_nIndex) const { return Element[p_nIndex]; }
			float& operator[](int p_nIndex) { return Element[p_nIndex]; }

			inline void Set(float p_fValue) {
				Min = Max = p_fValue;
			}

			inline void Set(float p_fMin, float p_fMax) {
				Min = p_fMin; Max = p_fMax;
			}
	
			Interval& operator=(const Interval &p_interval)
			{
				Min = p_interval.Min;
				Max = p_interval.Max;
				
				return *this;
			}

			inline bool operator==(const Interval &p_interval) const
			{
				if (Min != p_interval.Min) return false;
				if (Max != p_interval.Max) return false;

				return true;
			}

			inline bool operator!=(const Interval& p_interval) const {
				return !(*this == p_interval);
			}

			inline void Extend(float p_fValue) 
			{
				if (Min > p_fValue) Min = p_fValue;
				if (Max < p_fValue) Max = p_fValue;
			}

			inline float Median(void) const {
				return (Min + Max) * 0.5f;
			}

			inline float Span(void) const {
				return Max - Min;
			}

			inline bool Intersects(const Interval& p_interval) const
			{
				return Max >= p_interval.Min && Min <= p_interval.Max;
			}

			inline bool Contains(float p_fValue) const
			{
				return Min <= p_fValue && p_fValue <= Max;
			}

			std::string ToString(void)
			{
				boost::format formatter;
				std::string strOut = boost::str(boost::format("[%d %d]") % Min % Max);
				return strOut;
			}
		};

		typedef List<Interval> IntervalList;
	}
}