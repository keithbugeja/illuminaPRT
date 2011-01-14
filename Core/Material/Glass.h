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
			Spectrum m_reflectivity;
			
			float m_fAbsorption;
			
			float m_fEtaI,
				m_fEtaT;

			ITexture *m_pTexture;

		public:
			GlassMaterial(const std::string &p_strName, const Spectrum &p_reflectivity, float p_fAbsorption = 1.0f, float p_fEtaI = 1.0f, float p_fEtaT = 1.52f, ITexture *p_pTexture = NULL)
				: IMaterial(p_strName) 
				, m_reflectivity(p_reflectivity)
				, m_fAbsorption(p_fAbsorption)
				, m_fEtaI(p_fEtaI)
				, m_fEtaT(p_fEtaT)
				, m_pTexture(p_pTexture)
			{
				m_bxdfList.PushBack(new SpecularReflection());
				//m_bxdfList.PushBack(new SpecularTransmission(m_fEtaI, m_fEtaT));
				m_bxdfList.PushBack(new SpecularTransmission(p_fEtaI, p_fEtaT));
			}

			GlassMaterial(const Spectrum& p_reflectivity, float p_fAbsorption = 1.0f, float p_fEtaI = 1.0f, float p_fEtaT = 1.52f, ITexture *p_pTexture = NULL)
				: m_reflectivity(p_reflectivity)
				, m_fAbsorption(p_fAbsorption)
				, m_fEtaI(p_fEtaI)
				, m_fEtaT(p_fEtaT)
				, m_pTexture(p_pTexture)
			{
				m_bxdfList.PushBack(new SpecularReflection());
				//m_bxdfList.PushBack(new SpecularTransmission(m_fEtaI, m_fEtaT));
				m_bxdfList.PushBack(new SpecularTransmission(p_fEtaI, p_fEtaT));
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

			Spectrum SampleTexture(const DifferentialSurface &p_surface, int p_bxdfIndex)
			{
				if (m_pTexture)
				{
					RGBPixel pixel = m_pTexture->GetValue(p_surface.PointUV, p_surface.PointWS);
					return Spectrum(pixel.R, pixel.G, pixel.B);
				}
				else
					return p_bxdfIndex == 0 ? m_reflectivity : Spectrum(1.0f) - m_reflectivity;

			}
		};
	}
}