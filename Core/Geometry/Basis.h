//----------------------------------------------------------------------------------------------
//	Filename:	Basis.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>

#include "Maths/Maths.h"
#include "Geometry/Vector2.h"
#include "Geometry/Vector3.h"
#include "Geometry/Matrix3x3.h"

namespace Illumina 
{
	namespace Core
	{
		class OrthonormalBasis
		{
		private:
			static const float Epsilon;

		public:
			Vector3 U, V, W;

		protected:
			void FrowWV(const Vector3 &p_w, const Vector3 &p_v)
			{
				// Using Gram-Schmidt
				W = Vector3::Normalize(p_w);
				V = Vector3::Normalize(p_v);
				V = V - Vector3::Dot(V, W) * W;

				// Do we choose another axis?
				if (V.LengthSquared() <= Maths::Epsilon)
				{
					V.Set(W.Z, W.X, W.Y);
					V = Vector3::Normalize(Vector3::Cross(V, W));
					V = Vector3::Normalize(V - Vector3::Dot(V, W) * W);
				}

				U = Vector3::Cross(V, W);
			}

		public:
			OrthonormalBasis() {}

			OrthonormalBasis(const Vector3 &p_u, const Vector3 &p_v, const Vector3 &p_w) {
				U = p_u; V = p_v; W = p_w;
			}
 
			void InitFromU(const Vector3 &p_u)
			{
				U = Vector3::Normalize(p_u);
				V = Vector3::Cross(U, Vector3::UnitXPos);
				
				if (V.LengthSquared() < Epsilon)
					V = Vector3::Cross(U, Vector3::UnitYPos);

				W = Vector3::Cross(U, V);
			}

			void InitFromV(const Vector3 &p_v)
			{
				V = Vector3::Normalize(p_v);
				U = Vector3::Cross(V, Vector3::UnitXPos);
				
				if (U.LengthSquared() < Epsilon)
					U = Vector3::Cross(V, Vector3::UnitYPos);

				W = Vector3::Cross(U, V);
			}

			void InitFromW(const Vector3 &p_w)
			{
				W = Vector3::Normalize(p_w);
				U = Vector3::Cross(W, Vector3::UnitXPos);
				
				if (U.LengthSquared() < Epsilon)
					U = Vector3::Cross(W, Vector3::UnitYPos);

				V = Vector3::Cross(W, U);
			}

			void InitFromUV(const Vector3 &p_u, const Vector3 &p_v)
			{
				U = Vector3::Normalize(p_u);
				W = Vector3::Normalize(Vector3::Cross(p_u, p_v));
				V = Vector3::Cross(W, U);
			}

			void InitFromVU(const Vector3 &p_v, const Vector3 &p_u)
			{
				V = Vector3::Normalize(p_v);
				W = Vector3::Normalize(Vector3::Cross(p_u, p_v));
				U = Vector3::Cross(V, W);
			}

			void InitFromUW(const Vector3 &p_u, const Vector3 &p_w)
			{
				U = Vector3::Normalize(p_u);
				V = Vector3::Normalize(Vector3::Cross(p_w, p_u));
				W = Vector3::Cross(U, V);
			}

			void InitFromWU(const Vector3 &p_w, const Vector3 &p_u)
			{
				W = Vector3::Normalize(p_w);
				V = Vector3::Normalize(Vector3::Cross(p_w, p_u));
				U = Vector3::Cross(V, W);
			}
			
			void InitFromVW(const Vector3 &p_v, const Vector3 &p_w)
			{
				V = Vector3::Normalize(p_v);
				U = Vector3::Normalize(Vector3::Cross(p_v, p_w));
				W = Vector3::Cross(U, V);
			}

			void InitFromWV(const Vector3 &p_w, const Vector3 &p_v)
			{
				this->FrowWV(p_w, p_v);
				//Shirley's original version
				//W = Vector3::Normalize(p_w);
				//U = Vector3::Normalize(Vector3::Cross(p_v, p_w));
				//V = Vector3::Cross(W, U);
			}

			Vector3 GetU() const { return U; }
			Vector3 GetV() const { return V; }
			Vector3 GetW() const { return W; }

			Matrix3x3 GetMatrix() const { 
				return Matrix3x3(U, V, W);
			}

			bool operator==(const OrthonormalBasis &p_basis) const {
				return (U == p_basis.U) && (V == p_basis.V) && (W == p_basis.W);
			}

			inline static Vector2 ToSpherical(const Vector3 &p_point) {
				return Vector2(Maths::Atan(p_point.Y, p_point.X), Maths::Acos(p_point.Z));
			}

			inline static Vector2 ToSpherical(const Vector3 &p_point, const Vector3 &p_centre)
			{
				Vector3 direction = Vector3::Normalize(p_point - p_centre);
				return OrthonormalBasis::ToSpherical(direction);
			}

			inline static Vector3 FromSpherical(const Vector2 &p_omega)
			{
				float sinTheta = Maths::Sin(p_omega.X);
				return Vector3(Maths::Cos(p_omega.Y) * sinTheta,
					Maths::Sin(p_omega.Y) * sinTheta,
					Maths::Cos(p_omega.X));
			}

			inline static Vector3 FromSpherical(const Vector2 &p_omega, const Vector3 &p_centre, float p_fRadius = 1.0f) {
				return p_centre + p_fRadius * FromSpherical(p_omega);
			}

			std::string ToString(void)
			{
				boost::format formatter;
				std::string strOut = boost::str(boost::format("[%1% %2% %3%]") 
					% U.ToString() % V.ToString() % W.ToString());
				return strOut;
			}
		};
	}
}
