//----------------------------------------------------------------------------------------------
//	Filename:	Basis.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Geometry/Basis.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
const float OrthonormalBasis::Epsilon = 0.0001f;

//----------------------------------------------------------------------------------------------
OrthonormalBasis::OrthonormalBasis(void)
	: U(0.0f)
	, V(0.0f)
	, W(0.0f)
{ }
//----------------------------------------------------------------------------------------------
OrthonormalBasis::OrthonormalBasis(const Vector3 &p_u, const Vector3 &p_v, const Vector3 &p_w) 
	: U(p_u)
	, V(p_v)
	, W(p_w)
{ }
//----------------------------------------------------------------------------------------------
void OrthonormalBasis::FrowWV(const Vector3 &p_w, const Vector3 &p_v)
{
	// Using Gram-Schmidt
	W = Vector3::Normalize(p_w);
	V = Vector3::Normalize(p_v);
	V = V - Vector3::Dot(V, W) * W;

	// Do we choose another axis?
	if (V.LengthSquared() <= Maths::Epsilon)
	{
		V.Set(W.Z, W.X, W.Y);
		V = Vector3::Normalize(Vector3::Cross(V, W));
		V = Vector3::Normalize(V - Vector3::Dot(V, W) * W);
	}

	U = Vector3::Cross(V, W);

}
//----------------------------------------------------------------------------------------------
 void OrthonormalBasis::InitFromU(const Vector3 &p_u)
{
	U = Vector3::Normalize(p_u);
	V = Vector3::Cross(Vector3::UnitZPos, U);

	if (V.LengthSquared() < Epsilon)
		V = Vector3::Cross(Vector3::UnitXPos, U);

	W = Vector3::Cross(U, V);

	//U = Vector3::Normalize(p_u);
	//V = Vector3::Cross(U, Vector3::UnitXPos);
	//			
	//if (V.LengthSquared() < Epsilon)
	//	V = Vector3::Cross(U, Vector3::UnitYPos);

	//W = Vector3::Cross(U, V);
}
//----------------------------------------------------------------------------------------------
void OrthonormalBasis::InitFromV(const Vector3 &p_v)
{
	V = Vector3::Normalize(p_v);
	U = Vector3::Cross(V, Vector3::UnitZPos);

	if (U.LengthSquared() < Epsilon)
		U = Vector3::Cross(V, Vector3::UnitYNeg);

	W = Vector3::Cross(U, V);

	//V = Vector3::Normalize(p_v);
	//U = Vector3::Cross(V, Vector3::UnitXPos);
	//			
	//if (U.LengthSquared() < Epsilon)
	//	U = Vector3::Cross(V, Vector3::UnitYPos);

	//W = Vector3::Cross(U, V);
}
//----------------------------------------------------------------------------------------------
void OrthonormalBasis::InitFromW(const Vector3 &p_w)
{
	W = Vector3::Normalize(p_w);
	U = Vector3::Cross(W, Vector3::UnitXPos);
				
	if (U.LengthSquared() < Epsilon)
		U = Vector3::Cross(W, Vector3::UnitYPos);

	V = Vector3::Cross(W, U);
}
//----------------------------------------------------------------------------------------------
void OrthonormalBasis::InitFromUV(const Vector3 &p_u, const Vector3 &p_v)
{
	U = Vector3::Normalize(p_u);
	W = Vector3::Normalize(Vector3::Cross(p_u, p_v));
	V = Vector3::Cross(W, U);
}
//----------------------------------------------------------------------------------------------
void OrthonormalBasis::InitFromVU(const Vector3 &p_v, const Vector3 &p_u)
{
	V = Vector3::Normalize(p_v);
	W = Vector3::Normalize(Vector3::Cross(p_u, p_v));
	U = Vector3::Cross(V, W);
}
//----------------------------------------------------------------------------------------------
void OrthonormalBasis::InitFromUW(const Vector3 &p_u, const Vector3 &p_w)
{
	U = Vector3::Normalize(p_u);
	V = Vector3::Normalize(Vector3::Cross(p_w, p_u));
	W = Vector3::Cross(U, V);
}
//----------------------------------------------------------------------------------------------
void OrthonormalBasis::InitFromWU(const Vector3 &p_w, const Vector3 &p_u)
{
	W = Vector3::Normalize(p_w);
	V = Vector3::Normalize(Vector3::Cross(p_w, p_u));
	U = Vector3::Cross(V, W);
}
//----------------------------------------------------------------------------------------------
void OrthonormalBasis::InitFromVW(const Vector3 &p_v, const Vector3 &p_w)
{
	V = Vector3::Normalize(p_v);
	U = Vector3::Normalize(Vector3::Cross(p_v, p_w));
	W = Vector3::Cross(U, V);
}
//----------------------------------------------------------------------------------------------
void OrthonormalBasis::InitFromWV(const Vector3 &p_w, const Vector3 &p_v)
{
	this->FrowWV(p_w, p_v);

	//Shirley's original version
	//W = Vector3::Normalize(p_w);
	//U = Vector3::Normalize(Vector3::Cross(p_v, p_w));
	//V = Vector3::Cross(W, U);
}
//----------------------------------------------------------------------------------------------
Vector3 OrthonormalBasis::GetU() const { 
	return U; 
}
//----------------------------------------------------------------------------------------------
Vector3 OrthonormalBasis::GetV() const { 
	return V; 
}
//----------------------------------------------------------------------------------------------
Vector3 OrthonormalBasis::GetW() const { 
	return W; 
}
//----------------------------------------------------------------------------------------------
Vector3 OrthonormalBasis::Project(const Vector3 &p_vector) const
{
	return p_vector.X * U + p_vector.Y * V + p_vector.Z * W;
}
//----------------------------------------------------------------------------------------------
Matrix3x3 OrthonormalBasis::GetMatrix() const { 
	return Matrix3x3(U, V, W);
}
//----------------------------------------------------------------------------------------------
bool OrthonormalBasis::operator==(const OrthonormalBasis &p_basis) const {
	return (U == p_basis.U) && (V == p_basis.V) && (W == p_basis.W);
}
//----------------------------------------------------------------------------------------------
std::string OrthonormalBasis::ToString(void)
{
	std::string strOut = boost::str(boost::format("[%1% %2% %3%]") 
		% U.ToString() % V.ToString() % W.ToString());
	return strOut;
}
//----------------------------------------------------------------------------------------------