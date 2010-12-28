//----------------------------------------------------------------------------------------------
//	Filename:	Montecarlo.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "Geometry/Vector3.h"
#include "Geometry/Basis.h" 
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class Montecarlo
		{
		public:
			static Vector3 UniformSampleSphere(float p_u, float p_v)
			{
				float z = 1.0f - 2.0 * p_u;
				float r = Maths::Sqrt(Maths::Max(0.0f, 1.0f - z * z));
				float phi = Maths::PiTwo * p_v;
				float x = r * Maths::Cos(phi);
				float y = r * Maths::Sin(phi);
				return Vector3(x, y, z);
			}

			static void UniformSampleTriangle(float p_u, float p_v, float *p_uOut, float *p_vOut) 
			{
				float su1 = Maths::Sqrt(p_u);
				*p_uOut = 1.f - su1;
				*p_vOut = p_v * su1;
			}

			static Vector3 UniformSampleCone(float p_u, float p_v, float p_cosThetaMax) 
			{
				float cosTheta = (1.0f - p_u) + p_u * p_cosThetaMax;
				float sinTheta = Maths::Sqrt(1.0f - cosTheta * cosTheta);
				float phi = p_v * 2.0f * Maths::Pi;
				return Vector3(Maths::Cos(phi) * sinTheta, Maths::Sin(phi) * sinTheta, cosTheta);
			}

			static Vector3 UniformSampleCone(float p_u, float p_v, float p_cosThetaMax, OrthonormalBasis &p_basis) 
			{
				float cosTheta = (1.0f - p_u) + p_u * p_cosThetaMax;
				float sinTheta = Maths::Sqrt(1.0f - cosTheta * cosTheta);
				float phi = p_v * Maths::PiTwo;
				return Maths::Cos(phi) * sinTheta * p_basis.U + Maths::Sin(phi) * sinTheta * p_basis.V + cosTheta * p_basis.W;
			}

			static float UniformConePdf(float p_cosThetaMax)
			{
				return 1.0f / (Maths::PiTwo * (1.0f - p_cosThetaMax));
			}
		};
	}
}