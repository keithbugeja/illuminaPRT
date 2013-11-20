//----------------------------------------------------------------------------------------------
//	Filename:	Material.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Material/LambertianBxDF.h"
#include "Texture/Texture.h"

namespace Illumina
{
	namespace Core
	{
		class MatteMaterial 
			: public IMaterial
		{
		protected:
			using BSDF::m_bxdfList;

		protected:
			Spectrum m_reflectivity;
			ITexture *m_pTexture;

		public:
			MatteMaterial(const std::string &p_strName, const Spectrum &p_reflectivity, ITexture *p_pTexture = NULL)
				: IMaterial(p_strName) 
				, m_reflectivity(p_reflectivity)
				, m_pTexture(p_pTexture)
			{
				//m_bxdfList.PushBack(new Lambertian());
				m_bxdfList.push_back(new Lambertian());
			}

			MatteMaterial(const Spectrum& p_reflectivity, ITexture *p_pTexture = NULL)
				: m_reflectivity(p_reflectivity)
				, m_pTexture(p_pTexture)
			{
				//m_bxdfList.PushBack(new Lambertian());
				m_bxdfList.push_back(new Lambertian());
			}

			~MatteMaterial(void)
			{
				//delete m_bxdfList.At(0);
				delete m_bxdfList[0];
			}

			void SetTexture(ITexture* p_pTexture)
			{
				m_pTexture = p_pTexture;
			}

			Spectrum SampleTexture(const DifferentialSurface &p_surface, int p_bxdfIndex)
			{
				if (m_pTexture)
				{
					RGBPixel pixel; m_pTexture->GetValue(p_surface.PointUV, p_surface.PointWS, pixel);
					return Spectrum(pixel.R, pixel.G, pixel.B);
				}
				else
					return m_reflectivity;
			}
		};
	}
}