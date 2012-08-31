//----------------------------------------------------------------------------------------------
//	Filename:	BoundingVolume.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"

#include "Geometry/Vector3.h"
#include "Geometry/Plane.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		enum BoundingVolumeType
		{
			AxisAlignedBox,
			OrientedBox,
			Spherical,
			Capsule
		};

		class IBoundingVolume
		{
		public:
            virtual ~IBoundingVolume() {}
            
			virtual BoundingVolumeType GetType(void) const = 0;

			virtual boost::shared_ptr<IBoundingVolume> Clone(void) const = 0;
			virtual boost::shared_ptr<IBoundingVolume> CreateInstance(void) const = 0;

			virtual void Invalidate(void) = 0;

			virtual void ComputeFromPoints(const Vector3List &p_pointList) = 0;
			virtual void ComputeFromPoints(const Vector3 *p_pointList, int p_nCount) = 0;
			virtual void ComputeFromVolume(const IBoundingVolume& p_boundingVolume) = 0;
			virtual void ComputeFromVolume(const Transformation& p_transformation, const IBoundingVolume& p_boundingVolume) = 0;

			virtual boost::shared_ptr<IBoundingVolume> Apply(const Transformation &p_transformation) const = 0;
			virtual void Apply(const Transformation &p_transformation, IBoundingVolume &p_out) const = 0;

			virtual bool Intersects(const Ray &p_ray) const = 0;
			virtual bool Intersects(const Ray &p_ray, float &p_hitIn, float &p_hitOut) const = 0;
			virtual bool Intersects(const IBoundingVolume &p_boundingVolume) const = 0;

			virtual float GetRadius(void) const = 0;
			virtual Vector3 GetCentre(void) const = 0;
			virtual Vector3 GetSize(void) const = 0;

			virtual Vector3 GetExtent(void) const = 0;
			virtual Vector3 GetMinExtent(void) const = 0;
			virtual Vector3 GetMaxExtent(void) const = 0;
			virtual float GetMinExtent(int p_nAxis) const = 0;
			virtual float GetMaxExtent(int p_nAxis) const = 0;

			virtual bool Contains(const Vector3 &p_point) const = 0;
			virtual bool Contains(const IBoundingVolume &p_volume) const = 0;

			virtual void Union(const Vector3 &p_point) = 0;
			virtual void Union(const IBoundingVolume &p_volume) = 0;

			virtual float GetDistance(const Plane &p_plane) const = 0;
			virtual Plane::Side GetSide(const Plane &p_plane) const = 0;
			virtual Vector3 GetClosestPoint(const Vector3 &p_point) const = 0;
			virtual void GetClosestPoint(const Vector3 &p_point, Vector3 &p_out) const = 0;

			virtual void ProjectToInterval(const Vector3 &p_axis, Interval& p_interval) const = 0;

			virtual std::string ToString(void) const = 0;
		};

		typedef boost::shared_ptr<IBoundingVolume> BoundingVolumePtr;
	}
}

