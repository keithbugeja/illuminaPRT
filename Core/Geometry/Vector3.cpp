//----------------------------------------------------------------------------------------------
//	Filename:	Vector3.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Geometry/Vector3.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
Vector3::Vector3(void) 
{ }
//----------------------------------------------------------------------------------------------
Vector3::Vector3(float p_fValue)
	: X(p_fValue), Y(p_fValue), Z(p_fValue) 
{ }
//----------------------------------------------------------------------------------------------
Vector3::Vector3(float p_x, float p_y, float p_z)
	: X(p_x), Y(p_y), Z(p_z) 
{ }
//----------------------------------------------------------------------------------------------
Vector3::Vector3(const Vector3 &p_vector)
	: X(p_vector.X), Y(p_vector.Y), Z(p_vector.Z) 
{ }
//----------------------------------------------------------------------------------------------
std::string Vector3::ToString(void) const
{
	boost::format("[%d %d %d]") % X % Y % Z;
	std::string strOut = boost::str(boost::format("[%d %d %d]") % X % Y % Z);
	return strOut;
}
//----------------------------------------------------------------------------------------------
Vector3 const Vector3::Ones = Vector3(1.0f);
Vector3 const Vector3::Zero	= Vector3(0.0f);
Vector3 const Vector3::UnitXNeg = Vector3(-1.0f, 0.0f, 0.0f);
Vector3 const Vector3::UnitXPos = Vector3( 1.0f, 0.0f, 0.0f);
Vector3 const Vector3::UnitYNeg = Vector3( 0.0f,-1.0f, 0.0f);
Vector3 const Vector3::UnitYPos = Vector3( 0.0f, 1.0f, 0.0f);
Vector3 const Vector3::UnitZNeg = Vector3( 0.0f, 0.0f,-1.0f);
Vector3 const Vector3::UnitZPos = Vector3( 0.0f, 0.0f, 1.0f);
