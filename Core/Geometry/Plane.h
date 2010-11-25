//----------------------------------------------------------------------------------------------
//	Filename:	Plane.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>

#include "Maths/Maths.h"
#include "Geometry/Vector3.h"

namespace Illumina
{
	namespace Core
	{
		class Plane
		{
		public:
			enum Side
			{
				Side_Neither,
				Side_Negative,
				Side_Positive,
				Side_Both,

				Side_Count
			};

		public:
			Plane(void) { }	

			Plane(const Plane &p_plane) 
				: Distance( p_plane.Distance)
				, Normal( p_plane.Normal )
			{ }

			Plane(const Vector3 &p_normal, float p_fDistance) 
				: Normal(p_normal)
				, Distance(p_fDistance) 
			{ }

			Plane(const Vector3 &p_normal, const Vector3 &p_point) 
			{
				// This should formally be transcribed as:
				// TVector3<TReal> P = p_planePoint - p_planePoint.Origin;
				// Distance = P.DotProduct( p_normalVector );
				// Normal = p_normalVector;

				Distance = Vector3::Dot(p_normal, p_point);
				Normal = p_normal;
			}

			Plane(const Vector3 &p_point1, const Vector3 &p_point2, const Vector3 &p_point3) 
			{
				// N = norm(AB x AC)
				// D = (A - O) . N

				Vector3 AB = p_point2 - p_point1,
						AC = p_point3 - p_point1;
		
				Vector3::Cross(AB, AC, Normal);
				Normal.Normalize();

				Distance = Vector3::Dot(p_point1, Normal);
			}

			inline Vector3 GetReflectionVector(const Vector3 &p_vector) const {
				return p_vector - (Normal * 2 * (Normal.Dot(p_vector) - Distance));
			}

			inline void GetDisplacementVector(const Vector3 &p_vector, Vector3 &p_out) const {
				p_out = Normal * (Normal.Dot(p_vector) - Distance);
			}

			inline Vector3 GetDisplacementVector(const Vector3 &p_vector) const {
				return Normal * (Normal.Dot(p_vector) - Distance);
			}

			inline void GetReflectionVector(const Vector3 &p_vector, Vector3 &p_out) const
			{
				p_out = Normal * (2.0f * Vector3::Dot(Normal, p_vector) - Distance);
				Vector3::Subtract(p_vector, p_out, p_out);
			}

			inline float GetDisplacement(const Vector3& p_vector) const
			{
				// s = (V - dN) . N
				//   = V . N - dN . N
				//   = V . N - dN^2

				// |N| = 1
				// N^2 = 1^2 = 1

				// s = V . N - d(1)
				//   = V . N - d

				return Normal.Dot(p_vector) - Distance;
			}

			inline float GetDistance(const Vector3 &p_vector) const
			{
				return Maths::Abs(Normal.Dot(p_vector) - Distance);
			}

			Side GetSide(const Vector3 &p_vector) const
			{
				// (P - dN) . N = 0
				Vector3 P = p_vector - (Distance * Normal);
				int nDotSign = Maths::ISgn(P.Dot(Normal));
		
				if ( nDotSign == -1 )
					return Side_Negative;
				else if (nDotSign == 1)
					return Side_Positive;
		
				// Co-planar
				return Side_Neither;
			}

			Side GetSide(const Vector3 &p_midPoint, const Vector3 &p_halfVector) const
			{
				const Vector3 &M = p_midPoint - (Distance * Normal),
						&A = M + p_halfVector,
						&B = M - p_halfVector;

				int nASign = Maths::ISgn(A.Dot(Normal)),
					nBSign = Maths::ISgn(B.Dot(Normal));

				// Both points lie in the same half-space
				if ( nASign - nBSign == 0 )
				{
					if ( nASign == -1 )
						return Side_Negative;
					else if (nASign == 1)
						return Side_Positive;

					return Side_Neither;
				}
		
				return Side_Both;
			}

			inline void Normalize(void) {
				Normal.Normalize();
			}

			bool operator==(const Plane &p_plane) const {
				return (p_plane.Distance == Distance && p_plane.Normal == Normal);
			}

			bool operator!=(const Plane &p_plane) const {
				return !(*this == p_plane);
			}

		public:
			float	Distance;	
			Vector3 Normal;
		};
	}
}