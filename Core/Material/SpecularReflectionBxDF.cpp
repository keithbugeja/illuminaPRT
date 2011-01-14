//----------------------------------------------------------------------------------------------
//	Filename:	SpecularReflectionBxDF.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iostream>

#include "Material/SpecularReflectionBxDF.h"
#include "Material/BSDF.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
SpecularReflection::SpecularReflection(void) 
	: BxDF(BxDF::Type(BxDF::Reflection | BxDF::Specular))
{ }
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
Spectrum SpecularReflection::Rho(const Spectrum &p_reflectance, const Vector3 &p_wOut, int p_nSampleSize, float *p_pSampleList) 
{ 
	return p_reflectance; 
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
Spectrum SpecularReflection::SampleF(const Spectrum &p_reflectance, const Vector3 &p_wOut, Vector3 &p_wIn, float p_u, float p_v, float *p_pdf) 
{ 
	// Compute perfect specular reflection direction
	p_wIn.Set(-p_wOut.X, -p_wOut.Y, p_wOut.Z);
	*p_pdf = 1.f;

	return p_reflectance / Maths::FAbs(p_wIn.Z);
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
Spectrum SpecularReflection::F(const Spectrum &p_reflectance, const Vector3 &p_wOut, const Vector3 &p_wIn) 
{ 
	return 0.0f; 
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
Spectrum SpecularReflection::F(const Vector3 &p_wOut, const Vector3 &p_wIn) 
{
	return 0.0f;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
float SpecularReflection::Pdf(const Vector3 &p_wOut, const Vector3 &p_wIn) 
{	
	return 0.0f; 
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
