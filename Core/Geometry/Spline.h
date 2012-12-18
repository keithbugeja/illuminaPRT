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
			static float LaGrangePolynomial(int p_j, float p_t, std::vector<float> &p_points)
			{
				float product = 1,
					part;

				for (int m = 0; m < p_points.size(); m++)
				{
					if (m == p_j) continue;
					part = (p_t - p_points[m]) / (p_points[p_j] - p_points[m]); 
					product *= part;
					// std::cout << "LGP : [" << m << " : " << p_j << " :[" << p_points[p_j] << " : " << p_points[m] << "]:" << product << " : " << part << "]" << std::endl;
				}

				return product;
			}

			static Vector3 LaGrange(std::vector<Vector3> &p_points, float p_t)
			{
				// O(N^2) : Not very efficient for large paths!

				Vector3 finalPoint = 0.f;

				float totalDistance, distance;
				std::vector<float> functionPoints;
				
				totalDistance = 0;
				functionPoints.push_back(totalDistance);

				for (int j = 1; j < p_points.size(); j++)
				{
					distance = (p_points[j] - p_points[j - 1]).Length();
					totalDistance += distance;
					functionPoints.push_back(totalDistance);			
				}

				// std::cout << "Total Distance : " << totalDistance << std::endl; 

				for (int j = 1; j < p_points.size(); j++)
				{
					functionPoints[j] = functionPoints[j] / totalDistance;
				}

				for (int j = 0; j < p_points.size(); j++)
				{
					float l = LaGrangePolynomial(j, p_t, functionPoints);

					finalPoint += p_points[j] * l; 
					// std::cout << "[" << j << " : " << l << " : " << finalPoint.ToString() << "]" << std::endl;
				}

				return finalPoint;
			}

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
