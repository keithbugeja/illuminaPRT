//----------------------------------------------------------------------------------------------
//	Filename:	MLICIntegrator.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Integrator/Integrator.h"
#include "Integrator/IntegratorHelper.h"
#include "Geometry/Intersection.h"
#include "Geometry/BoundingBox.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		// Irradiance Cache Node
		//----------------------------------------------------------------------------------------------
		struct MLIrradianceCacheRecord
		{
			Spectrum Irradiance;

			Vector3 Position, 
				Normal;
			
			float RiClamp, 
				Ri;

			int Epoch;

			MLIrradianceCacheRecord(void) { }

			MLIrradianceCacheRecord(const MLIrradianceCacheRecord& p_record) {
				memcpy(this, &p_record, sizeof(MLIrradianceCacheRecord));
			}

			MLIrradianceCacheRecord& operator=(const MLIrradianceCacheRecord& p_record) {
				memcpy(this, &p_record, sizeof(MLIrradianceCacheRecord));
				return *this;
			}
		};

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
			WaitFreeList<MLIrradianceCacheRecord*> m_irradianceRecordList;
			
			float m_fErrorThreshold;
			
			int m_nDepth,
				m_nEpoch;

		public:
			int m_nInsertCount,
				m_nRecordCount,
				m_nInsertRejectCount,
				m_nInsertAcceptCount,
				m_nNodeCount;

		public:
			MLIrradianceCacheNode RootNode;

		protected:
			float W_Ward(const Vector3 &p_point, const Vector3 &p_normal, MLIrradianceCacheRecord &p_record);
			float W_Tabelion(const Vector3 &p_point, const Vector3 &p_normal, MLIrradianceCacheRecord &p_record);
			float W_Debattista(const Vector3 &p_point, const Vector3 &p_normal, MLIrradianceCacheRecord &p_record);

		public:
			MLIrradianceCache(void) 
				: m_nInsertCount(0)
				, m_nInsertRejectCount(0)
				, m_nInsertAcceptCount(0)
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
			void InsertPoisson(MLIrradianceCacheRecord *p_pRecord, float p_fMinDistance);
			void InsertPoisson(MLIrradianceCacheNode *p_pNode, MLIrradianceCacheRecord *p_pRecord, float p_fMinDistance, int p_nDepth);

			void Merge(MLIrradianceCache *p_pIrradianceCache);

			std::string ToString(void);
		};

		//----------------------------------------------------------------------------------------------
		// Irradiance Cache Integrator
		//----------------------------------------------------------------------------------------------
		class MLICIntegrator : 
			public IIntegrator
		{
		protected:
			IntegratorHelper<MLIrradianceCacheRecord> m_helper;
	
			// Irradiance cache
			MLIrradianceCache m_irradianceCache;

			// Irradiance record list
			WaitFreeList<MLIrradianceCacheRecord*> m_irradianceCacheRecordList;

			// Irradiance cache rendering parameters
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

			// Sample creation epoch
			int	m_nEpoch;

			// Disable creation of new samples
			bool m_bIsSampleGenerationDisabled;

			int m_nGenerationCount,
				m_nInsertionCount;

		protected:
			// Mini-path tracer
			Spectrum PathLi(Scene *p_pScene, Ray &p_ray);
			
			// Return Irradiance value at intersection
			Spectrum GetIrradiance(const Intersection &p_intersection, Scene *p_pScene);
			
			// Compute a new irradiance record
			void ComputeRecord(const Intersection &p_intersection, Scene *p_pScene, MLIrradianceCacheRecord &p_record);
			
		public:
			// Request allocation of a new irradiance record
			MLIrradianceCacheRecord* RequestRecord(void);
			MLIrradianceCacheRecord* RequestRecord(MLIrradianceCacheRecord* p_pRecord, int p_nEpoch = -1);

			// Release all allocated irradiance records
			void ReleaseRecords(void);

		public:
			// Return irradiance cache for direct manipulation outside of integrator
			MLIrradianceCache *GetIrradianceCache(void) { return &m_irradianceCache; }

			// Return irradiance samples by their epoch index
			void GetByEpoch(int p_nEpoch, std::vector<MLIrradianceCacheRecord*> &p_recordList);
			void GetByEpochRange(int p_nEpochMin, int p_nEpochMax, std::vector<MLIrradianceCacheRecord*> &p_recordList);
			bool HasEpochQuota(int p_nQuota);

			// Move to next epoch
			int NextEpoch(void) { return m_nEpoch++; }
			
			// Return current epoch
			int GetEpoch(void) { return m_nEpoch; }

			float GetMinRadius(void) { return m_fRMin * 0.1f; }
			float GetMaxRadius(void) { return m_fRMax; }

			// Disable sample generation
			void DisableSampleGeneration(bool p_bDisable) { m_bIsSampleGenerationDisabled = p_bDisable; }

		public:
			MLICIntegrator(const std::string &p_strName, int p_nCacheDepth, float p_fErrorThreshold, float p_fAmbientResolution, float p_fAmbientMultipler,
				int p_nAzimuthStrata, int p_nAltitudeStrata, int p_nRayDepth, int p_nShadowRays = 1, float p_fReflectEpsilon = 1e-1f);

			MLICIntegrator(int p_nCacheDepth, float p_fErrorThreshold, float p_fAmbientResolution, float p_fAmbientMultipler,
				int p_nAzimuthStrata, int p_nAltitudeStrata, int p_nRayDepth, int p_nShadowRays = 1, float p_fReflectEpsilon = 1e-1f);

			std::string GetType(void) const { return "WFIC"; }

			bool Initialise(Scene *p_pScene, ICamera *p_pCamera);
			bool Shutdown(void);

			bool Prepare(Scene *p_pScene);

			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, RadianceContext *p_pRadianceContext = NULL);
			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, Intersection &p_intersection, RadianceContext *p_pRadianceContext = NULL);
		
			std::string ToString(void);
		};
	}
}