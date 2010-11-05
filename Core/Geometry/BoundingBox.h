//----------------------------------------------------------------------------------------------
//	Filename:	AxisAlignedBoundingBox.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Geometry/BoundingVolume.h"

namespace Illumina 
{
	namespace Core
	{
		class AxisAlignedBoundingBox : 
			public IBoundingVolume
		{
		protected:
			float m_fRadius;

			Vector3 m_minExtent,
					m_maxExtent,
					m_centre,
					m_extent;
		
		public:
			AxisAlignedBoundingBox(void);
			AxisAlignedBoundingBox(const Vector3 &p_minExtent, const Vector3 &p_maxExtent);
			AxisAlignedBoundingBox(const AxisAlignedBoundingBox &p_aabb);

			BoundingVolumeType GetType(void) const;

			boost::shared_ptr<IBoundingVolume> Clone(void) const;
			boost::shared_ptr<IBoundingVolume> CreateInstance(void) const;

			void Invalidate(void);

			void ComputeFromPoints(const Vector3List &p_pointList);
			void ComputeFromPoints(const Vector3 *p_pointList, int p_nCount);
			void ComputeFromVolume(const IBoundingVolume& p_boundingVolume);
			void ComputeFromVolume(const Transformation& p_transformation, const IBoundingVolume& p_boundingVolume);

			boost::shared_ptr<IBoundingVolume> Apply(const Transformation &p_transformation) const;
			void Apply(const Transformation &p_transformation, IBoundingVolume &p_out) const;

			bool Intersects(const Ray &p_ray) const;
			bool Intersects(const Ray &p_ray, float &p_hitIn, float &p_hitOut) const;
			bool Intersects(const IBoundingVolume &p_boundingVolume) const;

			float GetRadius(void) const;
			Vector3 GetCentre(void) const;
			Vector3 GetSize(void) const;

			Vector3 GetExtent(void) const;
			Vector3 GetMinExtent(void) const;
			Vector3 GetMaxExtent(void) const;
			float GetMinExtent(int p_nAxis) const;
			float GetMaxExtent(int p_nAxis) const;

			bool Contains(const Vector3 &p_point) const;
			bool Contains(const IBoundingVolume &p_volume) const;

			void Union(const Vector3 &p_point);
			void Union(const IBoundingVolume &p_volume);

			void ProjectToInterval(const Vector3 &p_axis, Interval& p_interval) const;

			float GetDistance(const Plane &p_plane) const;
			Plane::Side GetSide(const Plane &p_plane) const;
			Vector3 GetClosestPoint(const Vector3 &p_point) const;
			void GetClosestPoint(const Vector3 &p_point, Vector3 &p_out) const;

			// AABB specific methods
			void SetMinExtent(const Vector3 &p_minExtent);
			void SetMaxExtent(const Vector3 &p_maxExtent);

			void SetMinExtent(int p_nAxis, float p_fValue);
			void SetMaxExtent(int p_nAxis, float p_fValue);

			void SetExtents(const Vector3 &p_minExtent, const Vector3 &p_maxExtent);

			AxisAlignedBoundingBox& operator=(const AxisAlignedBoundingBox &p_aabb);

			// ToString
			std::string ToString(void) const;

		protected:
			void Update(void);
		};
	}
}