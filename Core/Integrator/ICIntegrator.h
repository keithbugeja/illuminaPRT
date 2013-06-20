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

			bool SphereBoxOverlap(const AxisAlignedBoundingBox &p_aabb,
				const Vector3& p_centre, const float p_fRadius) const
			{
				float dmin = 0;
				
				for(int i = 0; i < 3; i++)
				{
					if (p_centre.Element[i] < p_aabb.GetMinExtent(i)) dmin += Maths::Sqr(p_centre.Element[i] - p_aabb.GetMinExtent(i));
					else if (p_centre.Element[i] > p_aabb.GetMaxExtent(i)) dmin += Maths::Sqr(p_centre.Element[i] - p_aabb.GetMaxExtent(i));
				}
			
				return dmin <= p_fRadius*p_fRadius;
			}
			
			void Insert(IrradianceCacheNode *p_pNode, IrradianceCacheRecord *p_pRecord)
			{
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
						if (SphereBoxOverlap(p_pNode->Children[i].Bounds, p_pRecord->Point, p_pRecord->RiClamp))
							Insert(p_pNode->Children + i, p_pRecord);
					}
				}
				else
					p_pNode->Add(p_pRecord);
			}

			bool FindRecords(const Vector3 &p_point, const Vector3 &p_normal, 
				std::vector<std::pair<float, IrradianceCacheRecord*>>& p_nearbyRecordList)
			{
				IrradianceCacheNode *pNode = &RootNode;

				float wi;

				while (pNode != nullptr)
				{
					for (auto r : pNode->RecordList)
					{
						if ((wi = W(p_point, p_normal, r)) > 0)
							p_nearbyRecordList.push_back(std::pair<float, IrradianceCacheRecord*>(wi, &r));
					}
				}

				return true;
			}

			float W(const Vector3 &p_point, const Vector3 &p_normal, IrradianceCacheRecord &p_record)
			{
				float dist = Vector3::Distance(p_point, p_record.Point) / p_record.Ri;
				float norm = Maths::Sqrt(1 - Vector3::Dot(p_normal, p_record.Normal));

				return 1.f / (dist + norm);
			}
		};

		class ICIntegrator : 
			public IIntegrator
		{
		protected:
			int m_nRayDepth,
				m_nDivisions,
				m_nShadowSampleCount;

			float m_fReflectEpsilon;

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
		};
	}
}