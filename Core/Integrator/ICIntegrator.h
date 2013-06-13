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
				: RecordList(0)
				, Children (nullptr)
			{ }

			void Add(IrradianceCacheRecord *p_pRecord)
			{
				RecordList.push_back(*p_pRecord);
			}
		};

		struct IrradianceCache
		{
			IrradianceCacheNode RootNode;

			inline void SetBounds(const AxisAlignedBoundingBox &p_parent, int p_nChildIndex, AxisAlignedBoundingBox &p_child)
			{
				Vector3 minExt (p_parent.GetMinExtent()),
						maxExt (p_parent.GetMaxExtent()),
						ctr(p_parent.GetCentre());

				switch (p_nChildIndex)
				{
					case 0:
						p_child.SetExtents(minExt, ctr);
						break;

					case 1:
						p_child.SetExtents(Vector3(ctr.X, minExt.Y, minExt.Z), Vector3(maxExt.X, ctr.Y, ctr.Z));
						break;

					case 2:
						p_child.SetExtents(Vector3(minExt.X, minExt.Y, ctr.Z), Vector3(ctr.X, ctr.Y, maxExt.Z));
						break;

					case 3:
						p_child.SetExtents(Vector3(ctr.X, minExt.Y, ctr.Z), Vector3(maxExt.X, ctr.Y, maxExt.Z));
						break;

					case 4:
						p_child.SetExtents(Vector3(minExt.X, ctr.Y, minExt.Z), Vector3(ctr.X, maxExt.Y, ctr.Z));
						break;

					case 5:
						p_child.SetExtents(Vector3(ctr.X, ctr.Y, minExt.Z), Vector3(maxExt.X, maxExt.Y, ctr.Z));
						break;

					case 6:
						p_child.SetExtents(Vector3(minExt.X, ctr.Y, ctr.Z), Vector3(ctr.X, maxExt.Y, maxExt.Z));
						break;

					case 7:
						p_child.SetExtents(ctr, maxExt);
						break;
				}
			}

			void Insert(IrradianceCacheNode *p_pNode, IrradianceCacheRecord *p_pRecord)
			{
				p_pNode->Add(p_pRecord);
				
				if (p_pNode->Bounds.GetRadius() > p_pRecord->RiClamp)
				{
					if (p_pNode->Children == nullptr)
					{
						Vector3 minExt (p_pNode->Bounds.GetMinExtent()),
							maxExt (p_pNode->Bounds.GetMaxExtent()),
							ctr(p_pNode->Bounds.GetCentre());

						p_pNode->Children = new IrradianceCacheNode[8];
						
						for (int i = 0; i < 8; i++)
							SetBounds(p_pNode->Bounds, i, p_pNode->Children[i].Bounds);
							
					}

					for (int i = 0; i < 8; i++)
					{
						// Test containment
						Insert(p_pNode->Children + i, p_pRecord);
					}
				}
			}
		};

		class ICIntegrator : 
			public IIntegrator
		{
		protected:
			int m_nShadowSampleCount,
				m_nIndirectSampleCount;

			float m_fReflectEpsilon;

		public:
			ICIntegrator(const std::string &p_strName, int p_nShadowSampleCount = 1, float p_fReflectEpsilon = 1E-1f);
			ICIntegrator(int p_nShadowSampleCount = 1, float p_fReflectEpsilon = 1E-1f);

			bool Initialise(Scene *p_pScene, ICamera *p_pCamera);
			bool Shutdown(void);

			bool Prepare(Scene *p_pScene);

			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, RadianceContext *p_pRadianceContext = NULL);
			Spectrum Radiance(IntegratorContext *p_pContext, Scene *p_pScene, Intersection &p_intersection, RadianceContext *p_pRadianceContext = NULL);
		};
	}
}