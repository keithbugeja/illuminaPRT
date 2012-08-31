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
		class Fragment
		{
		protected:
			int m_nGroupId;

		public:
			Fragment(void) : m_nGroupId(-1) { }
			Fragment(const Fragment& p_fragment) : m_nGroupId(p_fragment.m_nGroupId) { }

			void Reset(void) { m_nGroupId = -1; }
			bool HasGroup(void) { return m_nGroupId >= 0; }
			int GetGroupId(void) { return m_nGroupId; }
			void SetGroupId(int p_nGroupId) { m_nGroupId = p_nGroupId; }

			Fragment& operator=(const Fragment &p_fragment) {
				m_nGroupId = p_fragment.m_nGroupId;

				return *this;
			}
		};

		/* Structure describing a surface point */
		class DifferentialSurface
		{
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
			Vector3 RayOrigin,				
				RayDirection;

			Vector3 RayOriginWS,
				RayDirectionWS;

			Fragment GeometryFragment;
		
		protected:
			IShape *m_pShape;

		public:
			DifferentialSurface(void) : Distance(Maths::Maximum), m_pShape(NULL) { }

			DifferentialSurface(const DifferentialSurface &p_surface) 
				: ShadingNormal(p_surface.ShadingNormal)
				, GeometryNormal(p_surface.GeometryNormal)
				, Point(p_surface.Point)
				, PointUV(p_surface.PointUV)
				, ShadingBasisWS(p_surface.ShadingBasisWS)
				, GeometryBasisWS(p_surface.GeometryBasisWS)
				, PointWS(p_surface.PointWS)
				, Distance(p_surface.Distance)
				, RayOrigin(p_surface.RayOrigin)
				, RayDirection(p_surface.RayDirection)
				, RayOriginWS(p_surface.RayDirectionWS)
				, RayDirectionWS(p_surface.RayDirectionWS)
				, GeometryFragment(p_surface.GeometryFragment)
                , m_pShape(p_surface.m_pShape)
			{ }

			inline void Reset(void) 
			{
				Distance = Maths::Maximum;
				GeometryFragment.Reset();
				m_pShape = NULL;
			}

			inline void SetShape(const IShape *p_pShape) { 
				m_pShape = (IShape*)p_pShape; 
			}

			inline IShape* GetShape(void) const { 
				return m_pShape; 
			}

			inline bool HasShape(void) const { 
				return m_pShape != NULL;
			}

			const DifferentialSurface& operator=(const DifferentialSurface &p_surface)
			{
				ShadingNormal = p_surface.ShadingNormal;
				GeometryNormal = p_surface.GeometryNormal;
				Point = p_surface.Point;
				PointUV = p_surface.PointUV;
				ShadingBasisWS = p_surface.ShadingBasisWS;
				GeometryBasisWS = p_surface.GeometryBasisWS;
				PointWS = p_surface.PointWS;
				Distance = p_surface.Distance;
				RayOrigin = p_surface.RayOrigin;
				RayDirection = p_surface.RayDirection;
				RayOriginWS = p_surface.RayOriginWS;
				RayDirectionWS = p_surface.RayDirectionWS;
				GeometryFragment = p_surface.GeometryFragment;				

				m_pShape = p_surface.m_pShape;

				return *this;
			}
		};
	} 
}