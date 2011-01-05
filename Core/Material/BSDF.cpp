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
	float phi = Maths::PiTwo * p_u;
	float r = Maths::Sqrt(p_v);
	float x = r * Maths::Cos(phi);
	float y = r * Maths::Sin(phi);
	float z = Maths::Sqrt(1 - x*x - y*y);

	p_out.Set(x, y, z);

	//float cosTheta = Maths::Sqrt(1 - p_u);
	//float sinTheta = Maths::Sqrt(1 - cosTheta * cosTheta);
	//float phi = Maths::PiTwo * p_v;
	//float cosPhi = Maths::Cos(phi);
	//float sinPhi = Maths::Sin(phi);

	//p_out.Set(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta);
}
//----------------------------------------------------------------------------------------------
void BSDF::LocalToSurface(const DifferentialSurface &p_surface, const Vector3 &p_vector, Vector3 &p_out)
{
	p_out.Set(Vector3::Dot(p_surface.GeometryBasisWS.U, p_vector),
			  Vector3::Dot(p_surface.GeometryBasisWS.V, p_vector),
			  Vector3::Dot(p_surface.GeometryBasisWS.W, p_vector));
}
//----------------------------------------------------------------------------------------------
void BSDF::WorldToSurface(const Transformation &p_worldTransform, const DifferentialSurface &p_surface, const Vector3 &p_vector, Vector3 &p_out)
{
	p_out.Set(Vector3::Dot(p_surface.GeometryBasisWS.U, p_vector),
			  Vector3::Dot(p_surface.GeometryBasisWS.V, p_vector),
			  Vector3::Dot(p_surface.GeometryBasisWS.W, p_vector));

	p_out = p_worldTransform.ApplyInverse(p_out);
}
//----------------------------------------------------------------------------------------------
void BSDF::SurfaceToLocal(const DifferentialSurface &p_surface, const Vector3 &p_vector, Vector3 &p_out)
{
	p_out = p_surface.GeometryBasisWS.Project(p_vector);
}
//----------------------------------------------------------------------------------------------
void BSDF::SurfaceToWorld(const Transformation &p_worldTransform, const DifferentialSurface &p_surface, const Vector3 &p_vector, Vector3 &p_out)
{
	p_out = p_surface.GeometryBasisWS.Project(p_worldTransform.Apply(p_vector));
}
//----------------------------------------------------------------------------------------------
Spectrum BSDF::Rho(Vector3 &p_wOut) { return 0.0f; }
//----------------------------------------------------------------------------------------------
Spectrum BSDF::SampleF(const DifferentialSurface &p_surface, const Vector3 &p_wIn, Vector3 &p_wOut, float p_u, float p_v, float* p_pdf) { return 0.0f; }
//----------------------------------------------------------------------------------------------
Spectrum BSDF::F(const DifferentialSurface &p_surface, const Vector3 &p_wIn, const Vector3 &p_wOut) { return 0.0f; }
//----------------------------------------------------------------------------------------------
float BSDF::Pdf(const Vector3 &p_wIn, const Vector3 &p_wOut) { return 1.0f; }
//----------------------------------------------------------------------------------------------
