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
		struct IrradianceCacheRecord
		{
			Vector3 Point,
				Normal;

			Spectrum Irradiance;
			
			float RiClamp, Ri, Rmin, Rmax;
		};

		struct IrradianceCacheNode
		{
			AxisAlignedBoundingBox Bounds;
			IrradianceCacheNode *Children;
			std::vector<IrradianceCacheRecord> RecordList;

			IrradianceCacheNode(void) 
				: RecordList(128)
				: Children {nullptr}
			{ }

			void Add(IrradianceCacheRecord *p_pRecord)
			{
				RecordList.push_back(*p_pRecord);
			}
		};

		struct IrradianceCache
		{
			IrradianceCacheNode RootNode;

			void Insert(IrradianceCacheNode *p_pNode, IrradianceCacheRecord *p_pRecord)
			{
				p_pNode.Add(p_pRecord);
				
				if (p_pNode->GetRadius() > p_pRecord->RiClamp)
				{
					if (p_pNode->Children == nullptr)
						p_pNode->Children = new IrradianceCacheNode[8];

					for (int i = 0; i < 8; i++)
					{
						Insert(p_pNode->Children[i], p_pRecord);
					}
				}
			}
		};

		class ICIntegrator : 
			public IIntegrator
		{
		protected:
			int m_nMaxVPL,
				m_nMaxPath,
				m_nTileWidth,
				m_nTileArea,
				m_nMaxRayDepth,
				m_nShadowSampleCount,
				m_nIndirectSampleCount;

			float m_fReflectEpsilon,
				m_fGTermMax;

		public:
			std::vector<std::vector<VirtualPointLight>> VirtualPointLightSet;
			std::vector<VirtualPointLight> VirtualPointLightList;

		public:
			IGIIntegrator(const std::string &p_strName, int p_nMaxVPL = 256, int p_nMaxPath = 8, int p_nTileWidth = 3, float p_fGTermMax = 0.001f, int p_nMaxRayDepth = 4, int p_nShadowSampleCount = 1, int p_nIndirectSampleCount = 1, float p_fReflectEpsilon = 1E-1f);
			IGIIntegrator(int p_nMaxVPL = 256, int p_nMaxPath = 8, int p_nTileWidth = 3, float p_fGTermMax = 0.001f, int p_nMaxRayDepth = 4, int p_nShadowSampleCount = 1, int p_nIndirectSampleCount = 1, float p_fReflectEpsilon = 1E-1f);

			bool Initialise(Scene *p_pScene, ICamera *p_pCamera);
			bool Shutdown(void);

			bool Prepare(Scene *p_pScene);

			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, RadianceContext *p_pRadianceContext = NULL);
			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, Intersection &p_intersection, RadianceContext *p_pRadianceContext = NULL);
		
		protected:
			void TraceVirtualPointLights(Scene *p_pScene, int p_nMaxPaths, int p_nMaxPointLights, int p_nMaxBounces, std::vector<VirtualPointLight> &p_virtualPointLightList);
			void TraceVPLs(Scene *p_pScene, int p_nLightIdx, int p_nVPLPaths, int p_nMaxVPLs, int p_nMaxBounces, std::vector<VirtualPointLight> &p_vplList);
		};
	}
}