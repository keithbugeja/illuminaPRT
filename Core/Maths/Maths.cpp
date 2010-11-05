//----------------------------------------------------------------------------------------------
//	Filename:	Maths.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Hub for various mathematical functions in single precision floating point
//----------------------------------------------------------------------------------------------
#include "Maths/Maths.h"

using namespace Illumina::Core;

const float Maths::E			= Maths::Exp(1.0f);
const float Maths::Pi			= Maths::Atan(1.0f) * 4.0f;
const float Maths::PiTwo		= Maths::Atan(1.0f) * 8.0f;
const float Maths::PiHalf		= Maths::Atan(1.0f) * 2.0f;
const float Maths::InvPi		= 1.0f / (Maths::Atan(1.0f) * 4.0f);
const float Maths::InvPiTwo		= 1.0f / (Maths::Atan(1.0f) * 8.0f);
const float Maths::SqrtPi		= Maths::Sqrt(Maths::Atan(1.0f) * 4.0f);
const float Maths::SqrtPiTwo	= Maths::Sqrt(Maths::Atan(1.0f) * 8.0f);
const float Maths::InvSqrtPi	= 1.0f / Maths::Sqrt(Maths::Atan(1.0f) * 4.0f);
const float Maths::InvSqrtPiTwo	= 1.0f / Maths::Sqrt(Maths::Atan(1.0f) * 8.0f);
const float Maths::Epsilon		= std::numeric_limits<float>::epsilon();
const float Maths::Infinity		= std::numeric_limits<float>::infinity();
const float Maths::Minimum		= std::numeric_limits<float>::min();
const float Maths::Maximum		= std::numeric_limits<float>::max();
const float Maths::Tolerance	= 1e-06f;

const float Maths::_DegToRad		= Maths::Atan(1.0f) / 45.0f;
const float Maths::_RadToDeg		= 45.0f / Maths::Atan(1.0f);
