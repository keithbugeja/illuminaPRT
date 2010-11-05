//----------------------------------------------------------------------------------------------
//	Filename:	Noise.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>

#include "Maths/Random.h"
#include "Geometry/Vector2.h"
#include "Geometry/Vector3.h"

namespace Illumina 
{
	namespace Core
	{
		class Noise
		{
		protected:
			static const int GradientCount = 16;

			Vector3 m_gradients[GradientCount];
			int m_phi[GradientCount];

		public:
			Noise(void)
			{
				Random random(0x01051976);

				// Create gradient vector index perturbation array
				for (int i = 0; i < GradientCount; i++)
					m_phi[i] = i;

				for (int i = GradientCount - 2; i >= 0; i--)
				{
					int target = (int)(random.NextDouble() * i);
					int temp = m_phi[target];
					m_phi[target] = m_phi[i+1];
					m_phi[i+1] = temp;
				}

				// Generate the gradient look-up tables
				for(int i = 0; i < GradientCount; i++)
				{
					m_gradients[i].X = float(random.NextDouble() - 0.5f) * 2.0f; 
					m_gradients[i].Y = float(random.NextDouble() - 0.5f) * 2.0f; 
					m_gradients[i].Z = float(random.NextDouble() - 0.5f) * 2.0f; 
				}
			}

			float Turbulence(const Vector3 &p, int depth) const
			{
				float sum = 0.0f;
				float weight = 1.0f;
				Vector3 pTemp(p);

				sum = Maths::FAbs(Perlin(pTemp));

				for (int i = 1; i < depth; i++)
				{
					weight *= 2;
					pTemp.Set(p.X * weight, p.Y * weight, p.Z * weight);

					sum += Maths::FAbs(Perlin(pTemp)) / weight;
				}

				return sum;
			}

			float Perlin(const Vector3 &p) const
			{
				// Compute the surrounding point components on 
				// the lattice
				int lx0 = (int)Maths::Floor(p.X),
					ly0 = (int)Maths::Floor(p.Y),
					lz0 = (int)Maths::Floor(p.Z),
					lx1 = lx0 + 1,
					ly1 = ly0 + 1,
					lz1 = lz0 + 1;
					
				// Compute vector components from lattice points 
				// to current point [p]
				float vx0 = p.X - Maths::Floor(p.X),
					vy0 = p.Y - Maths::Floor(p.Y),
					vz0 = p.Z - Maths::Floor(p.Z),
					vx1 = vx0 - 1,
					vy1 = vy0 - 1,
					vz1 = vz0 - 1;

				// Return the gradient vectors on each vertex of the cube
				// bounding the current point
				const Vector3 &g000 = Gamma(lx0, ly0, lz0);
				const Vector3 &g100 = Gamma(lx1, ly0, lz0);
				const Vector3 &g010 = Gamma(lx0, ly1, lz0);
				const Vector3 &g110 = Gamma(lx1, ly1, lz0);
				const Vector3 &g001 = Gamma(lx0, ly0, lz1);
				const Vector3 &g101 = Gamma(lx1, ly0, lz1);
				const Vector3 &g011 = Gamma(lx0, ly1, lz1);
				const Vector3 &g111 = Gamma(lx1, ly1, lz1);

				// Compute the weights for each vertex
				float v000 = g000.X * vx0 + g000.Y * vy0 + g000.Z * vz0;
				float v100 = g100.X * vx1 + g100.Y * vy0 + g100.Z * vz0;
				float v010 = g010.X * vx0 + g010.Y * vy1 + g010.Z * vz0;
				float v110 = g110.X * vx1 + g110.Y * vy1 + g110.Z * vz0;
				float v001 = g001.X * vx0 + g001.Y * vy0 + g001.Z * vz1;
				float v101 = g101.X * vx1 + g101.Y * vy0 + g101.Z * vz1;
				float v011 = g011.X * vx0 + g011.Y * vy1 + g011.Z * vz1;
				float v111 = g111.X * vx1 + g111.Y * vy1 + g111.Z * vz1;

				// Interpolate over x
				float x1 = Lerp(v000, v100, vx0);
				float x2 = Lerp(v010, v110, vx0);
				float x3 = Lerp(v001, v101, vx0);
				float x4 = Lerp(v011, v111, vx0);

				// Interpolate over y
				float y1 = Lerp(x1, x2, vy0);
				float y2 = Lerp(x3, x4, vy0);

				// Interpolate over z
				return Lerp(y1, y2, vz0);
			}

			inline static float Lerp(float u0, float u1, float x) {
				return u0 - Omega(x) * (u0 - u1);
			}

			inline static float Omega(float t) 
			{
				t = (t > 0.0f) ? t : -t;
				return 6*t*t*t*t*t - 15*t*t*t*t + 10*t*t*t;
			}

			inline static float Hermite(float t) {
				return 3*t*t - 2*t*t*t;
			}

			inline int Phi(int i) const {
				return m_phi[(int)Maths::FAbs((float)i) % GradientCount];
			}

			inline Vector3 Gamma(int i, int j, int k) const {
				return m_gradients[Phi(i + Phi(j + Phi(k)))];
			}
		};
	} 
}