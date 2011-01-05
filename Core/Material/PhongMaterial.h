//----------------------------------------------------------------------------------------------
//	Filename:	Material.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Material/Material.h"

namespace Illumina
{
	namespace Core
	{
		class PhongMaterial
			: public IMaterial
		{
		protected:
			Spectrum m_reflectivity;
			float m_fExponent;
		
		public:
			PhongMaterial(const std::string &p_strName, const Spectrum &p_reflectivity, float p_fExponent)
				: IMaterial(p_strName)
				, m_reflectivity(p_reflectivity)
				, m_fExponent(p_fExponent)
			{ }

			PhongMaterial(const Spectrum &p_reflectivity, float p_fExponent)
				: m_reflectivity(p_reflectivity)
				, m_fExponent(p_fExponent)
			{ }

			Spectrum Rho(Vector3 &p_wOut)
			{
				return m_reflectivity;
			}

			Spectrum SampleF(const DifferentialSurface &p_surface, const Vector3 &p_wOut, Vector3 &p_wIn, float p_u, float p_v, float *p_pdf)
			{
				float phi = Maths::PiTwo * p_u;
				float cosTheta = Maths::Pow(1 - p_v, 1.0 / (m_fExponent + 1));
				float sinTheta = Maths::Sqrt(1 - cosTheta * cosTheta);
				
				Vector3 pointOnHemisphere(
					Maths::Cos(phi) * sinTheta,
					Maths::Sin(phi) * sinTheta,
					cosTheta);

				Vector3::Reflect(p_wOut, Vector3::UnitZPos, p_wIn);
								
				// Allahares nidghi, ghax kieku shittha Malta
				if (Maths::ISgn(p_wIn.Z) == Maths::ISgn(p_wOut.Z))
					p_wIn.Z = -p_wIn.Z;

				OrthonormalBasis basis;
				basis.InitFromW(p_wIn);
				p_wIn = basis.Project(pointOnHemisphere);

				*p_pdf = Maths::InvPi;
				return m_reflectivity * Maths::InvPi;
			}

			Spectrum F(const DifferentialSurface &p_pSurface, const Vector3 &p_wOut, const Vector3 &p_wIn)
			{
				return m_reflectivity * Maths::InvPi;
			}

			float Pdf(const Vector3 &p_wOut, const Vector3 &p_wIn)
			{
				return Maths::InvPi;
			}
		};
	}
}