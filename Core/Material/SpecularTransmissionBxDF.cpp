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

	// Evaluate Fresnel for dielectric
	Spectrum fresnel;

	// Compute Fresnel reflectance for dielectric
	float cosI = Maths::Clamp(cosT, -1.0f, 1.0f);

	// Compute indices of refraction for dielectric
	bEntering = cosI > 0;

	if (!bEntering) Maths::Swap(etaI, etaT);

	// Compute _sint_ using Snell's law
	float sinT = etaI/etaT * Maths::Sqrt(Maths::Max(0.0f, 1.0f - cosI * cosI));

	if (sinT > 1.) 
	{
		fresnel = 1.0f;
	}
	else 
	{
		cosT = Maths::Sqrt(Maths::Max(0.0f, 1.0f - sinT * sinT));

		Spectrum Rparl = ((etaT * cosI) - (etaI * cosT)) /
						 ((etaT * cosI) + (etaI * cosT));
		Spectrum Rperp = ((etaI * cosI) - (etaT * cosT)) /
						 ((etaI * cosI) + (etaT * cosT));

		fresnel = (Rparl * Rparl + Rperp * Rperp) / 2.f;
	}

	return (Spectrum(1.0) - fresnel) * et2xei2 / Maths::FAbs(p_wIn.Z);
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
