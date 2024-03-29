//----------------------------------------------------------------------------------------------
//	Filename:	Camera.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Camera/Camera.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
ICamera::ICamera(void)
	: Object()
{ }
//----------------------------------------------------------------------------------------------
ICamera::ICamera(const std::string &p_strName)
	: Object(p_strName)
{ }
//----------------------------------------------------------------------------------------------
void ICamera::SetProjection(float p_fLeft, float p_fRight, float p_fTop, float p_fBottom, float p_fDistance)
{
	m_d = p_fDistance;
	m_u0 = p_fLeft; m_u1 = p_fRight;
	m_v0 = p_fBottom; m_v1 = p_fTop;

	m_corner = m_centre + m_u0 * m_uvw.U + m_v0 * m_uvw.V + m_d * m_uvw.W;
	m_across = (m_u1 - m_u0) * m_uvw.U;
	m_up = (m_v1 - m_v0) * m_uvw.V;
}
//----------------------------------------------------------------------------------------------
void ICamera::SetFieldOfView(float p_fDegrees, float p_fAspectRatio)
{
	m_fov = Maths::DegToRad(p_fDegrees) * 0.5f;
	m_aspect = p_fAspectRatio;
				
	m_v1 = Maths::Tan(m_fov);
	m_v0 = -m_v1;

	m_u0 = m_v0 * m_aspect;
	m_u1 = m_v1 * m_aspect;

	m_corner = m_centre + m_u0 * m_uvw.U + m_v0 * m_uvw.V + m_d * m_uvw.W;
	m_across = (m_u1 - m_u0) * m_uvw.U;
	m_up = (m_v1 - m_v0) * m_uvw.V;
}
//----------------------------------------------------------------------------------------------
void ICamera::GetFieldOfView(float *p_pfDegrees, float *p_pfAspectRatio) const
{
	*p_pfDegrees = Maths::RadToDeg(m_fov) * 2.f;
	*p_pfAspectRatio = m_aspect;
}
//----------------------------------------------------------------------------------------------
float ICamera::GetFieldOfView(void) const
{
	return Maths::RadToDeg(m_fov);
}
//----------------------------------------------------------------------------------------------
float ICamera::GetAspectRatio(void) const
{
	return m_aspect;
}
//----------------------------------------------------------------------------------------------
void ICamera::Move(const Vector3 &p_displacement) {
	MoveTo(m_centre + p_displacement);
}
//----------------------------------------------------------------------------------------------
void ICamera::MoveTo(const Vector3 &p_position)
{
	m_centre = p_position;
	m_corner = m_centre + m_u0 * m_uvw.U + m_v0 * m_uvw.V + m_d * m_uvw.W;
}
//----------------------------------------------------------------------------------------------
void ICamera::Look(const Vector3 &p_direction, const Vector3 &p_up)
{
	m_uvw.InitFromWV(p_direction, p_up);
	m_corner = m_centre + m_u0 * m_uvw.U + m_v0 * m_uvw.V + m_d * m_uvw.W;
	m_across = (m_u1 - m_u0) * m_uvw.U;
	m_up = (m_v1 - m_v0) * m_uvw.V;
}
//----------------------------------------------------------------------------------------------
void ICamera::LookAt(const Vector3 &p_target) {
	Look(Vector3::Normalize(p_target - m_centre), Vector3::UnitYPos);
}
//----------------------------------------------------------------------------------------------
Vector3 ICamera::GetObserver(void) const { 
	return m_centre; 
}
//----------------------------------------------------------------------------------------------
OrthonormalBasis ICamera::GetFrame(void) const {
	return m_uvw;
}
