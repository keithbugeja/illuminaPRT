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
float DiffuseAreaLight::Pdf(const Vector3 &p_point, const Vector3 &p_wIn) {
	return 1.0f / m_pShape->GetArea(); //Maths::InvPi;
}
//----------------------------------------------------------------------------------------------
Spectrum DiffuseAreaLight::Power(void) 
{
	return m_emit * m_fArea * Maths::Pi;
}
//----------------------------------------------------------------------------------------------
Spectrum DiffuseAreaLight::Radiance(const Vector3 &p_point, const Vector3 &p_normal, const Vector3 &p_wIn)
{
	return m_emit; 
	//return Vector3::Dot(p_normal, p_wIn) < 0 ? m_emit : 0.0f;
}
//----------------------------------------------------------------------------------------------
Spectrum DiffuseAreaLight::SampleRadiance(const Vector3 &p_point, double p_u, double p_v, Vector3& p_wIn, float &p_pdf, VisibilityQuery &p_visibilityQuery)
{
	Vector3 surfaceNormal, 
		surfacePoint;

	// Sample surface point
	if (m_pWorldTransform != NULL && !m_pWorldTransform->IsIdentity())
	{
		const Vector3 &viewPoint = m_pWorldTransform->ApplyInverse(p_point);
		surfacePoint = m_pWorldTransform->Apply(m_pShape->SamplePoint(viewPoint, p_u, p_v, surfaceNormal));
		surfaceNormal = m_pWorldTransform->RotateInverse(surfaceNormal);
	}
	else
	{
		surfacePoint = m_pShape->SamplePoint(p_point, p_u, p_v, surfaceNormal);
	}

	// Set visibility query for intersection tests
	p_visibilityQuery.SetSegment(p_point, 1e-4f, surfacePoint, 1e-4f);

	// Get wOut direction => wIn for radiance reading
	Vector3::Subtract(p_point, surfacePoint, p_wIn);
	float distanceSquared = p_wIn.LengthSquared();
	p_wIn.Normalize();

	// Return radiance + part of geometry term
	return (m_emit * Maths::Max(0.0f, Vector3::Dot(p_wIn, surfaceNormal))) / (Pdf(surfacePoint, p_wIn) * distanceSquared);
}
//----------------------------------------------------------------------------------------------
Spectrum DiffuseAreaLight::SampleRadiance(double p_u, double p_v, Vector3 &p_point, Vector3 &p_normal, float &p_pdf)
{
	Vector3 surfaceNormal, 
		surfacePoint;

	// Sample surface point
	if (m_pWorldTransform != NULL && !m_pWorldTransform->IsIdentity())
	{
		p_point = m_pWorldTransform->Apply(m_pShape->SamplePoint(p_u, p_v, surfaceNormal));
		p_normal = m_pWorldTransform->RotateInverse(surfaceNormal);
	}
	else
	{
		p_point = m_pShape->SamplePoint(p_u, p_v, p_normal);
	}

	p_pdf = 1.0f;

	return m_emit;
}
//----------------------------------------------------------------------------------------------
