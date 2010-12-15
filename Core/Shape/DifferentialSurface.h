//----------------------------------------------------------------------------------------------
//	Filename:	Shape.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Implementation for Rays in R3
//----------------------------------------------------------------------------------------------
#pragma once

#include "Geometry/Ray.h"
#include "Geometry/Basis.h"

#include "Image/RGBPixel.h"

namespace Illumina 
{
	namespace Core
	{
		class Shape;

		/* Structure describing a surface point */
		class DifferentialSurface
		{
		protected:
			Shape *m_pShape;

		public:
			/* Surface info in local space */
			Vector3	Normal;
			Vector3 Point;
			Vector2 PointUV;

			/* Surface info in world space */
			OrthonormalBasis BasisWS;
			Vector3 PointWS;

			/* Space-independent info */
			/* Colour at surface point : temporary */
			RGBPixel		Colour;

			/* Surface point expressed as P = O + tD, where t = Distance */
			float	Distance;

		public:
			DifferentialSurface() {}

			inline void SetShape(const Shape *p_pShape) { 
				m_pShape = (Shape*)p_pShape; 
			}

			inline Shape* GetShape(void) const { 
				return m_pShape; 
			}
		};
	} 
}