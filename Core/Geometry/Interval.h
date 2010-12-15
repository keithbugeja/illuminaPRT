//----------------------------------------------------------------------------------------------
//	Filename:	Interval.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Implementation for Intervals
//----------------------------------------------------------------------------------------------
#pragma once

#include "Maths/Maths.h"
#include "Threading/List.h"
//----------------------------------------------------------------------------------------------
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
			Interval(void);
			Interval(float p_fValue);
			Interval(float p_fMin, float p_fMax);
			Interval(const Interval& p_interval); 

			float operator[](int p_nIndex) const;
			float& operator[](int p_nIndex);

			inline void Set(float p_fValue);
			inline void Set(float p_fMin, float p_fMax);

			inline Interval& operator=(const Interval &p_interval);
			inline bool operator==(const Interval &p_interval) const;
			inline bool operator!=(const Interval& p_interval) const;

			inline void Extend(float p_fValue);
			inline float Median(void) const;
			inline float Span(void) const;

			inline bool Intersects(const Interval& p_interval) const;
			inline bool Contains(float p_fValue) const;

			std::string ToString(void) const;
		};

		typedef List<Interval> IntervalList;
	}
}

#include "Geometry/Interval.inl"