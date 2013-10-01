//----------------------------------------------------------------------------------------------
//	Filename:	MLICIntegrator.h
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
		struct MLIrradianceCacheRecord;

		//----------------------------------------------------------------------------------------------
		// Irradiance Cache Node
		//----------------------------------------------------------------------------------------------
		struct MLIrradianceCacheNode
		{
			AxisAlignedBoundingBox Bounds;
			MLIrradianceCacheNode *Children;
			// std::vector<MLIrradianceCacheRecord*> RecordList;
			WaitFreeList<MLIrradianceCacheRecord*> RecordList;

			MLIrradianceCacheNode(void) 
				: Children(NULL)
			{ }

			void Add(MLIrradianceCacheRecord *p_pRecord) {
				RecordList.push_back(p_pRecord);
			}
		};

		//----------------------------------------------------------------------------------------------
		// Irradiance Cache
		//----------------------------------------------------------------------------------------------
		class MLIrradianceCache
		{
		protected:
			// std::vector<MLIrradianceCacheRecord*> m_irradianceRecordList;
			WaitFreeList<MLIrradianceCacheRecord*> m_irradianceRecordList;
			
			float m_fErrorThreshold;
			
			int m_nDepth,
				m_nEpoch;

		public:
			int m_nInsertCount,
				m_nRecordCount,
				m_nNodeCount;

		public:
			MLIrradianceCacheNode RootNode;

		protected:
			float W_Ward(const Vector3 &p_point, const Vector3 &p_normal, MLIrradianceCacheRecord &p_record);
			float W_Tabelion(const Vector3 &p_point, const Vector3 &p_normal, MLIrradianceCacheRecord &p_record);

		public:
			MLIrradianceCache(void) 
				: m_nInsertCount(0)
				, m_nRecordCount(0)
				, m_nNodeCount(0)
				, m_nEpoch(0)
				, m_nDepth(8)
				, m_fErrorThreshold(0.35f)
			{ }

			int CountNodes(MLIrradianceCacheNode* p_pNode) const;

			int GetEpoch(void) { return m_nEpoch; }
			void NextEpoch(void) { m_nEpoch++; }
			void SetEpoch(int p_nEpoch) { m_nEpoch = p_nEpoch; }
			void SetDepth(int p_nDepth) { m_nDepth = p_nDepth; }
			void SetErrorThreshold(float p_fErrorThreshold) { m_fErrorThreshold = p_fErrorThreshold; }
			void SetBounds(const AxisAlignedBoundingBox &p_parent, int p_nChildIndex, AxisAlignedBoundingBox &p_child);
			
			float W(const Vector3 &p_point, const Vector3 &p_normal, MLIrradianceCacheRecord &p_record);
			bool SphereBoxOverlap(const AxisAlignedBoundingBox &p_aabb, const Vector3& p_centre, const float p_fRadius) const;			
			bool FindRecords(const Vector3 &p_point, const Vector3 &p_normal, std::vector<std::pair<float, MLIrradianceCacheRecord*>>& p_nearbyRecordList);

			void Insert(MLIrradianceCacheRecord *p_pRecord);
			void Insert(MLIrradianceCacheNode *p_pNode, MLIrradianceCacheRecord *p_pRecord, int p_nDepth);

			void Merge(MLIrradianceCache *p_pIrradianceCache);

			std::string ToString(void) const;
		};

		//----------------------------------------------------------------------------------------------
		// Irradiance Cache Integrator
		//----------------------------------------------------------------------------------------------
		class MLICIntegrator : 
			public IIntegrator
		{
		protected:
			int m_nRayDepth,
				m_nShadowRays,
				m_nCacheDepth,
				m_nAzimuthStrata,
				m_nAltitudeStrata;

			float m_fErrorThreshold,
				m_fAmbientResolution,
				m_fAmbientMultiplier,
				m_fReflectEpsilon;

			float m_fRMin, 
				m_fRMax;

			int	m_nEpoch;

			MLIrradianceCache m_irradianceCache;
			// std::vector<MLIrradianceCacheRecord*> m_irradianceCacheRecordList;
			WaitFreeList<MLIrradianceCacheRecord*> m_irradianceCacheRecordList;

		protected:
			Spectrum PathLi(Scene *p_pScene, Ray &p_ray);
			Spectrum GetIrradiance(const Intersection &p_intersection, Scene *p_pScene);
			void ComputeRecord(const Intersection &p_intersection, Scene *p_pScene, MLIrradianceCacheRecord &p_record);
			MLIrradianceCacheRecord* RequestRecord(void);
			void ReleaseRecords(void);

		public:
			MLIrradianceCache *GetIrradianceCache(void) { return &m_irradianceCache; }

			void GetByEpoch(int p_nEpoch, std::vector<MLIrradianceCacheRecord*> &p_recordList) { }
			void DiscardByEpoch(int p_nEpoch) { }

		public:
			MLICIntegrator(const std::string &p_strName, int p_nCacheDepth, float p_fErrorThreshold, float p_fAmbientResolution, float p_fAmbientMultipler,
				int p_nAzimuthStrata, int p_nAltitudeStrata, int p_nRayDepth, int p_nShadowRays = 1, float p_fReflectEpsilon = 1e-1f);

			MLICIntegrator(int p_nCacheDepth, float p_fErrorThreshold, float p_fAmbientResolution, float p_fAmbientMultipler,
				int p_nAzimuthStrata, int p_nAltitudeStrata, int p_nRayDepth, int p_nShadowRays = 1, float p_fReflectEpsilon = 1e-1f);

			bool Initialise(Scene *p_pScene, ICamera *p_pCamera);
			bool Shutdown(void);

			bool Prepare(Scene *p_pScene);

			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, RadianceContext *p_pRadianceContext = NULL);
			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, Intersection &p_intersection, RadianceContext *p_pRadianceContext = NULL);
		
			std::string ToString(void) const;
		};
	}
}