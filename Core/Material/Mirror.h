//----------------------------------------------------------------------------------------------
//	Filename:	Mirror.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Material/SpecularReflectionBxDF.h"
#include "Texture/Texture.h"

namespace Illumina
{
	namespace Core
	{
		class MirrorMaterial 
			: public IMaterial
		{
		protected:
			using BSDF::m_bxdfList;

		protected:
			Spectrum m_reflectivity;
			ITexture *m_pDiffuseTexture;

		public:
			MirrorMaterial(const std::string &p_strName, const Spectrum &p_reflectivity)
				: IMaterial(p_strName) 
				, m_reflectivity(p_reflectivity)
				, m_pDiffuseTexture(NULL)
			{
				m_bxdfList.PushBack(new SpecularReflection());
			}

			MirrorMaterial(const Spectrum& p_reflectivity)
				: m_reflectivity(p_reflectivity)
				, m_pDiffuseTexture(NULL)
			{
				m_bxdfList.PushBack(new SpecularReflection());
			}

			~MirrorMaterial(void)
			{
				delete m_bxdfList.At(0);
			}

			void SetDiffuseTexture(ITexture* p_pTexture)
			{
				std::cout << "Setting diffuse texture of " << this->GetName() << " to " << p_pTexture->GetName() << std::endl;
				m_pDiffuseTexture = p_pTexture;
			}

			Spectrum SampleTexture(const DifferentialSurface &p_surface, int p_bxdfIndex)
			{
				if (m_pDiffuseTexture)
				{
					RGBPixel pixel = m_pDiffuseTexture->GetValue(p_surface.PointUV, p_surface.PointWS);
					return Spectrum(pixel.R, pixel.G, pixel.B);
				}
				else
					return m_reflectivity;
			}
		};
	}
}