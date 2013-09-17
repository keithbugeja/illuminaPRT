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

		//----------------------------------------------------------------------------------------------
		// Irradiance Cache Node
		//----------------------------------------------------------------------------------------------
		struct IrradianceCacheNode
		{
			AxisAlignedBoundingBox Bounds;
			IrradianceCacheNode *Children;
			std::vector<IrradianceCacheRecord*> RecordList;

			IrradianceCacheNode(void) 
				// : Children (nullptr)
				: Children(NULL)
			{ }

			void Add(IrradianceCacheRecord *p_pRecord)
			{
				RecordList.push_back(p_pRecord);
			}
		};

		//----------------------------------------------------------------------------------------------
		// Irradiance Cache
		//----------------------------------------------------------------------------------------------
		class IrradianceCache
		{
		protected:
			std::vector<IrradianceCacheRecord*> m_irradianceRecordList;
			float m_fErrorThreshold;

		public:
			int m_nInsertCount,
				m_nRecordCount,
				m_nNodeCount;

		public:
			IrradianceCacheNode RootNode;

		protected:
			float W_Ward(const Vector3 &p_point, const Vector3 &p_normal, IrradianceCacheRecord &p_record);
			float W_Tabelion(const Vector3 &p_point, const Vector3 &p_normal, IrradianceCacheRecord &p_record);

		public:
			IrradianceCache(void) 
				: m_nInsertCount(0)
				, m_nRecordCount(0)
				, m_nNodeCount(0)
			{ }

			int CountNodes(IrradianceCacheNode* p_pNode) const;

			void SetErrorThreshold(float p_fErrorThreshold) { m_fErrorThreshold = p_fErrorThreshold; }

			void SetBounds(const AxisAlignedBoundingBox &p_parent, 
				int p_nChildIndex, AxisAlignedBoundingBox &p_child);
			
			bool SphereBoxOverlap(const AxisAlignedBoundingBox &p_aabb,
				const Vector3& p_centre, const float p_fRadius) const;
			
			bool FindRecords(const Vector3 &p_point, const Vector3 &p_normal, 
				std::vector<std::pair<float, IrradianceCacheRecord*>>& p_nearbyRecordList);

			void Insert(IrradianceCacheNode *p_pNode, IrradianceCacheRecord *p_pRecord, int p_nDepth);

			float W(const Vector3 &p_point, const Vector3 &p_normal, IrradianceCacheRecord &p_record);

			void Merge(IrradianceCache *p_pIrradianceCache);

			std::string ToString(void) const;
		};

		//----------------------------------------------------------------------------------------------
		// Irradiance Cache Integrator
		//----------------------------------------------------------------------------------------------
		class ICIntegrator : 
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

			IrradianceCache m_irradianceCache;
			std::vector<IrradianceCacheRecord*> m_irradianceCacheRecordList;

		protected:
			Spectrum PathLi(Scene *p_pScene, Ray &p_ray);
			Spectrum GetIrradiance(const Intersection &p_intersection, Scene *p_pScene);
			void ComputeRecord(const Intersection &p_intersection, Scene *p_pScene, IrradianceCacheRecord &p_record);
			IrradianceCacheRecord* RequestRecord(void);
			void ReleaseRecords(void);

		public:
			ICIntegrator(const std::string &p_strName, int p_nCacheDepth, float p_fErrorThreshold, float p_fAmbientResolution, float p_fAmbientMultipler,
				int p_nAzimuthStrata, int p_nAltitudeStrata, int p_nRayDepth, int p_nShadowRays = 1, float p_fReflectEpsilon = 1e-1f);

			ICIntegrator(int p_nCacheDepth, float p_fErrorThreshold, float p_fAmbientResolution, float p_fAmbientMultipler,
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