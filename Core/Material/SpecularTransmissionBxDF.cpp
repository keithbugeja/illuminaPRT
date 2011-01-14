//----------------------------------------------------------------------------------------------
//	Filename:	SpecularTransmissionBxDF.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iostream>

#include "Material/SpecularTransmissionBxDF.h"
#include "Material/BSDF.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
SpecularTransmission::SpecularTransmission(float p_fEtaI, float p_fEtaT) 
	: BxDF(BxDF::Type(BxDF::Transmission | BxDF::Specular))
	, m_fEtaI(p_fEtaI)
	, m_fEtaT(p_fEtaT)
{ }
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
Spectrum SpecularTransmission::Rho(const Spectrum &p_reflectance, const Vector3 &p_wOut, int p_nSampleSize, float *p_pSampleList) 
{ 
	return p_reflectance; 
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
Spectrum SpecularTransmission::SampleF(const Spectrum &p_reflectance, const Vector3 &p_wOut, Vector3 &p_wIn, float p_u, float p_v, float *p_pdf) 
{
	bool bEntering = p_wOut.Z > 0;

	float etaI = m_fEtaI,
		etaT = m_fEtaT;

	if (bEntering) Maths::Swap(etaI, etaT);

	float sinI2 = 1.0f - (p_wOut.Z * p_wOut.Z);
	float eta = etaI / etaT;
	float sinT2 = eta * eta * sinI2;

	if (sinT2 >= 1.0f) return 0.0f;

	float cosT = Maths::Sqrt(Maths::Max(0.0f, 1.0f - sinT2));
	if (bEntering) cosT = -cosT;
	float sinTOverSinI = eta;

	*p_pdf = 1.0f;

	p_wIn.Set(sinTOverSinI * -p_wOut.X, sinTOverSinI * -p_wOut.Y, cosT);

	float et2xei2 = (etaT * etaT) / (etaI * etaI);

	return p_reflectance * et2xei2 / Maths::FAbs(p_wIn.Z);

	//Spectrum r = Spectrum(0.8) / Maths::FAbs(p_wIn.Z);
	//std::cout << "Result : " << r.ToString() << std::endl;
	//return r;
	///*(Spectrum(1.0) - fresnel) * et2xei2*/ 1e-3f / Maths::FAbs(p_wIn.Z);
	//return p_reflectance * (etaT * etaT) / (etaI * etaI) / Maths::FAbs(p_wIn.Z);
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
Spectrum SpecularTransmission::F(const Spectrum &p_reflectance, const Vector3 &p_wOut, const Vector3 &p_wIn) 
{ 
	return 0.0f; 
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
Spectrum SpecularTransmission::F(const Vector3 &p_wOut, const Vector3 &p_wIn) 
{
	return 0.0f;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
float SpecularTransmission::Pdf(const Vector3 &p_wOut, const Vector3 &p_wIn) 
{	
	return 0.0f; 
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
