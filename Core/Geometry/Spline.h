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

			template <class T>
			static T CatmullRom(const T &p_p0, const T &p_p1, const T &p_p2, const T &p_p3, float t)
			{
				float t2 = t * t,
					t3 = t2 * t;

				return 0.5f * ((2 * p_p1) +
					(-p_p0 + p_p2) * t +
					(2 * p_p0 - 5 * p_p1 + 4 * p_p2 - p_p3) * t2 +
					(-p_p0 + 3 * p_p1 - 3 * p_p2 + p_p3) * t3);
			}

			static float SmoothStep(float min, float max, float value)
			{
				float v = (value - min) / (max - min);
				if (v < 0.0f) v = 0.0f;
				if (v > 1.0f) v = 1.0f;
				return v * v * (-2.f * v  + 3.f);
			}
		};

		class Interpolator
		{
		private:
			static float LagrangePolynomial(int p_j, float p_t, std::vector<float> &p_points)
			{
				float product = 1;

				for (int m = 0; m < p_points.size(); m++)
				{
					if (m == p_j) continue;
					product *= (p_t - p_points[m]) / (p_points[p_j] - p_points[m]); 
				}

				return product;
			}

		public:
			static void ComputePivots(std::vector<Vector3> &p_points, std::vector<float> &p_pivotList)
			{
				float totalDistance = 0;
				p_pivotList.clear();
				p_pivotList.push_back(0);

				// Compute Euclidean distance between points
				for (int j = 1; j < p_points.size(); j++)
				{
					totalDistance += (p_points[j] - p_points[j - 1]).Length();
					p_pivotList.push_back(totalDistance);
				}

				// Scale in the range [0 .. 1)
				for (int j = 1; j < p_points.size(); j++)
				{
					p_pivotList[j] = p_pivotList[j] / totalDistance;
				}
			}

			static Vector3 Lagrange(std::vector<Vector3> &p_pointList, std::vector<float> &p_pivotList, float p_t)
			{
				// O(N^2) : Not very efficient for large paths!
				Vector3 finalPoint = 0.f;

				for (int j = 0; j < p_pointList.size(); j++)
				{
					finalPoint += p_pointList[j] * LagrangePolynomial(j, p_t, p_pivotList);
				}

				return finalPoint;
			}

			static Vector3 Lerp(std::vector<Vector3> &p_controlPoints, float p_x)
			{
				Vector3 d0, d1;
				float x_frac;

				if (p_controlPoints.size() == 2)
				{
					d0 = p_controlPoints[0];
					d1 = p_controlPoints[1];
					x_frac = p_x;
				}
				else
				{
					float x = p_x * Maths::Abs((float)(p_controlPoints.size() - 2));
					int x_int = (int)Maths::Floor(x);
					x_frac = x - x_int;

					d0 = p_controlPoints[x_int];
					d1 = p_controlPoints[x_int + 1];
				}

				return d0 + (d1 - d0) * x_frac;
			}

			static void PadForCubicInterpolation(std::vector<Vector3> &p_controlPoints)
			{
				p_controlPoints.insert(p_controlPoints.begin(), p_controlPoints[0]);
				p_controlPoints.push_back(p_controlPoints.back());
				p_controlPoints.push_back(p_controlPoints.back());
			}

			/*
			 * Assumes extra points at the start and end! 
			 */
			static Vector3 CubicInterpolation(std::vector<Vector3> &p_controlPoints, float p_x)
			{
				float x = 1 + p_x * Maths::Abs((float)(p_controlPoints.size() - 4));
				int x_int = (int)Maths::Floor(x);
				float x_frac = x - x_int;

				Vector3 d0 = p_controlPoints[x_int - 1],
						d1 = p_controlPoints[x_int],
						d2 = p_controlPoints[x_int + 1],
						d3 = p_controlPoints[x_int + 2];

				return Spline::CatmullRom<Vector3>(d0, d1, d2, d3, x_frac);
			}
		};
	}
}
