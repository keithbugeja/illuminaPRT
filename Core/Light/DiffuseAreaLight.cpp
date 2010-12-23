//----------------------------------------------------------------------------------------------
//	Filename:	DiffuseAreaLight.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Exception/Exception.h"
#include "Light/DiffuseAreaLight.h"
#include "Staging/Visibility.h"
#include "Shape/Shape.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
DiffuseAreaLight::DiffuseAreaLight(
	Transformation* p_pWorldTransform, 
	IShape* p_pShape, 
	const Spectrum &p_emit)
{
	m_emit = p_emit;

	SetWorldTransform(p_pWorldTransform);
	SetShape(p_pShape);
}
//----------------------------------------------------------------------------------------------
void DiffuseAreaLight::SetShape(IShape *p_pShape)
{
	m_pShape = p_pShape;

	if (p_pShape != NULL)
	{
		m_fArea = p_pShape->GetArea();
	}
}
//----------------------------------------------------------------------------------------------
Spectrum DiffuseAreaLight::Power(void) 
{
	return m_emit * m_fArea * Maths::Pi;
}
//----------------------------------------------------------------------------------------------
Spectrum DiffuseAreaLight::Radiance(const Vector3 &p_point, Vector3 &p_wOut, VisibilityQuery &p_visibilityQuery)
{
	throw new Exception("Should call radiance function providing surface sample coordinates!");
}
//----------------------------------------------------------------------------------------------
Spectrum DiffuseAreaLight::Radiance(const Vector3 &p_point, double p_u, double p_v, Vector3& p_wOut, VisibilityQuery &p_visibilityQuery)
{
	Vector3 normal,
		surfacePoint;

	if (m_pWorldTransform != NULL)
	{
		const Vector3 &viewPoint = m_pWorldTransform->ApplyInverse(p_point);
		surfacePoint = m_pWorldTransform->Apply(m_pShape->SamplePoint(viewPoint, p_u, p_v, normal));
	}
	else
	{
		surfacePoint = m_pShape->SamplePoint(p_point, p_u, p_v, normal);
	}

	p_visibilityQuery.SetSegment(p_point, 0.0001f, surfacePoint, 0.0001f);

	Vector3::Subtract(surfacePoint, p_point, p_wOut);
	double distanceSquared = p_wOut.LengthSquared();
	p_wOut.Normalize();

	return m_emit / (4 * Maths::Pi * distanceSquared);
}
//----------------------------------------------------------------------------------------------
