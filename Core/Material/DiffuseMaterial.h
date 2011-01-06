//----------------------------------------------------------------------------------------------
//	Filename:	Material.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Material/Material.h"
#include "Texture/Texture.h"

namespace Illumina
{
	namespace Core
	{
		class DiffuseMaterial 
			: public IMaterial
		{
		protected:
			Spectrum m_reflectivity;
			ITexture *m_pDiffuseTexture;

		public:
			DiffuseMaterial(const std::string &p_strName, const Spectrum &p_reflectivity)
				: IMaterial(p_strName) 
				, m_reflectivity(p_reflectivity)
				, m_pDiffuseTexture(NULL)
			{ }

			DiffuseMaterial(const Spectrum& p_reflectivity)
				: m_reflectivity(p_reflectivity)
				, m_pDiffuseTexture(NULL)
			{ }

			void SetDiffuseTexture(ITexture* p_pTexture)
			{
				std::cout << "Setting diffuse texture of " << this->GetName() << " to " << p_pTexture->GetName() << std::endl;
				m_pDiffuseTexture = p_pTexture;
			}

			Spectrum Rho(Vector3 &p_wOut)
			{
				return m_reflectivity;
			}

			Spectrum SampleF(const DifferentialSurface &p_surface, const Vector3 &p_wOut, Vector3 &p_wIn, float p_u, float p_v, float *p_pdf)
			{
				BSDF::GenerateVectorInHemisphere(p_u, p_v, p_wIn);
				
				// Allahares nidghi, ghax kieku shittha Malta
				if (Maths::ISgn(p_wIn.Z) == Maths::ISgn(p_wOut.Z))
					p_wIn.Z = -p_wIn.Z;

				*p_pdf = Maths::InvPi;

				if (m_pDiffuseTexture != NULL)
				{
					RGBPixel rgb = m_pDiffuseTexture->GetValue(p_surface.PointUV, Vector3::Zero);
					Spectrum diffuse(rgb.R, rgb.G, rgb.B);
					return diffuse * m_reflectivity * Maths::InvPi;
				}

				return m_reflectivity * Maths::InvPi;
			}

			Spectrum F(const DifferentialSurface &p_surface, const Vector3 &p_wOut, const Vector3 &p_wIn)
			{
				if (m_pDiffuseTexture != NULL)
				{
					RGBPixel rgb = m_pDiffuseTexture->GetValue(p_surface.PointUV, Vector3::Zero);
					Spectrum diffuse(rgb.R, rgb.G, rgb.B);
					return diffuse * m_reflectivity * Maths::InvPi;
				}

				return m_reflectivity * Maths::InvPi;
			}

			float Pdf(const Vector3 &p_wOut, const Vector3 &p_wIn)
			{
				return Maths::InvPi;
			}
		};
	}
}