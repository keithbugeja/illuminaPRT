//----------------------------------------------------------------------------------------------
//	Filename:	BxDF.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "BxDF.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
BxDF::BxDF(BxDF::Type p_bxdfType) 
	: m_bxdfType(p_bxdfType) 
{ }
//----------------------------------------------------------------------------------------------
BxDF::BxDF(const std::string &p_strName, BxDF::Type p_bxdfType) 
	: Object(p_strName)
	, m_bxdfType(p_bxdfType)
{ }
//----------------------------------------------------------------------------------------------
BxDF::Type BxDF::GetType(void) const { 
	return m_bxdfType; 
}
//----------------------------------------------------------------------------------------------
bool BxDF::IsType(BxDF::Type p_bxdfType, bool p_bExactMatch) const { 
	return p_bExactMatch ? (int)(p_bxdfType & m_bxdfType) == (int)p_bxdfType : (int)(p_bxdfType & m_bxdfType) != 0; 
}
//----------------------------------------------------------------------------------------------
Spectrum BxDF::Rho(const Spectrum &p_reflectance, const Vector3 &p_wOut, int p_nSampleSize, float *p_pSampleList) { return 0.0f; }
//----------------------------------------------------------------------------------------------
Spectrum BxDF::SampleF(const Spectrum &p_reflectance, const Vector3 &p_wOut, Vector3 &p_wIn, float p_u, float p_v, float *p_pdf) { return 0.0f; }
//----------------------------------------------------------------------------------------------
Spectrum BxDF::F(const Spectrum &p_reflectance, const Vector3 &p_wOut, const Vector3 &p_wIn) { return 0.0f; }
//----------------------------------------------------------------------------------------------
Spectrum BxDF::F(const Vector3 &p_wOut, const Vector3 &p_wIn) { return 0.0f; }
//----------------------------------------------------------------------------------------------
float BxDF::Pdf(const Vector3 &p_wOut, const Vector3 &p_wIn) { return 1.0f; }
//----------------------------------------------------------------------------------------------
