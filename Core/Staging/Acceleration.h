//----------------------------------------------------------------------------------------------
//	Filename:	Acceleration.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "Geometry/BoundingBox.h"
#include "Threading/List.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		template <class T>
		class IAccelerationStructure
		{
		public:
			virtual void Remove(const T &p_element) = 0;
			virtual void Insert(const T &p_element) = 0;

			virtual bool Build(void) = 0;
			virtual bool Update(void) = 0;

			// Intersect
			virtual bool Intersects(const Ray &p_ray) = 0;

			// Find 
			virtual bool Find(const Ray &p_ray, List<T> p_result) = 0;

			// Lookup
			virtual bool Lookup(const Vector3 &p_lookupPoint) = 0;

			virtual float GetIntegrityScore(void) = 0;
		};

		template <class T>
		class IAccelerationStructureLookupMethod
		{
			virtual bool operator()(const Vector3 &p_lookupPoint, T &p_element) = 0;
		};

		template <class T>
		struct KDTreeNode
		{
			unsigned char NodeType : 1;
			unsigned char Axis	   : 2;

			float Partition;
			KDTreeNode *ChildNode[2];
			List<T> ObjectList;

			KDTreeNode(void) { ChildNode[0] = ChildNode[1] = NULL; }
			~KDTreeNode() { }
		};

		template <class T, class U>
		class KDTree 
			: public IAccelerationStructure<T>
		{
			KDTreeNode<T*> RootNode;
			List<T*> ObjectList;

		protected:
			AxisAlignedBoundingBox ComputeNodeBounds(List<T*> p_objectList) 
			{
				AxisAlignedBoundingBox aabb;
			}

		public:
			void Remove(const T &p_element) {
				throw new Exception("Method not supported!");
			}

			void Insert(const T &p_element) {
				ObjectList.PushBack(&p_element);
			}

			bool Build(void) 
			{
				for (int index = 0; index < ObjectList.Size(); index++)
				{
				}
				// Compute bounding volume of scene
				// Select initial partition axis
				// Start building
				return true;
			}

			bool Update(void) {
				return true;
			}

			bool Intersects(const Ray &p_ray) {
				return true;
			}

			bool Find(const Ray &p_ray, List<T> p_result) {
				return true;
			}

			bool Lookup(const Vector3 &p_lookupPoint) {
				return true;
			}

			float GetIntegrityScore(void) {
				return 0;
			}
		};
	} 
}