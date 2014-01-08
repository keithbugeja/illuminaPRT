//----------------------------------------------------------------------------------------------
//	Filename:	MLICIntegrator.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iostream>
#include "Integrator/MLICIntegrator.h"

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

#define __INSTANT_CACHING__

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
int MLIrradianceCache::CountNodes(MLIrradianceCacheNode* p_pNode) const
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
void MLIrradianceCache::SetBounds(const AxisAlignedBoundingBox &p_parent, int p_nChildIndex, AxisAlignedBoundingBox &p_child)
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
bool MLIrradianceCache::SphereBoxOverlap(const AxisAlignedBoundingBox &p_aabb,
	const Vector3& p_centre, const float p_fRadius) const
{
	float dmin = 0;
	for( int i = 0; i < 3; i++ )
	{
		if (p_centre[i] < p_aabb.GetMinExtent(i) ) dmin += Maths::Sqr(p_centre[i] - p_aabb.GetMinExtent(i));
		else if (p_centre[i] > p_aabb.GetMaxExtent(i) ) dmin += Maths::Sqr(p_centre[i] - p_aabb.GetMaxExtent(i));
	}
	return dmin <= p_fRadius*p_fRadius;
}
//----------------------------------------------------------------------------------------------
void MLIrradianceCache::Insert(MLIrradianceCacheRecord *p_pRecord)
{
	Insert(&RootNode, p_pRecord, m_nDepth);
}
//----------------------------------------------------------------------------------------------
void MLIrradianceCache::Insert(MLIrradianceCacheNode *p_pNode, MLIrradianceCacheRecord *p_pRecord, int p_nDepth)
{
	if (p_nDepth <= 0 || p_pRecord->RiClamp > p_pNode->Bounds.GetExtent().MaxAbsComponent())
	{
		m_nRecordCount++;
		p_pNode->Add(p_pRecord);
	} 
	else
	{
		// This node has no children allocated
		if (p_pNode->Children == NULL)
		{
			MLIrradianceCacheNode *pTempNode = new MLIrradianceCacheNode[8];

			// Before we CAS, make sure the bounds of the node are correct!
			for (int i = 0; i < 8; i++)
				SetBounds(p_pNode->Bounds, i, pTempNode[i].Bounds);

			if (p_pNode->Children == (MLIrradianceCacheNode*)AtomicInt64::CompareAndSwap((Int64*)&(p_pNode->Children), (Int64)pTempNode, NULL))
			{
				std::cout << "CAS failed at new node!" << std::endl;
				delete [] pTempNode;
			}
			else
				m_nNodeCount+=8;

			for (int i = 0; i < 8; i++)
			{
				// We still aren't sure where the thread that made CAS fail stopped, 
				// so although redundant, we still set the bounds of the nodes
				// SetBounds(p_pNode->Bounds, i, p_pNode->Children[i].Bounds);
				if (SphereBoxOverlap(p_pNode->Children[i].Bounds, p_pRecord->Position, p_pRecord->RiClamp))
					Insert(p_pNode->Children + i, p_pRecord, p_nDepth - 1);
			}
		}
		else
		{
			for (int i = 0; i < 8; i++)
			{
				if (SphereBoxOverlap(p_pNode->Children[i].Bounds, p_pRecord->Position, p_pRecord->RiClamp))
					Insert(p_pNode->Children + i, p_pRecord, p_nDepth - 1);
			}
		}
	}
}
//----------------------------------------------------------------------------------------------
bool MLIrradianceCache::FindRecords(const Vector3 &p_point, const Vector3 &p_normal, 
	std::vector<std::pair<float, MLIrradianceCacheRecord*>>& p_nearbyRecordList)
{
	MLIrradianceCacheNode *pNode = &RootNode;

	float wi, minval = 1e+10, maxval = -1e+10;
	int rejected = 0;

	if (pNode->Bounds.Contains(p_point))
	{
		//while (pNode != nullptr)
		while(pNode != NULL)
		{
			// for (auto r : pNode->RecordList)
			for (auto iter = pNode->RecordList.begin(); iter != pNode->RecordList.end(); iter++)
			{
				MLIrradianceCacheRecord *r = *iter;

				wi = W(p_point, p_normal, *r);
				minval = Maths::Min(wi, minval);
				maxval = Maths::Max(wi, maxval);

				if (wi > 0.f)
					p_nearbyRecordList.push_back(std::pair<float, MLIrradianceCacheRecord*>(wi, r));
				else
					rejected++;
			}

			// if ((pNode = pNode->Children) == nullptr)
			if ((pNode = pNode->Children) == NULL)
				break;

			// Equivalent to three partition tests! Optimise!
			while(!pNode->Bounds.Contains(p_point)) pNode++;
		}
	}

	// std::cout << "MinMax : [" << minval << ":" << maxval << "], Rejected : [" << rejected << "], Accepted : [" << p_nearbyRecordList.size() << "]" << std::endl; 

	return true;
}
//----------------------------------------------------------------------------------------------
float MLIrradianceCache::W_Ward(const Vector3 &p_point, const Vector3 &p_normal, MLIrradianceCacheRecord &p_record)
{
	float dist = Vector3::Distance(p_point, p_record.Position) / p_record.RiClamp,
		cosTheta = Vector3::Dot(p_normal, p_record.Normal);

	if (cosTheta <= 0)
		return -1.f;
	
	float den = Maths::Max(Maths::Epsilon, dist + 1 - cosTheta);
	
	return (1.f / den) - (1.f / m_fErrorThreshold);

	// Also: cache reciprocal of error threshold
	// float norm = 1 - cosTheta;
	// return (1.f / (dist + 1 - cosTheta)) - (1.f / m_fErrorThreshold);
}
//----------------------------------------------------------------------------------------------
float MLIrradianceCache::W_Tabelion(const Vector3 &p_point, const Vector3 &p_normal, MLIrradianceCacheRecord &p_record)
{
	float cosMaxAngleDifference = 0.2f;

	float epi = Vector3::Distance(p_point, p_record.Position) / p_record.RiClamp;
	float eni = Maths::Sqrt((1 - Vector3::Dot(p_normal, p_record.Normal)) / 
							(1.f - cosMaxAngleDifference));

	float err = Maths::Max(epi, eni);

	return 1.f - (err * m_fErrorThreshold);
}
//----------------------------------------------------------------------------------------------
float MLIrradianceCache::W_Debattista(const Vector3 &p_point, const Vector3 &p_normal, MLIrradianceCacheRecord &p_record)
{
	float dist = Vector3::Distance(p_point, p_record.Position),
		cosTheta = Vector3::Dot(p_normal, p_record.Normal);

	if (cosTheta <= 0)
		return -1.f;

	float den = Maths::Max(Maths::Epsilon, dist + 1 - cosTheta);
	
	return (1.f / den) - (1.f / m_fErrorThreshold);

	// Also: cache reciprocal of error threshold
	// float norm = 1 - cosTheta;
	// return (1.f / (dist + 1 - cosTheta)) - (1.f / m_fErrorThreshold);
	// return (1.f / (dist + Maths::Sqrt(1 - cosTheta))) - (1.f / m_fErrorThreshold);
}
//----------------------------------------------------------------------------------------------
float MLIrradianceCache::W(const Vector3 &p_point, const Vector3 &p_normal, MLIrradianceCacheRecord &p_record)
{
#if (defined __INSTANT_CACHING__)
	return W_Debattista(p_point, p_normal, p_record);
#else
	return W_Ward(
	// return W_Tabelion(
		p_point, p_normal, p_record);
#endif
}
//----------------------------------------------------------------------------------------------
void MLIrradianceCache::Merge(MLIrradianceCache *p_pIrradianceCache)
{
	for (auto record : p_pIrradianceCache->m_irradianceRecordList)
		Insert(record);
}
//----------------------------------------------------------------------------------------------
std::string MLIrradianceCache::ToString(void) const
{
	std::stringstream output;

	output << std::endl << "[Wait-Free Irradiance Cache :: Stats]" << std::endl
		<< " :: Records : [" << m_nRecordCount  << "]" << std::endl 
		<< " :: Nodes : [" << m_nNodeCount<< "]" << std::endl;
		// << "Inserts : [ " << m_nInsertCount << "]" << std::endl 
		//<< "Counted Nodes : [" << CountNodes((IrradianceCacheNode*)&RootNode) << "]" << std::endl;
	
	return output.str();
}

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

// #define ___DEBUG_IC___

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
MLICIntegrator::MLICIntegrator(const std::string &p_strName, 
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
	, m_nEpoch(0)
	, m_bIsSampleGenerationDisabled(false)
{ }
//----------------------------------------------------------------------------------------------
MLICIntegrator::MLICIntegrator(int p_nCacheDepth, float p_fErrorThreshold, 
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
	, m_nEpoch(0)
	, m_bIsSampleGenerationDisabled(false)
{ }
//----------------------------------------------------------------------------------------------
void MLICIntegrator::GetByEpoch(int p_nEpoch, std::vector<MLIrradianceCacheRecord*> &p_recordList)
{
	for (auto record : m_irradianceCacheRecordList)
	{
		if (record->Epoch == p_nEpoch)
			p_recordList.push_back(record);
	}
}
//----------------------------------------------------------------------------------------------
void MLICIntegrator::GetByEpochRange(int p_nEpochMin, int p_nEpochMax, std::vector<MLIrradianceCacheRecord*> &p_recordList)
{
	for (auto record : m_irradianceCacheRecordList)
	{
		if (record->Epoch >= p_nEpochMin && record->Epoch <= p_nEpochMax)
			p_recordList.push_back(record);
	}
}
//----------------------------------------------------------------------------------------------
bool MLICIntegrator::HasEpochQuota(int p_nQuota)
{
	int quota = 0;

	for (auto record : m_irradianceCacheRecordList)
	{
		if (record->Epoch == m_nEpoch)  {
			quota++; if (quota >= p_nQuota) return true;
		}
	}

	return false;
}
//----------------------------------------------------------------------------------------------
bool MLICIntegrator::Initialise(Scene *p_pScene, ICamera *p_pCamera)
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
	m_irradianceCache.SetDepth(m_nCacheDepth);

	m_nGenerationCount = 0;
	m_nInsertionCount = 0;

#if (defined __INSTANT_CACHING__)
	m_helper.Initialise(p_pScene, m_fReflectEpsilon, m_nRayDepth, m_nShadowRays);
	m_helper.SetHemisphereDivisions(m_nAzimuthStrata, m_nAltitudeStrata);
	m_helper.SetVirtualPointSources(1024, 1, 10);
	m_helper.SetGeometryTerm(0.1f);
#endif

	return true;
}
//----------------------------------------------------------------------------------------------
bool MLICIntegrator::Shutdown(void)
{
	ReleaseRecords();

	return true;
}
//----------------------------------------------------------------------------------------------
bool MLICIntegrator::Prepare(Scene *p_pScene)
{
	m_helper.Prepare(IntegratorHelper<MLIrradianceCacheRecord>::PointLit);

	return true;
}
//----------------------------------------------------------------------------------------------
Spectrum MLICIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, Intersection &p_intersection, RadianceContext *p_pRadianceContext)
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

	#if (defined ___DEBUG_IC___)
	p_pRadianceContext->Direct = 
		p_pRadianceContext->Indirect = 0.f;

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
				return p_pRadianceContext->Indirect = GetIrradiance(p_intersection, p_pScene);

			return 0.f;
		}
	}
	
	return 0.f;
	#else
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
					p_pRadianceContext->Direct = p_intersection.GetLight()->Radiance(p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, wOut);
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
	
		return p_pRadianceContext->Direct + p_pRadianceContext->Indirect;
	#endif
}
//----------------------------------------------------------------------------------------------
Spectrum MLICIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, RadianceContext *p_pRadianceContext)
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
Spectrum MLICIntegrator::GetIrradiance(const Intersection &p_intersection, Scene *p_pScene)
{
	std::vector<std::pair<float, MLIrradianceCacheRecord*>> nearbyRecordList;

	// Find nearyby records
	m_irradianceCache.FindRecords(p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, nearbyRecordList);

	#if (defined ___DEBUG_IC___)
		if (nearbyRecordList.size() > 0)
			return Spectrum(0.f);

		MLIrradianceCacheRecord *record = RequestRecord();
		ComputeRecord(p_intersection, p_pScene, *record);
		m_irradianceCache.Insert(&(m_irradianceCache.RootNode), record, m_nCacheDepth);
		return Spectrum(1e+4f);
	#else
		if (nearbyRecordList.size() > 0)
		{
			Spectrum num = 0.f;
			float den = 0.f;

			//for (auto pair : nearbyRecordList)
			for (auto iter = nearbyRecordList.begin(); iter != nearbyRecordList.end(); iter++)
			{
				auto pair = *iter;

				num += pair.second->Irradiance * pair.first;
				den += pair.first;
			}

			num /= den;
			return num;
		}
		else
		{
			if (m_bIsSampleGenerationDisabled) return 0.f;

			m_nGenerationCount++;

			MLIrradianceCacheRecord *record = RequestRecord();
			ComputeRecord(p_intersection, p_pScene, *record);
			m_irradianceCache.Insert(&(m_irradianceCache.RootNode), record, m_nCacheDepth);
			return record->Irradiance;
		}
	#endif
}
//----------------------------------------------------------------------------------------------
void MLICIntegrator::ComputeRecord(const Intersection &p_intersection, Scene *p_pScene, MLIrradianceCacheRecord &p_record)
{
#if (defined __INSTANT_CACHING__)
	p_record.Position = p_intersection.Surface.PointWS;
	p_record.Normal = p_intersection.Surface.ShadingBasisWS.W;
	p_record.Ri = m_fErrorThreshold * 2.0f;
	p_record.RiClamp = Maths::Max(m_fRMin, Maths::Min(m_fRMax, p_record.Ri));

	// m_helper.Shade(&p_record, IntegratorHelper<MLIrradianceCacheRecord>::PathTraced);
	m_helper.Shade(&p_record, IntegratorHelper<MLIrradianceCacheRecord>::PointLit);
#else
	Intersection isect;
	Vector2 sample2D;
	Vector3 wOutR;
	Ray ray;

	Spectrum E = 0;
	
	float totLength = 0,
		minLength = Maths::Maximum;
	
	// Cache this - it doesn't change!
	int mn = m_nAzimuthStrata * m_nAltitudeStrata;

	for (int altitudeIndex = 0; altitudeIndex < m_nAltitudeStrata; altitudeIndex++)
	{
		for (int azimuthIndex = 0; azimuthIndex < m_nAzimuthStrata; azimuthIndex++)
		{
			sample2D = p_pScene->GetSampler()->Get2DSample();

			Vector3 vH = 
				Montecarlo::CosineSampleHemisphere(sample2D.X, sample2D.Y, altitudeIndex, azimuthIndex, m_nAltitudeStrata, m_nAzimuthStrata); 

			BSDF::SurfaceToWorld(p_intersection.WorldTransform, p_intersection.Surface, vH, wOutR);
			ray.Set(p_intersection.Surface.PointWS + wOutR * m_fReflectEpsilon, wOutR, m_fReflectEpsilon, Maths::Maximum);

			E += PathLi(p_pScene, ray);
			totLength += 1.f / ray.Max;
			minLength = Maths::Min(ray.Max, minLength);
		}
	}

	// MN = total samples

	p_record.Position = p_intersection.Surface.PointWS;
	p_record.Normal = p_intersection.Surface.ShadingBasisWS.W;
	p_record.Irradiance = E / mn;
	p_record.Ri = mn / totLength;
	// p_record.Ri = minLength;
	// p_record.RiClamp = p_record.Ri;
	p_record.RiClamp = Maths::Max(m_fRMin, Maths::Min(m_fRMax, p_record.Ri));
#endif

	// std::cout << "Ri = [" << p_record.Ri << "], [" << p_record.RiClamp << "]" << std::endl;
}
//----------------------------------------------------------------------------------------------
Spectrum MLICIntegrator::PathLi(Scene *p_pScene, Ray &p_ray)
{
	IMaterial *pMaterial = NULL;
	bool specularBounce = false;
	
	Spectrum L(0.f),
		pathThroughput = 1.;
	
	Ray ray(p_ray); p_ray.Max = 1.f;
	
	BxDF::Type bxdfType;
	
	Vector3 wIn, wOut, 
		wInLocal, wOutLocal; 

	float pdf;

	// Trace
	for (int pathLength = 0; ; ++pathLength) 
	{
		// Find next vertex of path
		Intersection isect;
		
		if (!p_pScene->Intersects(ray, isect))
			break;

		// Get material
		if (!isect.HasMaterial()) 
			break;
		
		pMaterial = isect.GetMaterial();

		// Set distance if first bounce
		if (pathLength == 0)
			p_ray.Max = isect.Surface.Distance;

		wOut = -ray.Direction;

		// Possibly add emitted light at path vertex
		if (specularBounce)
		{
			if (isect.IsEmissive())
			{
				L += pathThroughput * 
					isect.GetLight()->Radiance(
						isect.Surface.PointWS, 
						isect.Surface.GeometryBasisWS.W, 
						wOut);
			}
		}
	
		// Sample illumination from lights to find path contribution
		L += pathThroughput * SampleAllLights(p_pScene, isect, 
			isect.Surface.PointWS, isect.Surface.ShadingBasisWS.W, wOut, 
			p_pScene->GetSampler(), isect.GetLight(), m_nShadowRays);

		if (pathLength + 1 == m_nRayDepth) break;

		// Sample bsdf for next direction
		Vector2 sample = p_pScene->GetSampler()->Get2DSample();

		// Convert to surface coordinate system where (0,0,1) represents surface normal
		BSDF::WorldToSurface(isect.WorldTransform, isect.Surface, wOut, wOutLocal);

		// Sample new direction in wIn (remember we're tracing backwards)
		Spectrum f = pMaterial->SampleF(isect.Surface, wOutLocal, wInLocal, sample.U, sample.V, &pdf, BxDF::All_Combined, &bxdfType);

		// If the reflectivity or pdf are zero, terminate path
		if (f.IsBlack() || pdf == 0.0f) break;

		// Record if bounce is a specular bounce
		specularBounce = ((int)(bxdfType & BxDF::Specular)) != 0;

		// Convert back to world coordinates
		BSDF::SurfaceToWorld(isect.WorldTransform, isect.Surface, wInLocal, wIn);
		
		// Adjust path for new bounce
		// ray.Set(isect.Surface.PointWS + wIn * m_fReflectEpsilon, wIn, m_fReflectEpsilon, Maths::Maximum);
		ray.Set(isect.Surface.PointWS, wIn, m_fReflectEpsilon, Maths::Maximum);
		
		// Update path contribution at current stage
		pathThroughput *= f * Vector3::AbsDot(wIn, isect.Surface.ShadingBasisWS.W) / pdf;

		// Use Russian roulette to possibly terminate path
		if (pathLength > 2)
		{
			float continueProbability = Maths::Min(0.5f, 0.33f * (pathThroughput[0] + pathThroughput[1] + pathThroughput[2]));

			if (p_pScene->GetSampler()->Get1DSample() > continueProbability)
				break;

			pathThroughput /= continueProbability;
		}
	}

	return L;
}
//----------------------------------------------------------------------------------------------
MLIrradianceCacheRecord* MLICIntegrator::RequestRecord(void)
{
	m_nInsertionCount++;

	MLIrradianceCacheRecord *pRecord = new MLIrradianceCacheRecord();
	pRecord->Epoch = m_nEpoch;
	m_irradianceCacheRecordList.push_back(pRecord);
	return pRecord;
}
//----------------------------------------------------------------------------------------------
MLIrradianceCacheRecord* MLICIntegrator::RequestRecord(MLIrradianceCacheRecord* p_pRecord, int p_nEpoch)
{
	m_nInsertionCount++;
	
	MLIrradianceCacheRecord *pRecord = new MLIrradianceCacheRecord(*p_pRecord);
	pRecord->Epoch = (p_nEpoch == -1) ? m_nEpoch : p_nEpoch;
	m_irradianceCacheRecordList.push_back(pRecord);
	return pRecord;
}
//----------------------------------------------------------------------------------------------
void MLICIntegrator::ReleaseRecords(void)
{
	for (auto iter = m_irradianceCacheRecordList.begin();
		 iter != m_irradianceCacheRecordList.end(); iter++)
	{
		delete (*iter);
	}

	/*
	for (auto record : m_irradianceCacheRecordList)
		delete record;
	*/
}
//----------------------------------------------------------------------------------------------
std::string MLICIntegrator::ToString(void) const
{
	std::stringstream result; result 
		<< " :: Generated [" << m_nGenerationCount << "]" << std::endl 
		<< " :: Inserted [" << m_nInsertionCount << "]" << std::endl;
	
	return m_irradianceCache.ToString() + result.str();

	//return m_irradianceCache.ToString();
}