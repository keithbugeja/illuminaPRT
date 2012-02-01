//----------------------------------------------------------------------------------------------
//	Filename:	AxisAlignedBoundingBox.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <float.h>
#include <math.h>

#include <iostream>

#include "Geometry/BoundingBox.h"
#include "Geometry/Ray.h"
#include "Geometry/Plane.h"
#include "Geometry/Interval.h"
#include "Geometry/Transform.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
static const float flt_plus_inf = -logf(0);	// let's keep C and C++ compilers happy.
static const float ALIGN_16
	ps_cst_plus_inf[4]	= {  flt_plus_inf,  flt_plus_inf,  flt_plus_inf,  flt_plus_inf },
	ps_cst_minus_inf[4]	= { -flt_plus_inf, -flt_plus_inf, -flt_plus_inf, -flt_plus_inf }; 
//----------------------------------------------------------------------------------------------
AxisAlignedBoundingBox::AxisAlignedBoundingBox(void)
{ }
//----------------------------------------------------------------------------------------------
AxisAlignedBoundingBox::AxisAlignedBoundingBox(const Vector3 &p_minExtent,
											   const Vector3 &p_maxExtent)
{
	m_minExtent = Vector3::Min(p_maxExtent, p_minExtent);
	m_maxExtent = Vector3::Max(p_maxExtent, p_minExtent);

	Update();
}
//----------------------------------------------------------------------------------------------
AxisAlignedBoundingBox::AxisAlignedBoundingBox(const AxisAlignedBoundingBox &p_aabb)
	: m_fRadius(p_aabb.m_fRadius)
	, m_minExtent(p_aabb.m_minExtent)
	, m_maxExtent(p_aabb.m_maxExtent)
	, m_centre(p_aabb.m_centre)
	, m_extent(p_aabb.m_extent)
{
}
//----------------------------------------------------------------------------------------------
BoundingVolumeType AxisAlignedBoundingBox::GetType(void) const {
	return AxisAlignedBox;
}
//----------------------------------------------------------------------------------------------
boost::shared_ptr<IBoundingVolume> AxisAlignedBoundingBox::Clone(void) const {
	return boost::shared_ptr<IBoundingVolume>(new AxisAlignedBoundingBox(*this));
}
//----------------------------------------------------------------------------------------------
boost::shared_ptr<IBoundingVolume> AxisAlignedBoundingBox::CreateInstance(void) const {
	return Clone();
}
//----------------------------------------------------------------------------------------------
void AxisAlignedBoundingBox::Invalidate(void)
{
	m_minExtent.Set(Maths::Maximum, Maths::Maximum, Maths::Maximum);
	m_maxExtent.Set(-Maths::Maximum, -Maths::Maximum, -Maths::Maximum);
}
//----------------------------------------------------------------------------------------------
void AxisAlignedBoundingBox::ComputeFromPoints(const Vector3List &p_pointList)
{
	m_minExtent.Set(Maths::Maximum, Maths::Maximum, Maths::Maximum);
	m_maxExtent.Set(-Maths::Maximum, -Maths::Maximum, -Maths::Maximum);

	Vector3 point;

	for (int idx = 0, count = (int)p_pointList.Size(); idx < count; idx++)
	{
		point = p_pointList.At(idx);

		m_minExtent = Vector3::Min(m_minExtent, point);
		m_maxExtent = Vector3::Max(m_maxExtent, point);
	}

	Update();
}
//----------------------------------------------------------------------------------------------
void AxisAlignedBoundingBox::ComputeFromPoints(const Vector3 *p_pointList, int p_nCount)
{
	m_minExtent.Set(Maths::Maximum, Maths::Maximum, Maths::Maximum);
	m_maxExtent.Set(-Maths::Maximum, -Maths::Maximum, -Maths::Maximum);

	Vector3 *pointList = (Vector3*)p_pointList;

	for (int index = 0; index < p_nCount; index++, pointList++)
	{
		m_minExtent = Vector3::Min(m_minExtent, *pointList);
		m_maxExtent = Vector3::Max(m_maxExtent, *pointList);
	}

	Update();
}
//----------------------------------------------------------------------------------------------
void AxisAlignedBoundingBox::ComputeFromVolume(const IBoundingVolume &p_volume)
{
	// We only support AABBs so far, so assert type is correct
	BOOST_ASSERT(p_volume.GetType() == AxisAlignedBox);

	*this = *((AxisAlignedBoundingBox*)&p_volume);
}
//----------------------------------------------------------------------------------------------
void AxisAlignedBoundingBox::ComputeFromVolume(const Transformation &p_transformation, const IBoundingVolume &p_volume)
{
	// We only support AABBs so far, so assert type is correct
	BOOST_ASSERT(p_volume.GetType() == AxisAlignedBox);

	*this = *((AxisAlignedBoundingBox*)&p_volume);
	Apply(p_transformation);
}
//----------------------------------------------------------------------------------------------
boost::shared_ptr<IBoundingVolume> AxisAlignedBoundingBox::Apply(const Transformation &p_transformation) const
{
	boost::shared_ptr<AxisAlignedBoundingBox> aabb(new AxisAlignedBoundingBox());
	Apply(p_transformation, *aabb);
	return aabb;
}
//----------------------------------------------------------------------------------------------
void AxisAlignedBoundingBox::Apply(const Transformation &p_transformation, IBoundingVolume &p_out) const
{
	// We only support AABBs so far, so assert type is correct
	BOOST_ASSERT(p_out.GetType() == AxisAlignedBox);

	const Matrix3x3 &rotation = p_transformation.GetRotation();
	const Vector3 &centre = p_transformation.Apply(m_centre);
	const Vector3 extent = m_extent;

	Vector3 halfSize(
		Maths::Abs(rotation._00) * extent.X + Maths::Abs(rotation._01) * extent.Y + Maths::Abs(rotation._02) * extent.Z,
		Maths::Abs(rotation._10) * extent.X + Maths::Abs(rotation._11) * extent.Y + Maths::Abs(rotation._12) * extent.Z,
		Maths::Abs(rotation._20) * extent.X + Maths::Abs(rotation._21) * extent.Y + Maths::Abs(rotation._22) * extent.Z
	);

	if (p_transformation.HasScaling())
		p_transformation.Scale(halfSize, halfSize);

	AxisAlignedBoundingBox &aabb = (AxisAlignedBoundingBox&)p_out;

	Vector3::Subtract(centre, halfSize, aabb.m_minExtent);
	Vector3::Add(centre, halfSize, aabb.m_maxExtent);

	aabb.Update();
}
//----------------------------------------------------------------------------------------------
bool AxisAlignedBoundingBox::Intersects(const Ray &p_ray) const
{
	float fIn = 0, fOut = 0;
	return Intersects(p_ray, fIn, fOut);
}
//----------------------------------------------------------------------------------------------
bool AxisAlignedBoundingBox::Intersects(const Ray &p_ray, float &p_hitIn, float &p_hitOut) const
{
	/*
	// you may already have those values hanging around somewhere
	const __m128
		plus_inf	= loadps(ps_cst_plus_inf),
		minus_inf	= loadps(ps_cst_minus_inf);

	// use whatever's apropriate to load.
	const __m128
		box_min	= loadps(&m_minExtent),
		box_max	= loadps(&m_maxExtent),
		pos	= loadps(&p_ray.Origin),
		inv_dir	= loadps(&p_ray.DirectionInverseCache);

	// use a div if inverted directions aren't available
	const __m128 l1 = mulps(subps(box_min, pos), inv_dir);
	const __m128 l2 = mulps(subps(box_max, pos), inv_dir);

	// the order we use for those min/max is vital to filter out
	// NaNs that happens when an inv_dir is +/- inf and
	// (box_min - pos) is 0. inf * 0 = NaN
	const __m128 filtered_l1a = minps(l1, plus_inf);
	const __m128 filtered_l2a = minps(l2, plus_inf);

	const __m128 filtered_l1b = maxps(l1, minus_inf);
	const __m128 filtered_l2b = maxps(l2, minus_inf);

	// now that we're back on our feet, test those slabs.
	__m128 lmax = maxps(filtered_l1a, filtered_l2a);
	__m128 lmin = minps(filtered_l1b, filtered_l2b);

	// unfold back. try to hide the latency of the shufps & co.
	const __m128 lmax0 = rotatelps(lmax);
	const __m128 lmin0 = rotatelps(lmin);
	lmax = minss(lmax, lmax0);
	lmin = maxss(lmin, lmin0);

	const __m128 lmax1 = muxhps(lmax,lmax);
	const __m128 lmin1 = muxhps(lmin,lmin);
	lmax = minss(lmax, lmax1);
	lmin = maxss(lmin, lmin1);

	const bool ret = _mm_comige_ss(lmax, _mm_setzero_ps()) & _mm_comige_ss(lmax,lmin);

	storess(lmin, &p_hitIn);
	storess(lmax, &p_hitOut);

	return  ret;
	*/

	/**/
	// Implementation from http://ompf.org/ray/ray_box.html
	// based on geimer-muller ray-box intersection

	//Vector3 rcpDirection =
	//	Vector3(1.0f / p_ray.Direction.X,
	//			1.0f / p_ray.Direction.Y,
	//			1.0f / p_ray.Direction.Z);

	const Vector3 &rcpDirection = p_ray.DirectionInverseCache;

	float rMin = (m_minExtent.X - p_ray.Origin.X) * rcpDirection.X,
		rMax = (m_maxExtent.X - p_ray.Origin.X) * rcpDirection.X,
		limitMin = Maths::Min(rMin, rMax),
		limitMax = Maths::Max(rMin, rMax);

	rMin = (m_minExtent.Y - p_ray.Origin.Y) * rcpDirection.Y;
	rMax = (m_maxExtent.Y - p_ray.Origin.Y) * rcpDirection.Y;
	limitMin = Maths::Max(Maths::Min(rMin, rMax), limitMin);
	limitMax = Maths::Min(Maths::Max(rMin, rMax), limitMax);

	rMin = (m_minExtent.Z - p_ray.Origin.Z) * rcpDirection.Z;
	rMax = (m_maxExtent.Z - p_ray.Origin.Z) * rcpDirection.Z;
	limitMin = Maths::Max(Maths::Min(rMin, rMax), limitMin);
	limitMax = Maths::Min(Maths::Max(rMin, rMax), limitMax);

	if (limitMin > p_ray.Max || limitMax < p_ray.Min)
		return false;

	p_hitIn = limitMin;
	p_hitOut = limitMax;

	return ((limitMax >= 0.0f) & (limitMax >= limitMin));
	/**/
}
//----------------------------------------------------------------------------------------------
bool AxisAlignedBoundingBox::Intersects(const IBoundingVolume &p_volume) const
{
	// We only support AABBs so far, so assert type is correct
	BOOST_ASSERT(p_volume.GetType() == AxisAlignedBox);

	AxisAlignedBoundingBox &aabb = (AxisAlignedBoundingBox&)p_volume;

	return ((aabb.m_minExtent.X < m_maxExtent.X && aabb.m_maxExtent.X > m_minExtent.X) &&
		(aabb.m_minExtent.Y < m_maxExtent.Y && aabb.m_maxExtent.Y > m_minExtent.Y) &&
		(aabb.m_minExtent.Z < m_maxExtent.Z && aabb.m_maxExtent.Z > m_minExtent.Z));
}
//----------------------------------------------------------------------------------------------
bool AxisAlignedBoundingBox::Contains(const Vector3 &p_point) const
{
	for (int idx = 0; idx < 3; idx++)
	{
		if (m_minExtent[idx] > p_point[idx])
			return false;

		if (m_maxExtent[idx] < p_point[idx])
			return false;
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool AxisAlignedBoundingBox::Contains(const IBoundingVolume &p_volume) const
{
	// We only support AABBs so far, so assert type is correct
	BOOST_ASSERT(p_volume.GetType() == AxisAlignedBox);

	AxisAlignedBoundingBox &aabb = (AxisAlignedBoundingBox&)p_volume;

	for (int idx = 0; idx < 3; idx++)
	{
		if (m_minExtent[idx] > aabb.m_minExtent[idx])
			return false;

		if (m_maxExtent[idx] < aabb.m_maxExtent[idx])
			return false;
	}

	return true;
}
//----------------------------------------------------------------------------------------------
void AxisAlignedBoundingBox::Union(const Vector3 &p_point)
{
	m_minExtent = Vector3::Min(m_minExtent, p_point);
	m_maxExtent = Vector3::Max(m_maxExtent, p_point);

	Update();
}
//----------------------------------------------------------------------------------------------
void AxisAlignedBoundingBox::Union(const IBoundingVolume &p_volume)
{
	// We only support AABBs so far, so assert type is correct
	BOOST_ASSERT(p_volume.GetType() == AxisAlignedBox);

	AxisAlignedBoundingBox &aabb = (AxisAlignedBoundingBox&)p_volume;

	m_minExtent = Vector3::Min(m_minExtent, aabb.m_minExtent);
	m_maxExtent = Vector3::Max(m_maxExtent, aabb.m_maxExtent);

	Update();
}
//----------------------------------------------------------------------------------------------
void AxisAlignedBoundingBox::ProjectToInterval(const Vector3 &p_axis, Interval& p_interval) const
{
	p_interval.Set(p_axis.Dot(m_minExtent));
	p_interval.Extend(p_axis.Dot(m_maxExtent));
}
//----------------------------------------------------------------------------------------------
float AxisAlignedBoundingBox::GetDistance(const Plane &p_plane) const
{
	Vector3 pointOnPlane;

	p_plane.GetDisplacementVector(m_centre, pointOnPlane);
	Vector3::Add(m_centre, pointOnPlane, pointOnPlane);
	GetClosestPoint(pointOnPlane, pointOnPlane);

	return p_plane.GetDistance(pointOnPlane);
}
//----------------------------------------------------------------------------------------------
Plane::Side AxisAlignedBoundingBox::GetSide(const Plane &p_plane) const
{
	float fDistance = p_plane.GetDisplacement(m_centre);

	float fAbsDistance = Maths::FAbs(p_plane.Normal.X * m_extent.X) +
		Maths::Abs(p_plane.Normal.Y * m_extent.Y) +
		Maths::Abs(p_plane.Normal.Z * m_extent.Z);

	if (fDistance < -fAbsDistance)
		return Plane::Side_Negative;

	if (fDistance >  fAbsDistance)
		return Plane::Side_Positive;

	return Plane::Side_Both;
}
//----------------------------------------------------------------------------------------------
Vector3 AxisAlignedBoundingBox::GetClosestPoint(const Vector3 &p_point) const
{
	return Vector3(Maths::Min(Maths::Max(m_minExtent.X, p_point.X), m_maxExtent.X),
		Maths::Min(Maths::Max(m_minExtent.Y, p_point.Y), m_maxExtent.Y),
		Maths::Min(Maths::Max(m_minExtent.Z, p_point.Z), m_maxExtent.Z));
}
//----------------------------------------------------------------------------------------------
void AxisAlignedBoundingBox::GetClosestPoint(const Vector3 &p_point, Vector3 &p_out) const
{
	p_out.Set(Maths::Min(Maths::Max(m_minExtent.X, p_point.X), m_maxExtent.X),
		Maths::Min(Maths::Max(m_minExtent.Y, p_point.Y), m_maxExtent.Y),
		Maths::Min(Maths::Max(m_minExtent.Z, p_point.Z), m_maxExtent.Z));
}
//----------------------------------------------------------------------------------------------
float AxisAlignedBoundingBox::GetRadius(void) const {
	return m_fRadius;
}
//----------------------------------------------------------------------------------------------
Vector3 AxisAlignedBoundingBox::GetSize(void) const {
	return m_maxExtent - m_minExtent;
}
//----------------------------------------------------------------------------------------------
Vector3 AxisAlignedBoundingBox::GetExtent(void) const {
	return m_extent;
}
//----------------------------------------------------------------------------------------------
Vector3 AxisAlignedBoundingBox::GetCentre(void) const {
	return m_centre;
}
//----------------------------------------------------------------------------------------------
Vector3 AxisAlignedBoundingBox::GetMinExtent(void) const {
	return m_minExtent;
}
//----------------------------------------------------------------------------------------------
void AxisAlignedBoundingBox::SetMinExtent(const Vector3 &p_minExtent)
{
	m_minExtent = p_minExtent;
	Update();
}
//----------------------------------------------------------------------------------------------
Vector3 AxisAlignedBoundingBox::GetMaxExtent(void) const {
	return m_maxExtent;
}
//----------------------------------------------------------------------------------------------
void AxisAlignedBoundingBox::SetMaxExtent(const Vector3 &p_maxExtent)
{
	m_maxExtent = p_maxExtent;
	Update();
}
//----------------------------------------------------------------------------------------------
void AxisAlignedBoundingBox::SetMinExtent(int p_nAxis, float p_fValue)
{
	m_minExtent[p_nAxis] = p_fValue;
	Update();
}
//----------------------------------------------------------------------------------------------
void AxisAlignedBoundingBox::SetMaxExtent(int p_nAxis, float p_fValue)
{
	m_maxExtent[p_nAxis] = p_fValue;
	Update();
}
//----------------------------------------------------------------------------------------------
float AxisAlignedBoundingBox::GetMinExtent(int p_nAxis) const
{
	return m_minExtent[p_nAxis];
}
//----------------------------------------------------------------------------------------------
float AxisAlignedBoundingBox::GetMaxExtent(int p_nAxis) const
{
	return m_maxExtent[p_nAxis];
}
//----------------------------------------------------------------------------------------------
void AxisAlignedBoundingBox::SetExtents(const Vector3 &p_minExtent, const Vector3 &p_maxExtent)
{
	m_minExtent = p_minExtent;
	m_maxExtent = p_maxExtent;
	Update();
}
//----------------------------------------------------------------------------------------------
void AxisAlignedBoundingBox::Update(void)
{
	Vector3::Subtract(m_maxExtent, m_minExtent, m_extent);

	m_extent *= 0.5f;
	m_fRadius = m_extent.Length();

	Vector3::Add(m_minExtent, m_extent, m_centre);
}
//----------------------------------------------------------------------------------------------
AxisAlignedBoundingBox& AxisAlignedBoundingBox::operator=(const AxisAlignedBoundingBox &p_aabb)
{
	m_minExtent = p_aabb.m_minExtent;
	m_maxExtent = p_aabb.m_maxExtent;
	m_fRadius = p_aabb.m_fRadius;
	m_centre = p_aabb.m_centre;
	m_extent = p_aabb.m_extent;

	return *this;
}
//----------------------------------------------------------------------------------------------
std::string AxisAlignedBoundingBox::ToString(void) const
{
	boost::format formatter;
	std::string strOut = boost::str(boost::format("[%s - %s]") % m_minExtent.ToString() % m_maxExtent.ToString());
	return strOut;
}
//----------------------------------------------------------------------------------------------
