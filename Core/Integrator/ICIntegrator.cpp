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
	float dmin = 0;
	for( int i = 0; i < 3; i++ )
	{
		if (p_centre[i] < p_aabb.GetMinExtent(i) ) dmin += Maths::Sqr(p_centre[i] - p_aabb.GetMinExtent(i));
		else if (p_centre[i] > p_aabb.GetMaxExtent(i) ) dmin += Maths::Sqr(p_centre[i] - p_aabb.GetMaxExtent(i));
	}
	return dmin <= p_fRadius*p_fRadius;

	/*
	return (p_centre.X + p_fRadius > p_aabb.GetMinExtent(0) &&
			p_centre.X - p_fRadius < p_aabb.GetMaxExtent(0) &&
			p_centre.Y + p_fRadius > p_aabb.GetMinExtent(1) &&
			p_centre.Y - p_fRadius < p_aabb.GetMaxExtent(1) &&
			p_centre.Z + p_fRadius > p_aabb.GetMinExtent(2) &&
			p_centre.Z - p_fRadius < p_aabb.GetMaxExtent(2));
	*/
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
		// if (p_pNode->Children == nullptr)
		if (p_pNode->Children == NULL)
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
				IrradianceCacheRecord *r = *iter;

				wi = W(p_point, p_normal, *r);
				minval = Maths::Min(wi, minval);
				maxval = Maths::Max(wi, maxval);

				// Race!
				//if ((wi = W(p_point, p_normal, *r)) > 0.f)
				if (wi > 0.f)
				{
				// if ((wi = W(p_point, p_normal, *r)) > 1.f / m_fErrorThreshold)
					p_nearbyRecordList.push_back(std::pair<float, IrradianceCacheRecord*>(wi, r));
				// Race!
				}
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
float IrradianceCache::W_Ward(const Vector3 &p_point, const Vector3 &p_normal, IrradianceCacheRecord &p_record)
{
	float dist = Vector3::Distance(p_point, p_record.Point) / p_record.RiClamp;
	float norm = Maths::Sqrt(1 - Vector3::AbsDot(p_normal, p_record.Normal));
	// return (1.f / (dist + norm)) - (1.f / m_fErrorThreshold);

	return (1.f / dist) - (1.f / m_fErrorThreshold);
}
//----------------------------------------------------------------------------------------------
float IrradianceCache::W_Tabelion(const Vector3 &p_point, const Vector3 &p_normal, IrradianceCacheRecord &p_record)
{
	float cosMaxAngleDifference = 0.2f;

	float epi = Vector3::Distance(p_point, p_record.Point) / p_record.RiClamp;
	float eni = Maths::Sqrt((1 - Vector3::Dot(p_normal, p_record.Normal)) / 
							(1.f - cosMaxAngleDifference));

	float err = Maths::Max(epi, eni);

	return 1.f - (err * m_fErrorThreshold);
}
//----------------------------------------------------------------------------------------------
float IrradianceCache::W(const Vector3 &p_point, const Vector3 &p_normal, IrradianceCacheRecord &p_record)
{
	/* 
	Vector3 P = p_record.Point - p_point;
	float d = Vector3::Dot(P, (p_normal + p_record.Normal) * 0.5f);

	if (d < 0.05f) return -1;
	*/

	/* 
	Vector3 p = p_point - p_record.Point;
	float d = Vector3::Dot(p, (p_normal + p_record.Normal) * 0.5f);

	if (d < 0.1f) return -1;
	*/

	return W_Ward(
	// return W_Tabelion(
		p_point, p_normal, p_record);
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

//#define ___DEBUG_IC___

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
	ReleaseRecords();

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

	#if (defined ___DEBUG_IC___)
		if (nearbyRecordList.size() > 0)
			return Spectrum(0.f);

		IrradianceCacheRecord *record = RequestRecord();
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
			IrradianceCacheRecord *record = RequestRecord();
			ComputeRecord(p_intersection, p_pScene, *record);
			m_irradianceCache.Insert(&(m_irradianceCache.RootNode), record, m_nCacheDepth);
			return record->Irradiance;
		}
	#endif
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

	p_record.Point = p_intersection.Surface.PointWS;
	// p_record.Point = p_intersection.Surface.ShadingNormal;
	p_record.Normal = p_intersection.Surface.ShadingBasisWS.W;
	p_record.Irradiance = E / mn;
	p_record.Ri = mn / totLength;
	// p_record.Ri = minLength;
	// p_record.RiClamp = p_record.Ri;
	p_record.RiClamp = Maths::Max(m_fRMin, Maths::Min(m_fRMax, p_record.Ri));

	// std::cout << "Ri = [" << p_record.Ri << "], [" << p_record.RiClamp << "]" << std::endl;
}
//----------------------------------------------------------------------------------------------
Spectrum ICIntegrator::PathLi(Scene *p_pScene, Ray &p_ray)
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
IrradianceCacheRecord* ICIntegrator::RequestRecord(void)
{
	IrradianceCacheRecord *pRecord = new IrradianceCacheRecord();
	m_irradianceCacheRecordList.push_back(pRecord);
	return pRecord;
}
//----------------------------------------------------------------------------------------------
void ICIntegrator::ReleaseRecords(void)
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
std::string ICIntegrator::ToString(void) const
{
	return m_irradianceCache.ToString();
}