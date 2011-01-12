//----------------------------------------------------------------------------------------------
//	Filename:	BSDF.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iostream>
#include "Material/BSDF.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
void BSDF::GenerateVectorInHemisphere(float p_u, float p_v, Vector3 &p_out)
{
	//float phi = Maths::PiTwo * p_u;
	//float r = Maths::Sqrt(p_v);
	//float x = r * Maths::Cos(phi);
	//float y = r * Maths::Sin(phi);
	//float z = Maths::Sqrt(1 - x*x - y*y);

	//p_out.Set(x, y, z);

	float cosTheta = Maths::Sqrt(1 - p_u);
	float sinTheta = Maths::Sqrt(1 - cosTheta * cosTheta);
	float phi = Maths::PiTwo * p_v * 2;
	float cosPhi = Maths::Cos(phi);
	float sinPhi = Maths::Sin(phi);

	p_out.Set(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta);
}
//----------------------------------------------------------------------------------------------
void BSDF::LocalToSurface(const DifferentialSurface &p_surface, const Vector3 &p_vector, Vector3 &p_out, bool p_bUseShadingNormals)
{
	if (p_bUseShadingNormals)
	{
		p_out.Set(Vector3::Dot(p_surface.ShadingBasisWS.U, p_vector),
				  Vector3::Dot(p_surface.ShadingBasisWS.V, p_vector),
				  Vector3::Dot(p_surface.ShadingBasisWS.W, p_vector));
	}
	else
	{
		p_out.Set(Vector3::Dot(p_surface.GeometryBasisWS.U, p_vector),
				  Vector3::Dot(p_surface.GeometryBasisWS.V, p_vector),
				  Vector3::Dot(p_surface.GeometryBasisWS.W, p_vector));
	}
}
//----------------------------------------------------------------------------------------------
void BSDF::WorldToSurface(const Transformation &p_worldTransform, const DifferentialSurface &p_surface, const Vector3 &p_vector, Vector3 &p_out, bool p_bUseShadingNormals)
{
	if (p_bUseShadingNormals)
	{
		p_out.Set(Vector3::Dot(p_surface.ShadingBasisWS.U, p_vector),
				  Vector3::Dot(p_surface.ShadingBasisWS.V, p_vector),
				  Vector3::Dot(p_surface.ShadingBasisWS.W, p_vector));
	}
	else
	{
		p_out.Set(Vector3::Dot(p_surface.GeometryBasisWS.U, p_vector),
				  Vector3::Dot(p_surface.GeometryBasisWS.V, p_vector),
				  Vector3::Dot(p_surface.GeometryBasisWS.W, p_vector));
	}

	p_out = p_worldTransform.ApplyInverse(p_out);
}
//----------------------------------------------------------------------------------------------
void BSDF::SurfaceToLocal(const DifferentialSurface &p_surface, const Vector3 &p_vector, Vector3 &p_out, bool p_bUseShadingNormals)
{
	if (p_bUseShadingNormals)
		p_out = p_surface.ShadingBasisWS.Project(p_vector);
	else
		p_out = p_surface.GeometryBasisWS.Project(p_vector);
}
//----------------------------------------------------------------------------------------------
void BSDF::SurfaceToWorld(const Transformation &p_worldTransform, const DifferentialSurface &p_surface, const Vector3 &p_vector, Vector3 &p_out, bool p_bUseShadingNormals)
{
	if (p_bUseShadingNormals)
		p_out = p_surface.ShadingBasisWS.Project(p_worldTransform.Apply(p_vector));
	else
		p_out = p_surface.GeometryBasisWS.Project(p_worldTransform.Apply(p_vector));
}
//----------------------------------------------------------------------------------------------
int BSDF::GetBxDFCount(BxDF::Type p_bxdfType)
{
	if (p_bxdfType == BxDF::All_Combined)
		return m_bxdfList.Size();

	int bxdfCount = 0;

	for (size_t bxdfIndex = 0; bxdfIndex < m_bxdfList.Size(); bxdfIndex++)
		if (m_bxdfList[bxdfIndex]->IsType(p_bxdfType))
			bxdfCount++;

	return bxdfCount;
}
//----------------------------------------------------------------------------------------------
int BSDF::GetBxDF(BxDF::Type p_bxdfType, int p_nBxDFIndex, BxDF **p_pBxDF)
{
	if (p_bxdfType == BxDF::All_Combined)
	{
		*p_pBxDF = m_bxdfList[p_nBxDFIndex];
		return p_nBxDFIndex;
	}

	for (size_t bxdfIndex = 0; bxdfIndex < m_bxdfList.Size(); bxdfIndex++)
	{
		if (m_bxdfList[bxdfIndex]->IsType(p_bxdfType))
		{
			if (p_nBxDFIndex == 0)
			{
				*p_pBxDF = m_bxdfList[bxdfIndex];
				return bxdfIndex;
			}

			p_nBxDFIndex--;
		}
	}

	*p_pBxDF = NULL;
	return -1;
}
//----------------------------------------------------------------------------------------------
Spectrum BSDF::Rho(Vector3 &p_wOut, int p_nSampleCount, float *p_nSampleList, BxDF::Type p_bxdfType)
{
	return 0.0f; 
}
//----------------------------------------------------------------------------------------------
Spectrum BSDF::SampleF(const DifferentialSurface& p_surface, const Vector3 &p_wOut, Vector3 &p_wIn, float p_u, float p_v, float *p_pdf, BxDF::Type p_bxdfType, BxDF::Type *p_sampledBxDFType)
{ 
	int bxdfCount = GetBxDFCount(p_bxdfType);

	if (bxdfCount == 0)
	{
		*p_pdf = 0.0f;
		return 0.0f;
	}

	// Choose a bxdf to sample
	// #pragma message ("Need to pass random number or a way to generate it")
	// Need to get a new random number to remove bias!
	int bxdfIndexFilter = (int)(p_v * bxdfCount - 1),
		bxdfIndexList;

	BxDF *pBxDF;
	
	if ((bxdfIndexList = GetBxDF(p_bxdfType, bxdfIndexFilter, &pBxDF)) == -1)
		throw new Exception("No BxDF found.");

	// Sample chosen bxdf
	Spectrum reflectivity = SampleTexture(p_surface, bxdfIndexList);
	Spectrum f = pBxDF->SampleF(reflectivity, p_wOut, p_wIn, p_u, p_v, p_pdf);

	if (*p_pdf == 0.0f) 
		return 0.0f;
	
	if (p_sampledBxDFType != NULL) 
		*p_sampledBxDFType = pBxDF->GetType();
	
	// Compute PDF with matching BxDFs
	if (bxdfCount > 1)
	{
		if (!pBxDF->IsType(BxDF::Specular)) 
		{
			for (size_t bxdfIndex = 0; bxdfIndex < m_bxdfList.Size(); ++bxdfIndex) 
			{
				BxDF *pBxDF4Pdf = m_bxdfList[bxdfIndex];

				if (pBxDF4Pdf != pBxDF && pBxDF4Pdf->IsType(p_bxdfType))
					*p_pdf +=  pBxDF4Pdf->Pdf(p_wOut, p_wIn);
			}
		}
		
		*p_pdf /= bxdfCount;
	}

	// Compute value of BSDF for sampled direction
	if (!pBxDF->IsType(BxDF::Specular))
	{
		BxDF::Type bxdfFlags;

		f = 0.0;

		if (p_wIn.Z > 0)
			bxdfFlags = BxDF::Type(p_bxdfType & ~BxDF::Transmission);
		else
			bxdfFlags = BxDF::Type(p_bxdfType & ~BxDF::Reflection);

		for (size_t bxdfIndex = 0; bxdfIndex < m_bxdfList.Size(); ++bxdfIndex)
		{
			BxDF *pBxDF4F = m_bxdfList[bxdfIndex];
			
			if (pBxDF4F->IsType(bxdfFlags))
			{
				f += pBxDF4F->F(reflectivity, p_wOut, p_wIn);
			}
		}
	}

	return f;
}
//----------------------------------------------------------------------------------------------
Spectrum BSDF::F(const DifferentialSurface& p_surface, const Vector3 &p_wOut, const Vector3 &p_wIn, BxDF::Type p_bxdfType)
{	
	BxDF::Type bxdfFlags;
	Spectrum f = 0.0;

	if (p_wIn.Z > 0)
		bxdfFlags = BxDF::Type(p_bxdfType & ~BxDF::Transmission);
	else
		bxdfFlags = BxDF::Type(p_bxdfType & ~BxDF::Reflection);

	for (size_t bxdfIndex = 0; bxdfIndex < m_bxdfList.Size(); ++bxdfIndex)
	{
		BxDF *pBxDF4F = m_bxdfList[bxdfIndex];
			
		if (pBxDF4F->IsType(bxdfFlags))
		{
			f += pBxDF4F->F(SampleTexture(p_surface, bxdfIndex), p_wOut, p_wIn);
		}
	}

	return f;
}
//----------------------------------------------------------------------------------------------
float BSDF::Pdf(const Vector3 &p_wIn, const Vector3 &p_wOut, BxDF::Type p_bxdfType) 
{ 
	int bxdfCount = m_bxdfList.Size(),
		bxdfMatchCount = 0;
	
	float pdf = 0;

	if (bxdfCount == 0) 
		return 0;

	for (int bxdfIndex = 0; bxdfIndex < bxdfCount; bxdfIndex++)
	{
		if (m_bxdfList[bxdfIndex]->IsType(p_bxdfType))
		{
			bxdfMatchCount++;
			pdf += m_bxdfList[bxdfIndex]->Pdf(p_wIn, p_wOut);
		}
	}

	return bxdfMatchCount > 0 ? pdf / bxdfMatchCount : 0; 
}
//----------------------------------------------------------------------------------------------
Spectrum BSDF::SampleTexture(const DifferentialSurface &p_surface, int p_nBxDFIndex)
{
	return 1.0;
}
//----------------------------------------------------------------------------------------------
