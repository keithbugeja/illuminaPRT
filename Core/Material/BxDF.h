//----------------------------------------------------------------------------------------------
//	Filename:	BxDF.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <iostream>

#include "System/IlluminaPRT.h"
#include "Spectrum/Spectrum.h"
#include "Object/Object.h"

namespace Illumina
{
	namespace Core
	{
		class BxDF 
			: public Object
		{
		public:
			enum Type 
			{
				Reflection				= 1 << 0,
				Transmission			= 1 << 1,
				Diffuse					= 1 << 2,
				Glossy					= 1 << 3,
				Specular				= 1 << 4,
				Combined				= Diffuse | Glossy | Specular,
				Reflective_Specular     = Reflection | Specular,
				Transmissive_Specular	= Transmission | Specular,
				Reflective_Combined		= Reflection | Combined,
				Transmissive_Combined	= Transmission | Combined,
				All_Combined			= Reflective_Combined | Transmissive_Combined
			};
		
		protected:
			BxDF::Type m_bxdfType;

		public:
			BxDF(BxDF::Type p_bxdfType);			
			BxDF(const std::string &p_strName, BxDF::Type p_bxdfType);

			BxDF::Type GetType(void) const;
			bool IsType(BxDF::Type p_bxdfType, bool p_bExactMatch = false) const;

			virtual Spectrum Rho(const Spectrum &p_reflectance, const Vector3 &p_wOut, int p_nSampleSize = 1, float *p_pSampleList = NULL);
			virtual Spectrum SampleF(const Spectrum &p_reflectance, const Vector3 &p_wOut, Vector3 &p_wIn, float p_u, float p_v, float *p_pdf);
			virtual Spectrum F(const Spectrum &p_reflectance, const Vector3 &p_wOut, const Vector3 &p_wIn);
			virtual Spectrum F(const Vector3 &p_wOut, const Vector3 &p_wIn);
			virtual float Pdf(const Vector3 &p_wOut, const Vector3 &p_wIn);
		};

		class Fresnel
		{
		public:
			static Spectrum EvaluateDielectricTerm(float cosi, float etai, float etat)
			{
				cosi = Maths::Clamp(cosi, -1.f, 1.f);

				// Compute indices of refraction for dielectric
				bool entering = cosi > 0.;
				float ei = etai, et = etat;

				if (!entering)
					Maths::Swap(ei, et);

				// Compute _sint_ using Snell's law
				float sint = ei/et * Maths::Sqrt(Maths::Max(0.f, 1.f - cosi*cosi));
				if (sint >= 1.) 
				{
					// Handle total internal reflection
					return 1.;
				}
				else 
				{
					float cost = Maths::Sqrt(Maths::Max(0.f, 1.f - sint*sint));
					return FresnelDielectric(Maths::Abs(cosi), cost, ei, et);
				}
			}

			static Spectrum FresnelDielectric(const float cosi, 
						  const float cost, 
						  const float &etai, 
						  const float &etat)
			{
				float Rparl =	((etat * cosi) - (etai * cost)) /
								((etat * cosi) + (etai * cost));
				float Rperp =	((etai * cosi) - (etat * cost)) /
								((etai * cosi) + (etat * cost));
				
				return (Rparl*Rparl + Rperp*Rperp) / 2.f;
			}
		};
	}
}