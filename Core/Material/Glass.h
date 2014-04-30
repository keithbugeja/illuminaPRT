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
				m_bxdfList.push_back(new SpecularTransmission(p_fEtaI, p_fEtaT));
				m_bxdfList.push_back(new SpecularReflection());
			}

			GlassMaterial(const Spectrum &p_reflectivity, const Spectrum &p_transmittance, float p_fAbsorption = 1.0f, float p_fEtaI = 1.0f, float p_fEtaT = 1.52f, ITexture *p_pTexture = NULL)
				: m_reflectivity(p_reflectivity)
				, m_transmittance(p_transmittance)
				, m_fAbsorption(p_fAbsorption)
				, m_fEtaI(p_fEtaI)
				, m_fEtaT(p_fEtaT)
				, m_pTexture(p_pTexture)
			{
				m_bxdfList.push_back(new SpecularTransmission(p_fEtaI, p_fEtaT));
				m_bxdfList.push_back(new SpecularReflection());
			}

			~GlassMaterial(void)
			{
				delete m_bxdfList[1];
				delete m_bxdfList[0];
			}

			void SetTexture(ITexture* p_pTexture)
			{
				m_pTexture = p_pTexture;
			}
			
			Spectrum SampleF(const DifferentialSurface &p_surface, const Vector3 &p_wOut, Vector3 &p_wIn, float p_u, float p_v, float p_w,
				float *p_pdf, BxDF::Type p_bxdfType = BxDF::All_Combined, BxDF::Type *p_sampledBxDFType = NULL)
			{
				return 	BSDF::SampleF(p_surface, p_wOut, p_wIn, p_u, p_v, p_w, p_pdf, p_bxdfType, p_sampledBxDFType);
				
				const static Spectrum one(1.f);

				// int bxdfIndex = 1;
				int bxdfIndex = p_w < 0.5f ?  0 : 1;
				BxDF *pBxDF = m_bxdfList[bxdfIndex];

				Spectrum reflectivity = SampleTexture(p_surface, bxdfIndex);
				Spectrum f = pBxDF->SampleF(reflectivity, p_wOut, p_wIn, p_u, p_v, p_pdf);
				Spectrum fr = Fresnel::EvaluateDielectricTerm(p_wOut.Z, m_fEtaI, m_fEtaT);
				
				*p_pdf *= 0.5f;

				/*
				
				if (!pBxDF->IsType(BxDF::Reflection)) 
					fr = one - fr;
				
				f *= fr;
				
				*/
				/*
				f.Set(Maths::Exp(-p_surface.Distance * 0.15 * f[0]),
					Maths::Exp(-p_surface.Distance * 0.15 * f[1]),
					Maths::Exp(-p_surface.Distance * 0.15 * f[2]));
				*/

				return f;
				
				/* 
				BSDF::SampleF(p_surface, p_wOut, p_wIn, p_u, p_v, p_w, p_pdf, p_bxdfType, p_sampledBxDFType);
				
				const static Spectrum one(1.f);

				Spectrum f = BSDF::SampleF(p_surface, p_wOut, p_wIn, p_u, p_v, p_w, p_pdf, p_bxdfType, p_sampledBxDFType);
				Spectrum fr = EvaluateFresnelTerm(p_wOut.Z, m_fEtaI, m_fEtaT);
				
				if (*p_sampledBxDFType & BxDF::Reflection)
					return f * fr;

				return f * (one - fr);
				*/
				// return BSDF::SampleF(p_surface, p_wOut, p_wIn, p_u, p_v, p_w, p_pdf, p_bxdfType, p_sampledBxDFType);

				/*
				const Spectrum &F = BSDF::SampleF(p_surface, p_wOut, p_wIn, p_u, p_v, p_w, p_pdf, p_bxdfType, p_sampledBxDFType);
				const Spectrum &fresnel = EvaluateFresnelTerm(p_wOut.Z, m_fEtaI, m_fEtaT);

				if (*p_sampledBxDFType & BxDF::Reflection)
					return F * fresnel;

				return F * (Spectrum(1.0f) - fresnel);
				*/
			}
			
			Spectrum SampleTexture(const DifferentialSurface &p_surface, int p_bxdfIndex)
			{
				if (m_pTexture)
				{
					RGBPixel pixel; m_pTexture->GetValue(p_surface.PointUV, p_surface.PointWS, pixel);
					return Spectrum(pixel.R, pixel.G, pixel.B);
				}
				else
				{
					return p_bxdfIndex == 0 ? m_transmittance : m_reflectivity;
				}
			}
		};
	}
}