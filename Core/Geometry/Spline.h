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
			/*
			 * Temporary! For video recording of CGF paper!
			 */
			template <class T>
			static void ComputePivots(std::vector<T> &p_vertexList, std::vector<float> &p_pivotList)
			{
				float totalDistance = 0;
				p_pivotList.clear();
				p_pivotList.push_back(0);

				// Compute Euclidean distance between points
				for (int j = 1; j < p_vertexList.size(); j++)
				{
					float t = 0, t2;

					std::cout << "Elements :: ";

					for (int i = 0; i < p_vertexList[j].size(); i++)
					{
						std::cout << p_vertexList[j][i] << ", ";

						t2 = p_vertexList[j][i] - p_vertexList[j - 1][i];
						t += t2 * t2;
					}

					totalDistance += Maths::Sqrt(t);
					p_pivotList.push_back(totalDistance);

					std::cout << std::endl << "Point [" << j << "] has t of " << t << "with a total distance of " << totalDistance << std::endl;
				}

				// Scale in the range [0 .. 1)
				for (int j = 1; j < p_pivotList.size(); j++)
				{
					p_pivotList[j] = p_pivotList[j] / totalDistance;

					std::cout << "Normalised distance for Point [" << j << "] is " << p_pivotList[j] << std::endl;
				}
			}

			template <class T>
			static T Lagrange(std::vector<T> &p_vertexList, std::vector<float> &p_pivotList, float p_t)
			{
				// O(N^2) : Not very efficient for large paths!
				T finalPoint;
				
				for (int i = 0; i < finalPoint.size(); i++)
					finalPoint[i] = 0.f;
				
				float lp;

				for (int j = 0; j < p_vertexList.size(); j++)
				{
					lp = LagrangePolynomial(j, p_t, p_pivotList);

					for (int i = 0; i < p_vertexList[j].size(); i++)
						finalPoint[i] += p_vertexList[j][i] * lp;
				}

				return finalPoint;
			}

			// 
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
		};

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
