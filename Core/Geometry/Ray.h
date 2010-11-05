//----------------------------------------------------------------------------------------------
//	Filename:	Ray.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Implementation for Rays in R3
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>

#include "Geometry/Vector2.h"
#include "Geometry/Vector3.h"
#include "Geometry/Plane.h"
#include "Geometry/Transform.h"

namespace Illumina 
{
	namespace Core
	{
		class Ray
		{
		public:
			Vector3 Origin,
				Direction;
		
			float Min, Max;

		public:
			Ray(void) {}

			Ray(const Vector3 &p_origin, const Vector3 &p_direction)
				: Origin(p_origin), Direction(p_direction), Min(0.0f), Max(Maths::Maximum) {}

			Ray(const Vector3 &p_origin, const Vector3 &p_direction, float p_fMin, float p_fMax = Maths::Maximum)
				: Origin(p_origin), Direction(p_direction), Min(p_fMin), Max(p_fMax) {}

			Ray(const Ray &p_ray) {
				*this = p_ray;
			}

			inline int GetID(void) const {
				return ((int)(Direction.X * 0xFF00) & 0xFF00)  + (int)(Direction.Y * 0xFF);
				//return (int)(Direction.X * 0xFF00) + (Direction.Y * 0xFF);
			}

			inline Vector3 GetOrigin(void) const {
				return Origin;
			}

			inline Vector3 GetDirection(void) const {
				return Direction;
			}

			inline float GetMin(void) const {
				return Min;
			}

			inline float GetMax(void) const {
				return Max;
			}

			inline void Set(const Vector3 &p_origin, const Vector3 &p_direction, float p_fMin = 0.0f, float p_fMax = Maths::Maximum)
			{
				Origin = p_origin;
				Direction = p_direction;
				Min = p_fMin;
				Max = p_fMax;
			}

			inline Vector3 PointAlongRay(float p_fDistance) const {
				return Origin + p_fDistance * Direction;
			}

			inline void PointAlongRay(float p_fDistance, Vector3 &p_out) {
				Vector3::Add(Origin, p_fDistance * Direction, p_out);
			}

			inline Ray Apply(const Transformation &p_transformation) const
			{
				Ray result;
				Ray::Apply(p_transformation, *this, result);
				return result;
			}

			inline Ray ApplyInverse(const Transformation &p_transformation) const 
			{
				Ray result;
				Ray::ApplyInverse(p_transformation, *this, result);
				return result;
			}

			inline Ray& operator=(const Ray &p_ray)
			{
				Origin = p_ray.Origin;
				Direction = p_ray.Direction;
				Min = p_ray.Min;
				Max = p_ray.Max;

				return *this;
			}
			
			static void Apply(const Transformation &p_transformation, const Ray &p_ray, Ray &p_out)
			{
				p_transformation.Rotate(p_ray.Direction, p_out.Direction);
				p_transformation.Scale(p_out.Direction, p_out.Direction);
				p_transformation.Apply(p_ray.Origin, p_out.Origin);

				p_out.Min = p_ray.Min;
				p_out.Max = p_ray.Max;
			}

			static void ApplyInverse(const Transformation &p_transformation, const Ray &p_ray, Ray &p_out)
			{
				p_transformation.RotateInverse(p_ray.Direction, p_out.Direction);
				p_transformation.ScaleInverse(p_out.Direction, p_out.Direction);
				p_transformation.ApplyInverse(p_ray.Origin, p_out.Origin);

				p_out.Min = p_ray.Min;
				p_out.Max = p_ray.Max;
			}

			std::string ToString(void)
			{
				boost::format formatter;
				std::string strOut = boost::str(boost::format("[O:<%1%> D:<%2%> <%d-%d>]") 
					% Origin.ToString() 
					% Direction.ToString() 
					% Min % Max);
				return strOut;
			}
		};

		class RaySIMD 
		{
		};

		// Frustum
		struct Frustum 
		{
			Plane frustum[6];
			Plane& operator[](int i) { return frustum[i]; }
		};

		// Ray Packet
		template<class T>
		class RayPacket
		{
		public:
			bool m_bDirty;
			int m_nDensity;
			//Plane m_frustum[6];
			Frustum m_frustum;
			List<T*> RayList;
			
			// Rays should be in the same general direction!
			void BuildPacket(T& p_ray00, T& p_ray10, T& p_ray01, T& p_ray11, int p_nDensity)
			{
				Vector3 near[3], far[3];

				p_ray10.PointAlongRay(p_ray10.Min, near[0]);
				p_ray01.PointAlongRay(p_ray01.Min, near[1]);
				p_ray11.PointAlongRay(p_ray11.Min, near[2]);

				p_ray10.PointAlongRay(p_ray10.Min, far[0]);
				p_ray11.PointAlongRay(p_ray11.Min, far[1]);
				p_ray01.PointAlongRay(p_ray01.Min, far[2]);

				// Replace with Build method
				new (m_frustum[0]) Plane(near[0], near[1], near[2]);	// N
				new (m_frustum[1]) Plane(far[0], far[1], far[2]);		// F
				new (m_frustum[2]) Plane(far[1], near[1], far[2]);	// L
				new (m_frustum[3]) Plane(far[0], near[0], near[2]);	// R
				new (m_frustum[4]) Plane(far[0], far[1], near[0]);	// T
				new (m_frustum[5]) Plane(far[2], near[1], near[2]);	// B

				RayList.PushBack(p_ray00);
				RayList.PushBack(p_ray10);
				RayList.PushBack(p_ray01);
				RayList.PushBack(p_ray11);

				m_nDensity = p_nDensity;
				m_bDirty = true;
			}

			void BuildFrustum(void) 
			{
			}

			List<Ray> GetRayList(void)
			{
				// If dirty, generate rays
				if (m_bDirty)
				{
					Vector3 nearT = RayList[1].Origin - RayList[0].Origin;
					Vector3 nearB = RayList[3].Origin - RayList[2].Origin;

					Vector3 farT = RayList[1].Direction - RayList[0].Direction;
					Vector3 farB = RayList[3].Direction - RayList[2].Direction;

					float minT = RayList[1].Min - RayList[0].Min;
					float maxT = RayList[1].Max - RayList[0].Max;
					float minB = RayList[3].Min - RayList[2].Min;
					float maxB = RayList[3].Max - RayList[3].Max;

					Ray rT, rB, rU, r;

					float i = 1.0f / ((float)m_nDensity);

					List<Ray> list(m_nDensity * m_nDensity);

					for (t = 0; t < 1; t += i)
					{
						float hT = Maths::Hermite(t);

						rT.Origin = RayList[0].Origin + hT * nearT;
						rT.Direction = RayList[0].Direction + hT * farT;
						rT.Min = RayList[0].Min + hT * minT;
						rT.Max = RayList[0].Max + hT * maxT;
						rT.Direction.Normalize();

						rB.Origin = RayList[2].Origin + Maths::Hermite(t) * nearB;
						rB.Direction = RayList[2].Direction + Maths::Hermite(t) * farB;
						rB.Min = RayList[2].Min + hT * minB;
						rB.Max = RayList[2].Max + hT * maxB;
						rB.Direction.Normalize();

						rU.Origin = rB.Origin - rT.Origin;
						rU.Direction = rB.Direction - rT.Direction;
						rU.Min = rB.Min - rT.Min;
						rU.Max = rB.Max - rT.Max;
						rU.Direction.Normalize();

						for (u = 0; u < 1; u += i)
						{
							float hU = Maths::Hermite(u);
							r.Origin = rT.Origin + hU * rU.Origin;
							r.Direction = rT.Direction + hU * rU.Direction;
							r.Min = rT.Min + hU * rU.Min;
							r.Max = rT.Max + hU * rU.Max;

							list.PushBack(r);
						}
					}

					return m_nList;
				}
			}
		};
	} 
}