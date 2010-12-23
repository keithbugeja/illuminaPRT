//----------------------------------------------------------------------------------------------
//	Filename:	Shape.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Implementation for Rays in R3
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"

#include "Geometry/Ray.h"
#include "Geometry/Basis.h"
#include "Image/RGBPixel.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		/* Structure describing a surface point */
		class DifferentialSurface
		{
		protected:
			IShape *m_pShape;

		public:
			/* Surface info in local space */
			Vector3 ShadingNormal;
			Vector3 GeometryNormal;
			Vector3 Point;
			Vector2 PointUV;

			/* Surface info in world space */
			OrthonormalBasis ShadingBasisWS;
			OrthonormalBasis GeometryBasisWS;
			Vector3 PointWS;

			/* Surface point expressed as P = O + tD, where t = Distance */
			float	Distance;

		public:
			DifferentialSurface() {}

			inline void SetShape(const IShape *p_pShape) { 
				m_pShape = (IShape*)p_pShape; 
			}

			inline IShape* GetShape(void) const { 
				return m_pShape; 
			}
		};
	} 
}