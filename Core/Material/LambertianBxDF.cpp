//----------------------------------------------------------------------------------------------
//	Filename:	LambertianBxDF.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iostream>

#include "Material/LambertianBxDF.h"
#include "Material/BSDF.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
Lambertian::Lambertian(void) 
	: BxDF(BxDF::Type(BxDF::Reflection | BxDF::Diffuse))
{ }
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
Spectrum Lambertian::Rho(const Spectrum &p_reflectance, const Vector3 &p_wOut, int p_nSampleSize, float *p_pSampleList) 
{ 
	return p_reflectance; 
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
Spectrum Lambertian::SampleF(const Spectrum &p_reflectance, const Vector3 &p_wOut, Vector3 &p_wIn, float p_u, float p_v, float *p_pdf) 
{ 
	BSDF::GenerateVectorInHemisphere(p_u, p_v, p_wIn);
				
	// Allahares nidghi, ghax kieku shittha Malta
	if (Maths::ISgn(p_wIn.Z) == Maths::ISgn(p_wOut.Z))
		p_wIn.Z = -p_wIn.Z;

	*p_pdf = Maths::InvPi;

	return p_reflectance * Maths::InvPi;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
Spectrum Lambertian::F(const Spectrum &p_reflectance, const Vector3 &p_wOut, const Vector3 &p_wIn) 
{ 
	return p_reflectance * Maths::InvPi; 
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
Spectrum Lambertian::F(const Vector3 &p_wOut, const Vector3 &p_wIn) 
{
	return Maths::InvPi;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
float Lambertian::Pdf(const Vector3 &p_wOut, const Vector3 &p_wIn) 
{	
	return 1.0f; 
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------