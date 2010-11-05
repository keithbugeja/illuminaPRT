//----------------------------------------------------------------------------------------------
//	Filename:	Vector2.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//  Description
//----------------------------------------------------------------------------------------------
#include "Geometry/Vector2.h"

using namespace Illumina::Core;

Vector2 const Vector2::Zero	= Vector2(0.0f);
Vector2 const Vector2::UnitXNeg = Vector2(-1.0f, 0.0f);
Vector2 const Vector2::UnitXPos = Vector2( 1.0f, 0.0f);
Vector2 const Vector2::UnitYNeg = Vector2( 0.0f,-1.0f);
Vector2 const Vector2::UnitYPos = Vector2( 0.0f, 1.0f);
