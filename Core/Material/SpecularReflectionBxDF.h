//----------------------------------------------------------------------------------------------
//	Filename:	SpecularReflectionBxDF.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "Material/BxDF.h"

namespace Illumina
{
	namespace Core
	{
		class SpecularReflection : public BxDF
		{
		public:
			SpecularReflection(void);

			Spectrum Rho(const Spectrum &p_reflectance, const Vector3 &p_wOut, int p_nSampleSize = 1, float *p_pSampleList = NULL);
			Spectrum SampleF(const Spectrum &p_reflectance, const Vector3 &p_wOut, Vector3 &p_wIn, float p_u, float p_v, float *p_pdf);
			Spectrum F(const Spectrum &p_reflectance, const Vector3 &p_wOut, const Vector3 &p_wIn);
			Spectrum F(const Vector3 &p_wOut, const Vector3 &p_wIn);
			float Pdf(const Vector3 &p_wOut, const Vector3 &p_wIn);
		};
	}
}