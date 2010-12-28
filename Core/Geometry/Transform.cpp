//----------------------------------------------------------------------------------------------
//	Filename:	Transformatin.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Description goes here
//----------------------------------------------------------------------------------------------
#include "Geometry/Transform.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
Transformation::Transformation(void)
{
	m_transform = Matrix4x4::Identity;
	m_rotation = Matrix3x3::Identity;
	m_rotationInverse = Matrix3x3::Identity;
	m_translation = Vector3::Zero;
	m_scaling = Vector3::Ones;

	m_hasRotation = 
		m_hasScaling = 
		m_hasTranslation = false;
	
	m_isIdentity = true;
}
//----------------------------------------------------------------------------------------------
Transformation::Transformation(const Matrix3x3 &p_rotation, const Vector3 &p_scaling, const Vector3 &p_translation)
{
	m_rotation = p_rotation;
	Matrix3x3::Transpose(m_rotation, m_rotationInverse);

	m_transform = Matrix4x4::Identity;
	m_translation = p_translation;
	m_scaling = p_scaling;

	m_hasRotation = !m_rotation.IsIdentity();
	m_hasTranslation = !m_translation.IsZero();
	m_hasScaling = !m_scaling.IsOnes();
	m_isIdentity = !(m_hasRotation || m_hasScaling || m_hasTranslation);
}
//----------------------------------------------------------------------------------------------
void Transformation::Reset(void)
{
	m_transform = Matrix4x4::Identity;
	m_rotation = Matrix3x3::Identity;
	m_rotationInverse = Matrix3x3::Identity;
	m_translation = Vector3::Zero;
	m_scaling = Vector3::Ones;

	m_hasRotation = 
		m_hasScaling = 
		m_hasTranslation = false;
	
	m_isIdentity = true;
}
//----------------------------------------------------------------------------------------------
Vector3 Transformation::Apply(const Vector3 &p_vector) const
{
	Vector3 result;
	Transformation::Apply(p_vector, result);
	return result;
}
//----------------------------------------------------------------------------------------------
Vector3 Transformation::ApplyInverse(const Vector3 &p_vector) const
{
	Vector3 result;
	Transformation::ApplyInverse(p_vector, result);
	return result;
}
//----------------------------------------------------------------------------------------------
Vector3 Transformation::Scale(const Vector3 &p_vector) const
{
	Vector3 result;
	Transformation::Scale(p_vector, result);
	return result;
}
//----------------------------------------------------------------------------------------------
Vector3 Transformation::ScaleInverse(const Vector3 &p_vector) const
{
	Vector3 result;
	Transformation::ScaleInverse(p_vector, result);
	return result;
}
//----------------------------------------------------------------------------------------------
Vector3 Transformation::Rotate(const Vector3 &p_vector) const
{
	Vector3 result;
	Transformation::Rotate(p_vector, result);
	return result;
}
//----------------------------------------------------------------------------------------------
Vector3 Transformation::RotateInverse(const Vector3 &p_vector) const
{
	Vector3 result;
	Transformation::RotateInverse(p_vector, result);
	return result;
}
//----------------------------------------------------------------------------------------------
Vector3 Transformation::Translate(const Vector3 &p_vector) const
{
	Vector3 result;
	Transformation::Translate(p_vector, result);
	return result;
}
//----------------------------------------------------------------------------------------------
void Transformation::Apply(const Vector3 &p_vector, Vector3 &p_out) const
{
	p_out = p_vector;

	if (m_hasScaling)
		p_out *= m_scaling;
		
	if (m_hasRotation)
		Matrix3x3::Product(m_rotation, p_out, p_out);

	if (m_hasTranslation)
		p_out += m_translation;
}
//----------------------------------------------------------------------------------------------
void Transformation::ApplyInverse(const Vector3 &p_vector, Vector3 &p_out) const
{
	Vector3::Subtract(p_vector, m_translation, p_out);
	Matrix3x3::Product(m_rotationInverse, p_out, p_out);

	float yz = m_scaling.Y * m_scaling.Z,
		xz = m_scaling.X * m_scaling.Z,
		xy = m_scaling.X * m_scaling.Y,
		ixyz = 1 / (xy * m_scaling.Z);

	p_out.X *= (ixyz * yz);
	p_out.Y *= (ixyz * xz);
	p_out.Z *= (ixyz * xy);
}
//----------------------------------------------------------------------------------------------
void Transformation::Scale(const Vector3 &p_vector, Vector3 &p_out) const
{
	if (m_hasScaling)
		p_out.Set(p_vector.X * m_scaling.X, p_vector.Y * m_scaling.Y, p_vector.Z * m_scaling.Z);
	else
		p_out = p_vector;
}
//----------------------------------------------------------------------------------------------
void Transformation::ScaleInverse(const Vector3 &p_vector, Vector3 &p_out) const
{
	if (m_hasScaling)
		p_out.Set(p_vector.X / m_scaling.X, p_vector.Y / m_scaling.Y, p_vector.Z / m_scaling.Z);
	else
		p_out = p_vector;
}
//----------------------------------------------------------------------------------------------
void Transformation::Rotate(const Vector3 &p_vector, Vector3 &p_out) const
{
	p_out = p_vector;

	if (m_hasScaling)
	{
		p_out = m_scaling * p_out;

		if (m_hasRotation)
			p_out = m_rotation * p_out;
	}
	else
	{
		if (m_hasRotation)
			p_out = m_rotation * p_out;
	}
}
//----------------------------------------------------------------------------------------------
void Transformation::RotateInverse(const Vector3 &p_vector, Vector3 &p_out) const
{
	Matrix3x3::Product(m_rotationInverse, p_vector, p_out);
}
//----------------------------------------------------------------------------------------------
void Transformation::Translate(const Vector3 &p_vector, Vector3 &p_out) const
{
	if (m_hasTranslation)
		Vector3::Add(p_vector, m_translation, p_out);
	else
		p_out = p_vector;
}
//----------------------------------------------------------------------------------------------
Matrix3x3 Transformation::GetRotationInverse(void) const {
	return m_rotationInverse;
}
//----------------------------------------------------------------------------------------------
void Transformation::GetRotationInverse(Matrix3x3 &p_out) const
{
	p_out = m_rotationInverse;
}
//----------------------------------------------------------------------------------------------
Matrix3x3 Transformation::GetRotation(void) const {
	return m_rotation;
}
//----------------------------------------------------------------------------------------------
void Transformation::GetRotation(Matrix3x3 &p_out) const
{
	p_out = m_rotation;
}
//----------------------------------------------------------------------------------------------
void Transformation::SetRotation(const Matrix3x3& p_rotation)
{
	// Work out rotation and inverse
	m_rotation = p_rotation;
	Matrix3x3::Transpose(m_rotation, m_rotationInverse);

	// Transform is not longer an identity
	m_hasRotation = true;
	m_isIdentity = false;
} 
//----------------------------------------------------------------------------------------------
Vector3 Transformation::GetTranslation(void) const {
	return m_translation; 
}
//----------------------------------------------------------------------------------------------
void Transformation::GetTranslation(Vector3 &p_out) const
{
	p_out = m_translation; 
}
//----------------------------------------------------------------------------------------------
void Transformation::SetTranslation(const Vector3 &p_translation)
{
	// Translation component of transform
	m_translation = p_translation;

	// Transform is no longer an identity
	m_hasTranslation = true;
	m_isIdentity = false;
}
//----------------------------------------------------------------------------------------------
Vector3 Transformation::GetScaling(void) const {
	return m_scaling;
}
//----------------------------------------------------------------------------------------------
void Transformation::GetScaling(Vector3 &p_out) const
{
	p_out = m_scaling;
}
//----------------------------------------------------------------------------------------------
void Transformation::SetScaling(const Vector3 &p_scaling)
{ 
	// Scaling component 
	m_scaling = p_scaling;

	// Transform is no longer an identity
	m_hasScaling = true;
	m_isIdentity = false;
}
//----------------------------------------------------------------------------------------------
Matrix4x4 Transformation::GetTransform(void)
{
	UpdateTransform();
	return m_transform; 
}
//----------------------------------------------------------------------------------------------
void Transformation::GetTransform(Matrix4x4 &p_out)
{
	UpdateTransform();
	p_out = m_transform; 
}
//----------------------------------------------------------------------------------------------
void Transformation::UpdateTransform(void)
{
	m_transform._00 = m_rotation._00 * m_scaling[0];
	m_transform._01 = m_rotation._01;
	m_transform._02 = m_rotation._02;

	m_transform._10 = m_rotation._10;
	m_transform._11 = m_rotation._11 * m_scaling[1];
	m_transform._12 = m_rotation._12;

	m_transform._20 = m_rotation._20;
	m_transform._21 = m_rotation._21;
	m_transform._22 = m_rotation._22 * m_scaling[2];

	m_transform._03 = m_translation[0];
	m_transform._13 = m_translation[1];
	m_transform._23 = m_translation[2];
	
	m_transform._33 = 1.0f;
}
//----------------------------------------------------------------------------------------------
