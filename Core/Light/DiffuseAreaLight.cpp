//----------------------------------------------------------------------------------------------
//	Filename:	DiffuseAreaLight.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iostream>

#include "Light/DiffuseAreaLight.h"
#include "Exception/Exception.h"
#include "Scene/Visibility.h"
#include "Shape/Shape.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
DiffuseAreaLight::DiffuseAreaLight(const std::string &p_strName, Transformation* p_pWorldTransform, IShape* p_pShape, const Spectrum &p_emit)
	: IAreaLight(p_strName, p_pWorldTransform, p_pShape)
{
	m_emit = p_emit;
}
//----------------------------------------------------------------------------------------------
DiffuseAreaLight::DiffuseAreaLight(Transformation* p_pWorldTransform, IShape* p_pShape, const Spectrum &p_emit)
	: IAreaLight(p_pWorldTransform, p_pShape)
{
	m_emit = p_emit;
}
//----------------------------------------------------------------------------------------------
void DiffuseAreaLight::SetShape(IShape *p_pShape)
{
	m_pShape = p_pShape;

	if (p_pShape != NULL) m_fArea = p_pShape->GetArea();
}
//----------------------------------------------------------------------------------------------
float DiffuseAreaLight::Pdf(const Vector3 &p_point, const Vector3 &p_wIn) 
{
	// should get pdf from shape instead!
	return 1.0f / m_pShape->GetArea();
}
//----------------------------------------------------------------------------------------------
Spectrum DiffuseAreaLight::Power(void) 
{
	return m_emit * m_fArea * Maths::Pi;
}
//----------------------------------------------------------------------------------------------
Spectrum DiffuseAreaLight::Radiance(const Vector3 &p_lightSurfacePoint, const Vector3 &p_lightSurfaceNormal, const Vector3 &p_wIn)
{
	//return m_emit; 
	return Vector3::Dot(p_lightSurfaceNormal, p_wIn) > 0 ? m_emit : 0.0f;
}
//----------------------------------------------------------------------------------------------
Spectrum DiffuseAreaLight::SampleRadiance(const Vector3 &p_surfacePoint, double p_u, double p_v, Vector3 &p_wIn, float &p_pdf, VisibilityQuery &p_visibilityQuery)
{
	Vector3 surfaceNormal, 
		surfacePoint;

	surfacePoint = SamplePoint(p_surfacePoint, p_u, p_v, surfaceNormal, p_pdf);

	// Set visibility query for intersection tests
	p_visibilityQuery.SetSegment(p_surfacePoint, 1e-4f, surfacePoint, 1e-4f);

	// Get wOut direction => wIn for radiance reading
	Vector3::Subtract(p_surfacePoint, surfacePoint, p_wIn);
	float distanceSquared = p_wIn.LengthSquared();
	p_wIn.Normalize();

	// Return radiance + part of geometry term
	return (m_emit * Maths::Max(0.0f, Vector3::Dot(p_wIn, surfaceNormal))) / (Pdf(surfacePoint, p_wIn) * distanceSquared);
}
//----------------------------------------------------------------------------------------------
Vector3 DiffuseAreaLight::SamplePoint(const Vector3 &p_viewPoint, double p_u, double p_v, Vector3 &p_lightSurfaceNormal, float &p_pdf)
{
	Vector3 surfaceNormal, 
		surfacePoint;

	// Sample surface point
	if (m_pWorldTransform != NULL && !m_pWorldTransform->IsIdentity())
	{
		// Get point on surface of shape
		const Vector3 &viewPoint = m_pWorldTransform->ApplyInverse(p_viewPoint);
		surfacePoint = m_pShape->SamplePoint(viewPoint, p_u, p_v, surfaceNormal);

		// Get pdf for point
		p_pdf = m_pShape->GetPdf(surfacePoint);

		// Transform into world coordinates
		surfacePoint = m_pWorldTransform->Apply(surfacePoint);
		surfaceNormal = m_pWorldTransform->RotateInverse(surfaceNormal);
	}
	else
	{
		surfacePoint = m_pShape->SamplePoint(p_viewPoint, p_u, p_v, surfaceNormal);
		p_pdf = m_pShape->GetPdf(surfacePoint);
	}

	p_lightSurfaceNormal = surfaceNormal;
	return surfacePoint;
}
//----------------------------------------------------------------------------------------------
Vector3 DiffuseAreaLight::SamplePoint(double p_u, double p_v, Vector3 &p_lightSurfaceNormal, float &p_pdf)
{
	Vector3 surfaceNormal, 
		surfacePoint;

	// Sample surface point
	if (m_pWorldTransform != NULL && !m_pWorldTransform->IsIdentity())
	{
		// Get point on surface of shape
		surfacePoint = m_pShape->SamplePoint(p_u, p_v, surfaceNormal);

		// Get pdf for point
		p_pdf = m_pShape->GetPdf(surfacePoint);

		// Transform into world coordinates
		surfacePoint = m_pWorldTransform->Apply(surfacePoint);
		surfaceNormal = m_pWorldTransform->RotateInverse(surfaceNormal);
	}
	else
	{
		surfacePoint = m_pShape->SamplePoint(p_u, p_v, surfaceNormal);
		p_pdf = m_pShape->GetPdf(surfacePoint);
	}

	p_lightSurfaceNormal = surfaceNormal;
	return surfacePoint;
}
//----------------------------------------------------------------------------------------------
