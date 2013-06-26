//----------------------------------------------------------------------------------------------
//	Filename:	ICIntegrator.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Integrator/Integrator.h"
#include "Geometry/Intersection.h"
#include "Geometry/BoundingBox.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		struct IrradianceCacheRecord;

		struct IrradianceCacheNode
		{
			AxisAlignedBoundingBox Bounds;
			IrradianceCacheNode *Children;
			std::vector<IrradianceCacheRecord*> RecordList;

			IrradianceCacheNode(void) 
				: Children (nullptr)
			{ }

			void Add(IrradianceCacheRecord *p_pRecord)
			{
				RecordList.push_back(p_pRecord);
			}
		};

		class IrradianceCache
		{
		public:
			int m_nInsertCount,
				m_nRecordCount,
				m_nNodeCount;

		public:
			IrradianceCacheNode RootNode;

		public:
			IrradianceCache(void) 
				: m_nInsertCount(0)
				, m_nRecordCount(0)
				, m_nNodeCount(0)
			{ 
				AxisAlignedBoundingBox aabb, aabb2;
				aabb.SetMinExtent(Vector3(-1));
				aabb.SetMaxExtent(Vector3(1));

				for (int i=0; i< 8; i++)
				{
					SetBounds(aabb, i, aabb2);
					std::cout << "BB : [" << i << "] :: " << aabb2.ToString() << std::endl;
				}
			}

			int CountNodes(IrradianceCacheNode* p_pNode) const;
			
			void SetBounds(const AxisAlignedBoundingBox &p_parent, int p_nChildIndex, AxisAlignedBoundingBox &p_child);

			bool SphereBoxOverlap(const AxisAlignedBoundingBox &p_aabb,
				const Vector3& p_centre, const float p_fRadius) const;
			
			void Insert(IrradianceCacheNode *p_pNode, IrradianceCacheRecord *p_pRecord, int p_nDepth);

			bool FindRecords(const Vector3 &p_point, const Vector3 &p_normal, 
				std::vector<std::pair<float, IrradianceCacheRecord*>>& p_nearbyRecordList);

			float W(const Vector3 &p_point, const Vector3 &p_normal, IrradianceCacheRecord &p_record);

			std::string ToString(void) const;
		};

		class ICIntegrator : 
			public IIntegrator
		{
		protected:
			int m_nRayDepth,
				m_nTreeDepth,
				m_nDivisions,
				m_nShadowSampleCount;

			float m_fReflectEpsilon;

			float m_fRMin, 
				m_fRMax;

			IrradianceCache m_irradianceCache;

		protected:
			Spectrum GetIrradiance(const Intersection &p_intersection, Scene *p_pScene);
			void ComputeRecord(const Intersection &p_intersection, Scene *p_pScene, IrradianceCacheRecord &p_record);

		public:
			ICIntegrator(const std::string &p_strName, int p_nRayDepth, int p_nDivisions, int p_nShadowSampleCount = 1, float p_fReflectEpsilon = 1E-1f);
			ICIntegrator(int p_nRayDepth, int p_nDivisions, int p_nShadowSampleCount = 1, float p_fReflectEpsilon = 1E-1f);

			bool Initialise(Scene *p_pScene, ICamera *p_pCamera);
			bool Shutdown(void);

			bool Prepare(Scene *p_pScene);

			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, RadianceContext *p_pRadianceContext = NULL);
			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, Intersection &p_intersection, RadianceContext *p_pRadianceContext = NULL);
		
			std::string ToString(void) const;
		};
	}
}