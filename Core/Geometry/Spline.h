//----------------------------------------------------------------------------------------------
//	Filename:	Spline.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <limits>
#include <cmath>

#include <System/IlluminaPRT.h>
#include <Geometry/Vector3.h>

namespace Illumina
{
	namespace Core
	{
		class Spline
		{
		public:
			template <class T>
			static T Hermite(const T &p_x0, const T &p_x1, const T &p_m0, const T &p_m1, float p_t)
			{
				float t_sq = p_t * p_t,
					t_cb = t_sq * p_t;

				return (2 * t_cb - 3 * t_sq + 1) * p_x0 +
					(t_cb - 2 * t_sq + p_t) * p_m0 +
					(-2 * t_cb + 3 * t_sq) * p_x1 + 
					(t_cb - t_sq) * p_m1;
			}

			template <class T>
			static T Hermite(const T &p_x0, const T &p_x1, const T &p_m0, const T &p_m1, float p_fAccent, float p_t)
			{
				float t_sq = p_t * p_t,
					t_cb = t_sq * p_t;

				return (2 * t_cb - 3 * t_sq + 1) * p_x0 +
					(t_cb - 2 * t_sq + p_t) * p_fAccent * p_m0 +
					(-2 * t_cb + 3 * t_sq) * p_x1 + 
					(t_cb - t_sq) * p_fAccent * p_m1;
			}
		};
	}
}
