//----------------------------------------------------------------------------------------------
//	Filename:	Material.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Material/SpecularReflectionBxDF.h"
#include "Material/SpecularTransmissionBxDF.h"
#include "Texture/Texture.h"

namespace Illumina
{
	namespace Core
	{
		class GlassMaterial 
			: public IMaterial
		{
		protected:
			using BSDF::m_bxdfList;

		protected:
			Spectrum m_reflectivity,
				m_transmittance;
			
			float m_fAbsorption;
			
			float m_fEtaI,
				m_fEtaT;

			ITexture *m_pTexture;

		public:
			GlassMaterial(const std::string &p_strName, const Spectrum &p_reflectivity, const Spectrum &p_transmittance, float p_fAbsorption = 1.0f, float p_fEtaI = 1.0f, float p_fEtaT = 1.52f, ITexture *p_pTexture = NULL)
				: IMaterial(p_strName) 
				, m_reflectivity(p_reflectivity)
				, m_transmittance(p_transmittance)
				, m_fAbsorption(p_fAbsorption)
				, m_fEtaI(p_fEtaI)
				, m_fEtaT(p_fEtaT)
				, m_pTexture(p_pTexture)
			{
				m_bxdfList.PushBack(new SpecularTransmission(p_fEtaI, p_fEtaT));
				m_bxdfList.PushBack(new SpecularReflection());
			}

			GlassMaterial(const Spectrum &p_reflectivity, const Spectrum &p_transmittance, float p_fAbsorption = 1.0f, float p_fEtaI = 1.0f, float p_fEtaT = 1.52f, ITexture *p_pTexture = NULL)
				: m_reflectivity(p_reflectivity)
				, m_transmittance(p_transmittance)
				, m_fAbsorption(p_fAbsorption)
				, m_fEtaI(p_fEtaI)
				, m_fEtaT(p_fEtaT)
				, m_pTexture(p_pTexture)
			{
				m_bxdfList.PushBack(new SpecularTransmission(p_fEtaI, p_fEtaT));
				m_bxdfList.PushBack(new SpecularReflection());
			}

			~GlassMaterial(void)
			{
				delete m_bxdfList.At(1);
				delete m_bxdfList.At(0);
			}

			void SetTexture(ITexture* p_pTexture)
			{
				m_pTexture = p_pTexture;
			}
			
			Spectrum SampleF(const DifferentialSurface &p_surface, const Vector3 &p_wOut, Vector3 &p_wIn, float p_u, float p_v, 
				float *p_pdf, BxDF::Type p_bxdfType = BxDF::All_Combined, BxDF::Type *p_sampledBxDFType = NULL)
			{
				Spectrum F = BSDF::SampleF(p_surface, p_wOut, p_wIn, p_u, p_v, p_pdf, p_bxdfType, p_sampledBxDFType);
				Spectrum fresnel = EvaluateFresnelTerm(p_wOut.Z, m_fEtaI, m_fEtaT);

				if (*p_sampledBxDFType & BxDF::Reflection)
					return F * fresnel;

				return F * (Spectrum(1.0f) - fresnel);
			}
			
			Spectrum EvaluateFresnelTerm(float cosi, float etai, float etat) const
			{
				// Compute Fresnel reflectance for dielectric
				cosi = Maths::Clamp(cosi, -1.0f, 1.0f);
		
				// Compute indices of refraction for dielectric
				bool entering = cosi > 0.;
				float ei = etai, et = etat;
		
				if (!entering)
					Maths::Swap(ei, et);
		
				// Compute _sint_ using Snell's law
				float sint = ei / et * Maths::Sqrt(Maths::Max(0.f, 1.0f - cosi*cosi));
		
				if (sint >= 1.0f) 
				{
					// Handle total internal reflection
					return 1.0f;
				}
				else
				{
					float cost = Maths::Sqrt(Maths::Max(0.0f, 1.0f - sint*sint));
					return FresnelDielectric(Maths::FAbs(cosi), cost, ei, et);
				}
			}

			Spectrum FresnelDielectric(const float cosi, 
						  const float cost, 
						  const float &etai, 
						  const float &etat) const
			{
				float Rparl = ((etat * cosi) - (etai * cost)) /
							   ((etat * cosi) + (etai * cost));
		
				float Rperp = ((etai * cosi) - (etat * cost)) /
							   ((etai * cosi) + (etat * cost));

				return (Rparl * Rparl + Rperp * Rperp) / 2.0f;
			}

			Spectrum SampleTexture(const DifferentialSurface &p_surface, int p_bxdfIndex)
			{
				if (m_pTexture)
				{
					RGBPixel pixel = m_pTexture->GetValue(p_surface.PointUV, p_surface.PointWS);
					return Spectrum(pixel.R, pixel.G, pixel.B);
				}
				else
				{
					return p_bxdfIndex == 0 ? m_transmittance : m_reflectivity;// p_bxdfIndex == 0 ? m_reflectivity : Spectrum(1.0f) - m_reflectivity;
				}

			}
		};
	}
}