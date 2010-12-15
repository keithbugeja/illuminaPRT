//----------------------------------------------------------------------------------------------
//	Filename:	Basis.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <string>

#include "System/IlluminaPRT.h"

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
			void FrowWV(const Vector3 &p_w, const Vector3 &p_v);

		public:
			OrthonormalBasis(void);
			OrthonormalBasis(const Vector3 &p_u, const Vector3 &p_v, const Vector3 &p_w);
 
			void InitFromU(const Vector3 &p_u);
			void InitFromV(const Vector3 &p_v);
			void InitFromW(const Vector3 &p_w);
			void InitFromUV(const Vector3 &p_u, const Vector3 &p_v);
			void InitFromVU(const Vector3 &p_v, const Vector3 &p_u);
			void InitFromUW(const Vector3 &p_u, const Vector3 &p_w);
			void InitFromWU(const Vector3 &p_w, const Vector3 &p_u);
			void InitFromVW(const Vector3 &p_v, const Vector3 &p_w);
			void InitFromWV(const Vector3 &p_w, const Vector3 &p_v);

			Vector3 GetU(void) const;
			Vector3 GetV(void) const;
			Vector3 GetW(void) const;

			Matrix3x3 GetMatrix(void) const;

			bool operator==(const OrthonormalBasis &p_basis) const;

			inline static Vector2 ToSpherical(const Vector3 &p_point);
			inline static Vector2 ToSpherical(const Vector3 &p_point, const Vector3 &p_centre);
			inline static Vector3 FromSpherical(const Vector2 &p_omega);
			inline static Vector3 FromSpherical(const Vector2 &p_omega, const Vector3 &p_centre, float p_fRadius = 1.0f);

			std::string ToString(void);
		};
	}
}

#include "Basis.inl"
