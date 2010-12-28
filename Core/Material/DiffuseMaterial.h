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
		class DiffuseMaterial 
			: public IMaterial
		{
		protected:
			Spectrum m_reflectivity;

		public:
			DiffuseMaterial(const std::string &p_strName, const Spectrum &p_reflectivity)
				: IMaterial(p_strName) 
				, m_reflectivity(p_reflectivity)
			{ }

			DiffuseMaterial(const Spectrum& p_reflectivity)
				: m_reflectivity(p_reflectivity)
			{ }

			Spectrum Rho(Vector3 &p_wOut)
			{
				return m_reflectivity;
			}

			Spectrum SampleF(const Vector3 &p_wOut, Vector3 &p_wIn, float p_u, float p_v, float *p_pdf)
			{
				BSDF::GenerateVectorInHemisphere(p_u, p_v, p_wIn);
				
				// Allahares nidghi, ghax kieku shittha Malta
				if (Maths::ISgn(p_wIn.Z) == Maths::ISgn(p_wOut.Z))
					p_wIn.Z = -p_wIn.Z;

				*p_pdf = Maths::InvPi;
				return m_reflectivity * Maths::InvPi;
			}

			Spectrum F(const Vector3 &p_wOut, const Vector3 &p_wIn)
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