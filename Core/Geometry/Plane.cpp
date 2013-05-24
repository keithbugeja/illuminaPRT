//----------------------------------------------------------------------------------------------
//	Filename:	Plane.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Geometry/Plane.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
Plane::Plane(void) 
{ }
//----------------------------------------------------------------------------------------------
Plane::Plane(const Plane &p_plane)
	: Distance( p_plane.Distance)
	, Normal( p_plane.Normal )
{ }
//----------------------------------------------------------------------------------------------
Plane::Plane(const Vector3 &p_normal, float p_fDistance)
	: Distance(p_fDistance)
	, Normal(p_normal)
{ }
//----------------------------------------------------------------------------------------------
Plane::Plane(const Vector3 &p_normal, const Vector3 &p_point)
{
	// This should formally be transcribed as:
	// TVector3<TReal> P = p_planePoint - p_planePoint.Origin;
	// Distance = P.DotProduct( p_normalVector );
	// Normal = p_normalVector;

	Distance = Vector3::Dot(p_normal, p_point);
	Normal = p_normal;
}
//----------------------------------------------------------------------------------------------
Plane::Plane(const Vector3 &p_point1, const Vector3 &p_point2, const Vector3 &p_point3)
{
	// N = norm(AB x AC)
	// D = (A - O) . N

	Vector3 AB = p_point2 - p_point1,
			AC = p_point3 - p_point1;

	Vector3::Cross(AB, AC, Normal);
	Normal.Normalize();

	Distance = Vector3::Dot(p_point1, Normal);
}
//----------------------------------------------------------------------------------------------
Plane::Side Plane::GetSide(const Vector3 &p_vector) const
{
	// (P - dN) . N = 0
	Vector3 P = p_vector - (Distance * Normal);
	int nDotSign = Maths::ISgn(P.Dot(Normal));

	if ( nDotSign == -1 )
		return Side_Negative;
	else if (nDotSign == 1)
		return Side_Positive;

	// Co-planar
	return Side_Neither;
}
//----------------------------------------------------------------------------------------------
Plane::Side Plane::GetSide(const Vector3 &p_midPoint, const Vector3 &p_halfVector) const
{
	const Vector3 &M = p_midPoint - (Distance * Normal),
			&A = M + p_halfVector,
			&B = M - p_halfVector;

	int nASign = Maths::ISgn(A.Dot(Normal)),
		nBSign = Maths::ISgn(B.Dot(Normal));

	// Both points lie in the same half-space
	if ( nASign - nBSign == 0 )
	{
		if ( nASign == -1 )
			return Side_Negative;
		else if (nASign == 1)
			return Side_Positive;

		return Side_Neither;
	}

	return Side_Both;
}
//----------------------------------------------------------------------------------------------
bool Plane::Equals(const Plane &p_plane, float p_fEpsilon) const
{
	return ((Maths::Abs(p_plane.Distance - Distance) <= p_fEpsilon) &&
		p_plane.Normal.Equals(Normal));
}
//----------------------------------------------------------------------------------------------
bool Plane::operator==(const Plane &p_plane) const {
	return Equals(p_plane);
}
//----------------------------------------------------------------------------------------------
bool Plane::operator!=(const Plane &p_plane) const {
	return !Equals(p_plane);
}
//----------------------------------------------------------------------------------------------
std::string Plane::ToString(void) const
{
	std::string strOut = boost::str(boost::format("[%d %d]") % Normal.ToString() % Distance);
	return strOut;
}
//----------------------------------------------------------------------------------------------
