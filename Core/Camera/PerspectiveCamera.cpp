//----------------------------------------------------------------------------------------------
//	Filename:	PerspectiveCamera.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Camera/PerspectiveCamera.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
PerspectiveCamera::PerspectiveCamera(const Vector3 &p_centre, const Vector3 &p_direction, const Vector3 &p_up, 
	float p_fLeft, float p_fRight, float p_fBottom, float p_fTop, float p_fDistance)
{
	m_centre = p_centre;
	m_d = p_fDistance;
	m_u0 = p_fLeft;
	m_u1 = p_fRight;
	m_v0 = p_fBottom;
	m_v1 = p_fTop;

	m_uvw.InitFromWV(p_direction, p_up);
	m_corner = m_centre + m_u0 * m_uvw.U + m_v0 * m_uvw.V + m_d * m_uvw.W;
	m_across = (m_u1 - m_u0) * m_uvw.U;
	m_up = (m_v1 - m_v0) * m_uvw.V;
}
//----------------------------------------------------------------------------------------------
PerspectiveCamera::PerspectiveCamera(const std::string &p_strName, const Vector3 &p_centre, const Vector3 &p_direction, const Vector3 &p_up, 
	float p_fLeft, float p_fRight, float p_fBottom, float p_fTop, float p_fDistance)
	: ICamera(p_strName)
{
	m_centre = p_centre;
	m_d = p_fDistance;
	m_u0 = p_fLeft;
	m_u1 = p_fRight;
	m_v0 = p_fBottom;
	m_v1 = p_fTop;

	m_uvw.InitFromWV(p_direction, p_up);
	m_corner = m_centre + m_u0 * m_uvw.U + m_v0 * m_uvw.V + m_d * m_uvw.W;
	m_across = (m_u1 - m_u0) * m_uvw.U;
	m_up = (m_v1 - m_v0) * m_uvw.V;
}
//----------------------------------------------------------------------------------------------
Ray PerspectiveCamera::GetRay(float p_fPixelX, float p_fPixelY, float p_fXi1, float p_fXi2) const 
{
	Vector3 origin = m_centre;
	
	Vector3 target = m_corner + 
		m_across * p_fPixelX + 
		m_up * p_fPixelY;

	return Ray(origin, Vector3::Normalize(target - origin));
}
//----------------------------------------------------------------------------------------------
void PerspectiveCamera::GetRay(float p_fPixelX, float p_fPixelY, float p_fXi1, float p_fXi2, Ray &p_ray) const 
{
	Vector3 target = m_corner + 
		m_across * p_fPixelX + 
		m_up * p_fPixelY;

	p_ray.Origin = m_centre;
	p_ray.SetDirection(m_corner + Vector3::Normalize(target - m_centre));
	
	//p_ray.Direction = m_corner + Vector3::Normalize(target - m_centre);
	//Vector3::Inverse(p_ray.Direction, p_ray.DirectionInverseCache);
}
//----------------------------------------------------------------------------------------------
