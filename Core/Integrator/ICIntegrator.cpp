//----------------------------------------------------------------------------------------------
//	Filename:	ICIntegrator.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iostream>
#include "Integrator/ICIntegrator.h"

#include "Geometry/Ray.h"
#include "Geometry/BoundingBox.h"
#include "Geometry/Intersection.h"
#include "Sampler/JitterSampler.h"
#include "Material/Material.h"
#include "Spectrum/Spectrum.h"
#include "Scene/Visibility.h"
#include "Scene/Primitive.h"
#include "Scene/Scene.h"

#include "Sampler/QuasiRandom.h"
#include "Maths/Montecarlo.h"

using namespace Illumina::Core;
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
			
			float RiClamp, Ri;

			IrradianceCacheRecord(void) { }

			IrradianceCacheRecord(const IrradianceCacheRecord& p_record)
			{
				memcpy(this, &p_record, sizeof(IrradianceCacheRecord));
			}

			IrradianceCacheRecord& operator=(const IrradianceCacheRecord& p_record)
			{
				memcpy(this, &p_record, sizeof(IrradianceCacheRecord));
				return *this;
			}
		};
	}
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
int IrradianceCache::CountNodes(IrradianceCacheNode* p_pNode) const
{
	int nodes = 1;

	if (p_pNode->Children != nullptr)
	{
		for (int i = 0; i < 8; i++)
			nodes += CountNodes(p_pNode->Children + i); 
	}

	return nodes;
}
//----------------------------------------------------------------------------------------------
void IrradianceCache::SetBounds(const AxisAlignedBoundingBox &p_parent, int p_nChildIndex, AxisAlignedBoundingBox &p_child)
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
//----------------------------------------------------------------------------------------------
bool IrradianceCache::SphereBoxOverlap(const AxisAlignedBoundingBox &p_aabb,
	const Vector3& p_centre, const float p_fRadius) const
{
	return (p_centre.X + p_fRadius > p_aabb.GetMinExtent(0) &&
			p_centre.X - p_fRadius < p_aabb.GetMaxExtent(0) &&
			p_centre.Y + p_fRadius > p_aabb.GetMinExtent(1) &&
			p_centre.Y - p_fRadius < p_aabb.GetMaxExtent(1) &&
			p_centre.Z + p_fRadius > p_aabb.GetMinExtent(2) &&
			p_centre.Z - p_fRadius < p_aabb.GetMaxExtent(2));
}
//----------------------------------------------------------------------------------------------
void IrradianceCache::Insert(IrradianceCacheNode *p_pNode, IrradianceCacheRecord *p_pRecord, int p_nDepth)
{
	if (p_nDepth <= 0 || p_pRecord->RiClamp > p_pNode->Bounds.GetExtent().MaxAbsComponent()) // p_pNode->Bounds.GetRadius())
	{
		m_nRecordCount++;
		p_pNode->Add(p_pRecord);
	} 
	else
	{
		// This node has no children allocated
		if (p_pNode->Children == nullptr)
		{
			m_nNodeCount+=8;
			p_pNode->Children = new IrradianceCacheNode[8];

			for (int i = 0; i < 8; i++)
			{
				SetBounds(p_pNode->Bounds, i, p_pNode->Children[i].Bounds);
				if (SphereBoxOverlap(p_pNode->Children[i].Bounds, p_pRecord->Point, p_pRecord->RiClamp))
					Insert(p_pNode->Children + i, p_pRecord, p_nDepth - 1);
			}
		}
		else
		{
			for (int i = 0; i < 8; i++)
			{
				if (SphereBoxOverlap(p_pNode->Children[i].Bounds, p_pRecord->Point, p_pRecord->RiClamp))
					Insert(p_pNode->Children + i, p_pRecord, p_nDepth - 1);
			}
		}
	}
}
//----------------------------------------------------------------------------------------------
bool IrradianceCache::FindRecords(const Vector3 &p_point, const Vector3 &p_normal, 
	std::vector<std::pair<float, IrradianceCacheRecord*>>& p_nearbyRecordList)
{
	IrradianceCacheNode *pNode = &RootNode;

	float wi;

	if (pNode->Bounds.Contains(p_point))
	{
		while (pNode != nullptr)
		{
			for (auto r : pNode->RecordList)
			{
				// Race!
				if ((wi = W(p_point, p_normal, *r)) > 0.f)
					p_nearbyRecordList.push_back(std::pair<float, IrradianceCacheRecord*>(wi, r));
				// Race!
			}

			if ((pNode = pNode->Children) == nullptr)
				break;

			while(!pNode->Bounds.Contains(p_point)) pNode++;
		}
	}

	return true;
}
//----------------------------------------------------------------------------------------------
float IrradianceCache::W(const Vector3 &p_point, const Vector3 &p_normal, IrradianceCacheRecord &p_record)
{
	/* 
	Vector3 P = p_record.Point - p_point;
	float d = Vector3::Dot(P, (p_normal + p_record.Normal) * 0.5f);

	if (d < 0.05f) return -1;
	*/

	/* */
	// OK - Ward
	// float dist = Vector3::Distance(p_point, p_record.Point) / p_record.Ri;
	float dist = Vector3::Distance(p_point, p_record.Point) / p_record.RiClamp;
	float norm = Maths::Sqrt(1 - Vector3::Dot(p_normal, p_record.Normal));
	float alpha = m_fErrorThreshold;

	return (1.f / (dist + norm)) - alpha;
	/* */

	/* 
	// Tabellion
	float epi = Vector3::Distance(p_point, p_record.Point) / (p_record.RiClamp * 0.5f);
	float eni = Maths::Sqrt(1 - Vector3::Dot(p_normal, p_record.Normal)) * 8.0f;
	float K = m_fErrorThreshold;

	return 1 - K * Maths::Max(epi, eni);
	/* */
}
//----------------------------------------------------------------------------------------------
std::string IrradianceCache::ToString(void) const
{
	std::stringstream output;

	output 
		//<< "Inserts : [ " << m_nInsertCount << "]" << std::endl 
		<< "Records : [" << m_nRecordCount  << "]" << std::endl 
		<< "Nodes : [" << m_nNodeCount<< "]" << std::endl;
		//<< "Counted Nodes : [" << CountNodes((IrradianceCacheNode*)&RootNode) << "]" << std::endl;
	
	return output.str();
}

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
ICIntegrator::ICIntegrator(const std::string &p_strName, 
						   int p_nCacheDepth, float p_fErrorThreshold, 
						   float p_fAmbientResolution, float p_fAmbientMultiplier,  
						   int p_nAzimuthStrata, int p_nAltitudeStrata, 
						   int p_nRayDepth, int p_nShadowRays, 
						   float p_fReflectEpsilon)
	: IIntegrator(p_strName)
	, m_nCacheDepth(p_nCacheDepth)
	, m_fErrorThreshold(p_fErrorThreshold)
	, m_fAmbientResolution(p_fAmbientResolution)
	, m_fAmbientMultiplier(p_fAmbientMultiplier)
	, m_nAzimuthStrata(p_nAzimuthStrata)
	, m_nAltitudeStrata(p_nAltitudeStrata)
	, m_nRayDepth(p_nRayDepth)
	, m_nShadowRays(p_nShadowRays)
	, m_fReflectEpsilon(p_fReflectEpsilon)
{ }
//----------------------------------------------------------------------------------------------
ICIntegrator::ICIntegrator(int p_nCacheDepth, float p_fErrorThreshold, 
						   float p_fAmbientResolution, float p_fAmbientMultiplier,
						   int p_nAzimuthStrata, int p_nAltitudeStrata, 
						   int p_nRayDepth, int p_nShadowRays, 
						   float p_fReflectEpsilon)
	: IIntegrator()
	, m_nCacheDepth(p_nCacheDepth)
	, m_fErrorThreshold(p_fErrorThreshold)
	, m_fAmbientResolution(p_fAmbientResolution)
	, m_fAmbientMultiplier(p_fAmbientMultiplier)
	, m_nAzimuthStrata(p_nAzimuthStrata)
	, m_nAltitudeStrata(p_nAltitudeStrata)
	, m_nRayDepth(p_nRayDepth)
	, m_nShadowRays(p_nShadowRays)
	, m_fReflectEpsilon(p_fReflectEpsilon)
{ }
//----------------------------------------------------------------------------------------------
bool ICIntegrator::Initialise(Scene *p_pScene, ICamera *p_pCamera)
{
	BOOST_ASSERT(p_pScene != nullptr && p_pScene->GetSpace() != nullptr);

	ISpace *pSpace = p_pScene->GetSpace(); 
	
	pSpace->Initialise();
	pSpace->Build();

	Vector3 centroid = pSpace->GetBoundingVolume()->GetCentre();
	Vector3 longestEdge = Vector3(pSpace->GetBoundingVolume()->GetSize().MaxAbsComponent());

	Vector3 pointList[2];
	pointList[0] = centroid - longestEdge;
	pointList[1] = centroid + longestEdge;

	pointList[0] *= (1.0f + Maths::Epsilon);
	pointList[1] *= (1.0f + Maths::Epsilon);

	m_irradianceCache.RootNode.Bounds.ComputeFromPoints((Vector3*)&pointList, 2);

	m_fRMin = longestEdge.X * m_fAmbientResolution;
	m_fRMax = longestEdge.X * m_fAmbientMultiplier;

	m_irradianceCache.SetErrorThreshold(m_fErrorThreshold);

	return true;
}
//----------------------------------------------------------------------------------------------
bool ICIntegrator::Shutdown(void)
{
	return true;
}
//----------------------------------------------------------------------------------------------
bool ICIntegrator::Prepare(Scene *p_pScene)
{
	return true;
}
//----------------------------------------------------------------------------------------------
Spectrum ICIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, Intersection &p_intersection, RadianceContext *p_pRadianceContext)
{
	Vector3 wIn;

	// Avoid having to perform multiple checks for a NULL radiance context
	RadianceContext radianceContext;

	if (p_pRadianceContext == NULL)
		p_pRadianceContext = &radianceContext;
	
	// Initialise context
	p_pRadianceContext->Flags = 0;

	p_pRadianceContext->Indirect = 
		p_pRadianceContext->Direct = 
		p_pRadianceContext->Albedo = 0.f;

	if (p_intersection.IsValid())
	{
		if (p_intersection.HasMaterial()) 
		{
			// Get material for intersection primitive
			IMaterial *pMaterial = p_intersection.GetMaterial();

			// Set wOut to eye ray direction vector
			Vector3 wOut = -Vector3::Normalize(p_intersection.Surface.RayDirectionWS);

			// Start populating radiance context
			p_pRadianceContext->SetSpatialContext(&p_intersection);
			
			if (!p_intersection.IsEmissive())
			{
				// Sample direct lighting
				p_pRadianceContext->Direct = SampleAllLights(p_pScene, p_intersection, 
					p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, 
					wOut, p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowRays);

				// Set albedo
				p_pRadianceContext->Albedo = pMaterial->Rho(wOut, p_intersection.Surface);
				
				// Set indirect 
				p_pRadianceContext->Indirect = GetIrradiance(p_intersection, p_pScene) * p_pRadianceContext->Albedo * Maths::InvPi;
			}
			else
			{
				p_pRadianceContext->Direct = p_intersection.GetLight()->Radiance(p_intersection.Surface.PointWS, 
					p_intersection.Surface.GeometryBasisWS.W, wOut);
			}
		}
	}
	else
	{
		/*
		Ray ray(p_intersection.Surface.RayOriginWS, -p_intersection.Surface.RayDirectionWS);

		for (size_t lightIndex = 0; lightIndex < p_pScene->LightList.Size(); ++lightIndex)
			p_pRadianceContext->Direct += p_pScene->LightList[lightIndex]->Radiance(ray);
		*/

		p_pRadianceContext->Direct.Set(20, 50, 100);
	}

	// Populate radiance context
	p_pRadianceContext->Flags |= RadianceContext::DF_Albedo |  
		RadianceContext::DF_Direct | RadianceContext::DF_Indirect;
	
	return /* p_pRadianceContext->Direct + */ p_pRadianceContext->Indirect;
}
//----------------------------------------------------------------------------------------------
Spectrum ICIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, RadianceContext *p_pRadianceContext)
{
	// Compute intersection step
	if (!p_pScene->Intersects(Ray(p_ray), p_intersection))
	{
		p_intersection.Surface.RayOriginWS = p_ray.Origin;
		p_intersection.Surface.RayDirectionWS = p_ray.Direction;
	}

	return Radiance(p_pContext, p_pScene, p_intersection, p_pRadianceContext);
}
//----------------------------------------------------------------------------------------------
Spectrum ICIntegrator::GetIrradiance(const Intersection &p_intersection, Scene *p_pScene)
{
	std::vector<std::pair<float, IrradianceCacheRecord*>> nearbyRecordList;

	// Find nearyby records
	m_irradianceCache.FindRecords(p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, nearbyRecordList);
	
	if (nearbyRecordList.size() > 0)
	{
		Spectrum num = 0.f;
		float den = 0.f;

		for (auto pair : nearbyRecordList)
		{
			num += pair.second->Irradiance * pair.first;
			den += pair.first;
		}

		num /= den;
		return num;
	}
	else
	{
		IrradianceCacheRecord *r = new IrradianceCacheRecord();
		ComputeRecord(p_intersection, p_pScene, *r);
		m_irradianceCache.Insert(&(m_irradianceCache.RootNode), r, m_nCacheDepth);
		return r->Irradiance;
	}
}
//----------------------------------------------------------------------------------------------
void ICIntegrator::ComputeRecord(const Intersection &p_intersection, Scene *p_pScene, IrradianceCacheRecord &p_record)
{
	Intersection isect;
	Vector2 sample2D;
	Vector3 wOutR;
	Ray ray;

	Spectrum E = 0;
	
	float totLength = 0,
		minLength = Maths::Maximum;

	for (int altitudeIndex = 0; altitudeIndex < m_nAltitudeStrata; altitudeIndex++)
	{
		for (int azimuthIndex = 0; azimuthIndex < m_nAzimuthStrata; azimuthIndex++)
		{
			// Get samples for initial position and direction
			/* 
			sample2D.X = QuasiRandomSequence::VanDerCorput(rayIndex);
			sample2D.Y = QuasiRandomSequence::Sobol2(rayIndex);
			/* */

			/* */
			sample2D.X = p_pScene->GetSampler()->Get1DSample();
			sample2D.Y = p_pScene->GetSampler()->Get1DSample();
			/* */

			Vector3 vH = 
				// Montecarlo::UniformSampleSphere(sample2D.U, sample2D.V);
				Montecarlo::CosineSampleHemisphere(sample2D.X, sample2D.Y, altitudeIndex, azimuthIndex, m_nAltitudeStrata, m_nAzimuthStrata); 

			BSDF::SurfaceToWorld(p_intersection.WorldTransform, p_intersection.Surface, vH, wOutR); 

			ray.Set(p_intersection.Surface.PointWS + wOutR * m_fReflectEpsilon, wOutR, m_fReflectEpsilon, Maths::Maximum);

			p_pScene->Intersects(ray, isect);
		
			E += SampleAllLights(p_pScene, isect, 
					isect.Surface.PointWS, isect.Surface.ShadingBasisWS.W, wOutR, 
					p_pScene->GetSampler(), isect.GetLight(), m_nShadowRays);

			totLength += 1.f / isect.Surface.Distance;
			// totLength += isect.Surface.Distance;
			minLength = Maths::Min(isect.Surface.Distance, minLength);
		}
	}

	int mn = m_nAzimuthStrata * m_nAltitudeStrata;

	p_record.Point = p_intersection.Surface.PointWS;
	p_record.Normal = p_intersection.Surface.ShadingBasisWS.W;
	p_record.Irradiance = E / mn;
	p_record.Ri = /*totLength / mn; */ mn / totLength; 
	p_record.RiClamp = Maths::Max(p_record.Ri, m_fRMin);
	p_record.RiClamp = Maths::Min(p_record.RiClamp, m_fRMax);

	//p_record.Ri = minLength; 
	//p_record.RiClamp = p_record.Ri; //Maths::Max(m_fRMin, Maths::Min(m_fRMax, p_record.Ri));
}
//----------------------------------------------------------------------------------------------
std::string ICIntegrator::ToString(void) const
{
	return m_irradianceCache.ToString();
}