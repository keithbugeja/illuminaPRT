//----------------------------------------------------------------------------------------------
//	Filename:	PhotonIntegrator.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iostream>
#include "Integrator/PhotonIntegrator.h"

#include "Geometry/Ray.h"
#include "Geometry/BoundingBox.h"
#include "Geometry/Intersection.h"
#include "Sampler/JitterSampler.h"
#include "Material/Material.h"
#include "Spectrum/Spectrum.h"
#include "Scene/Visibility.h"
#include "Scene/Primitive.h"
#include "Scene/Scene.h"

#include "Maths/Montecarlo.h"

// TODOS:
//
// 1. Photon shooting should cater for additional bounces
// 2. Photon shooting should provide proper reflection/transmission for other materials
// 3. Re-write lookup - seems crap
//

namespace Illumina
{
	namespace Core
	{
		struct DistancePhoton
		{
			Photon *pPhoton;
			float Distance;
		};

		template <class T>
		class KDTreePhotonLookupMethod
			: public IAccelerationStructureLookupMethod<T>
		{
		public:
			struct tagSort {
				bool operator() (const DistancePhoton& d1, const DistancePhoton& d2) 
				{ 
					return d1.Distance < d2.Distance;;
				}
			} SortPredicate;

		public:
			int PhotonCount;
			std::vector<DistancePhoton> PhotonList;

			void Reset()
			{
				PhotonCount = 0;
				PhotonList.clear();
			}

			bool operator()(const Vector3 &p_lookupPoint, float p_fMaxDistance, T &p_element) 
			{
				DistancePhoton photon;

				photon.pPhoton = &p_element;
				photon.Distance = (photon.pPhoton->Position - p_lookupPoint).Length();
			
				if (photon.Distance < p_fMaxDistance)
				{
					PhotonCount++;

					PhotonList.push_back(photon);
					return true;
				}

				return false;
			}
		};

	}
}

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
PhotonIntegrator::PhotonIntegrator(const std::string &p_strName, int p_nMaxPhotonCount, int p_nMaxRayDepth, int p_nShadowSampleCount, float p_fReflectEpsilon)
	: IIntegrator(p_strName) 
	, m_nMaxPhotonCount(p_nMaxPhotonCount)
	, m_nShadowSampleCount(p_nShadowSampleCount)
	, m_nMaxRayDepth(p_nMaxRayDepth)
	, m_fReflectEpsilon(p_fReflectEpsilon)
{ }
//----------------------------------------------------------------------------------------------
PhotonIntegrator::PhotonIntegrator(int p_nMaxPhotonCount, int p_nMaxRayDepth, int p_nShadowSampleCount, float p_fReflectEpsilon)
	: m_nMaxPhotonCount(p_nMaxPhotonCount)
	, m_nShadowSampleCount(p_nShadowSampleCount)
	, m_nMaxRayDepth(p_nMaxRayDepth)
	, m_fReflectEpsilon(p_fReflectEpsilon)
{ }
//----------------------------------------------------------------------------------------------
Spectrum PhotonIntegrator::SampleF(Scene *p_pScene, Intersection &p_intersection, const Vector3 &p_wOut, Vector3 &p_wIn, float &p_pdf, BxDF::Type &p_bxdfType)
{
	if (!p_intersection.HasMaterial())
		return 0.f;

	IMaterial *pMaterial = p_intersection.GetMaterial();

	Vector3 wOutLocal, 
		wInLocal;

	//----------------------------------------------------------------------------------------------
	// Sample bsdf for next direction
	//----------------------------------------------------------------------------------------------
	// Generate random samples
	Vector2 sample = p_pScene->GetSampler()->Get2DSample();

	// Convert to surface coordinate system where (0,0,1) represents surface normal
	// Note: 
	// -- All Material/BSDF/BxDF operations are carried out in surface coordinates
	// -- All inputs must be in surface coordinates
	// -- All outputs are in surface coordinates
	BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, p_wOut, wOutLocal);

	// Sample new direction in wIn (remember we're tracing backwards)
	// -- wIn returns the sampled direction
	// -- pdf returns the reflectivity function's pdf at the sampled point
	// -- bxdfType returns the type of BxDF sampled
	Spectrum f = pMaterial->SampleF(p_intersection.Surface, wOutLocal, wInLocal, sample.U, sample.V, &p_pdf, BxDF::All_Combined, &p_bxdfType);

	// If the reflectivity or pdf are zero, terminate path
	if (f.IsBlack() || p_pdf == 0.0f) return 0.f;

	// Convert back to world coordinates
	BSDF::SurfaceToWorld(p_intersection.WorldTransform, p_intersection.Surface, wInLocal, p_wIn);

	return f;
}

//----------------------------------------------------------------------------------------------
bool PhotonIntegrator::Initialise(Scene *p_pScene, ICamera *p_pCamera)
{
	std::cout << "Constructing photon map" << std::endl;

	float sample, 
		pdf;

	int lightIdx, 
		objIdx;

	BxDF::Type bxdfType;
	
	bool specularBounce, 
		lastBounceSpecular;
	
	Vector2 uvSample;

	Vector3 objectPoint, 
		lightPoint, 
		normal, 
		wIn, wOut;

	VisibilityQuery visibilityQuery(p_pScene);
	Intersection intersection;
	IPrimitive *pPrimitive;

	Vector3 photonDirection;
	Spectrum photonPower;

	int photonCount = 0,
		intersections;

	// for each photon, we have to determine initial ray before using 
	// a montecarlo like russian roulette to propagate photons across
	// scene.
	m_nMaxPhotonCount = 10000000;

	int nMaxDirectPhotonCount = 1000,
		nMaxIndirectPhotonCount = 40000,
		nMaxCausticsPhotonCount = 80000;

	int nIndirectCount = nMaxIndirectPhotonCount,
		nDirectCount = nMaxDirectPhotonCount,
		nCausticsCount = nMaxCausticsPhotonCount;

	m_directMap.Clear();
	m_indirectMap.Clear();
	m_causticsMap.Clear();

	while (true)
	{
		// Start by selecting a light source
		sample = p_pScene->GetSampler()->Get1DSample();
		lightIdx = Maths::Floor(sample * p_pScene->LightList.Size());

		// Select an object in the scene
		//sample = p_pScene->GetSampler()->Get1DSample();
		//objIdx = Maths::Floor(sample * p_pScene->GetSpace()->PrimitiveList.Size());		

		// Sample point on object surface
		//uvSample = p_pScene->GetSampler()->Get2DSample();
		//pPrimitive = p_pScene->GetSpace()->PrimitiveList[objIdx];
		//objectPoint = pPrimitive->SamplePoint(uvSample.U, uvSample.V, normal);

		// Sample point on light source
		uvSample = p_pScene->GetSampler()->Get2DSample();
		lightPoint = p_pScene->LightList[lightIdx]->SamplePoint(objectPoint, uvSample.U, uvSample.V, normal, pdf);

		OrthonormalBasis b; b.InitFromW(normal);
		uvSample = p_pScene->GetSampler()->Get2DSample();
		photonDirection = Montecarlo::UniformSampleCone(uvSample.U, uvSample.V, 0.05f, b);

		// Determine photon direction and power
		//photonDirection = Vector3::Normalize(objectPoint - lightPoint);
		photonPower = p_pScene->LightList[lightIdx]->Radiance(lightPoint, normal, photonDirection);

		// Correct normal
		if (Vector3::Dot(photonDirection, normal) < 0.f)
			normal = -normal;

		// Initialise ray
		//Ray photonRay(lightPoint + normal * 1e-4f, photonDirection);
		Ray photonRay(lightPoint + photonDirection * 1e-4f, photonDirection);

		// -> re-arrange / clean
		specularBounce = lastBounceSpecular = false;
		intersections = 0;
		m_nMaxRayDepth = 6;
		m_fReflectEpsilon = 1e-04f;
		// -> re-arrange / clean

		while (p_pScene->Intersects(photonRay, intersection))
		{
			wOut = -Vector3::Normalize(photonRay.Direction);
			//
			Spectrum f = SampleF(p_pScene, intersection, wOut, wIn, pdf, bxdfType);

			if (f.IsBlack())
				break;
			//
			
			lastBounceSpecular &= specularBounce;
			
			specularBounce = ((int)(bxdfType & BxDF::Specular)) != 0;
			//specularBounce = (intersection.GetMaterial()->HasBxDFType(BxDF::Diffuse) == 0);
			//specularBounce = intersection.GetMaterial()->HasBxDFType(BxDF::Type(BxDF::Specular), false);

			if (intersections == 0)
				lastBounceSpecular = specularBounce;

			if (!specularBounce)
			{
				// Define photon we are storing
				Photon photon;

				photon.Direction = photonDirection;
				photon.Position = intersection.Surface.PointWS;
				photon.Power = photonPower;

				if (lastBounceSpecular && intersections > 0)
				{
					if (nCausticsCount > 0)
					{
						m_causticsMap.Insert(photon);
						nCausticsCount--;
					}
				}
				else
				{
					// direct or indirect
					if (intersections != 0)
					{
						if (nIndirectCount > 0)
						{
							m_indirectMap.Insert(photon);
							nIndirectCount--;
						}
					}
					else
					{
						if (nDirectCount > 0)
						{
							m_directMap.Insert(photon);
							nDirectCount--;
						}
					}
				}
			}

			intersections++;

			if (intersections > m_nMaxRayDepth)
				break;

			float continueProbability = 0.33f * (f[0] + f[1] + f[2]),
				xi = p_pScene->GetSampler()->Get1DSample();

			if (xi > continueProbability)
				break;

			photonRay.Min = 0.f;
			photonRay.Max = Maths::Maximum;
			photonRay.Origin = intersection.Surface.PointWS + wIn * m_fReflectEpsilon;
			photonRay.Direction = wIn;

			photonPower = photonPower * (xi / continueProbability) * f;
		}

		//std::cout << nDirectCount << ", " << nIndirectCount << ", " << nCausticsCount << ", " << lastBounceSpecular << std::endl;

		if (nDirectCount == 0 && nIndirectCount == 0 && nCausticsCount == 0)
			break;
	}

	std::cout << "Building photon map..." << std::endl;
	m_directMap.Build();
	m_indirectMap.Build();
	m_causticsMap.Build();

	return true;
}
//----------------------------------------------------------------------------------------------
bool PhotonIntegrator::Shutdown(void)
{
	return true;
}
//----------------------------------------------------------------------------------------------
Spectrum PhotonIntegrator::Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection)
{
	VisibilityQuery visibilityQuery(p_pScene);

	Spectrum pathThroughput(1.0f), L(0.0f);

	IMaterial *pMaterial = NULL;

	bool specularBounce = false;

	BxDF::Type bxdfType;

	Vector3 wIn, wOut, wInLocal, wOutLocal; 

	Vector2 sample;

	Ray ray(p_ray); 

	float pdf;

	KDTreePhotonLookupMethod<Photon> lookup;
	
	for (int rayDepth = 0; rayDepth < m_nMaxRayDepth; rayDepth++)
	{
		//----------------------------------------------------------------------------------------------
		// No intersection
		//----------------------------------------------------------------------------------------------
		if(!p_pScene->Intersects(ray, p_intersection))
		{
			if (rayDepth == 0 || specularBounce) 
			{
				for (size_t lightIndex = 0; lightIndex < p_pScene->LightList.Size(); ++lightIndex)
					L += pathThroughput * p_pScene->LightList[lightIndex]->Radiance(-ray);
			}

			break;
		}
		
		//----------------------------------------------------------------------------------------------
		// Primitive has no material assigned - terminate
		//----------------------------------------------------------------------------------------------
		if (!p_intersection.HasMaterial()) 
			break;
		
		// Get material for intersection primitive
		pMaterial = p_intersection.GetMaterial();

		//----------------------------------------------------------------------------------------------
		// Sample lights for specular / first bounce
		//----------------------------------------------------------------------------------------------
		wOut = -Vector3::Normalize(ray.Direction);

		// Add emitted light : only on first bounce or specular to avoid double counting
		if (p_intersection.IsEmissive())
		{
			if (rayDepth == 0 || specularBounce)
			{
				// Add contribution from luminaire
				// -- Captures highlight on specular materials
				// -- Transmits light through dielectrics
				// -- Renders light primitive for first bounce intersections
				L += pathThroughput * p_intersection.GetLight()->Radiance(p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, wOut);

				if (rayDepth == 0) break;
			}
		}

		//----------------------------------------------------------------------------------------------
		// If material has a diffuse component, compute using photon map
		//----------------------------------------------------------------------------------------------
		if (pMaterial->HasBxDFType(BxDF::Diffuse, false))
		{
			float r = 1e-03f;
			r = 0.1f;
			
			lookup.Reset();
			/**/
			if (m_indirectMap.Lookup(p_intersection.Surface.PointWS, r, lookup))
			{
				Spectrum indirectL = 0;
				float k = 1.5f;

				std::sort(lookup.PhotonList.begin(), lookup.PhotonList.end(), lookup.SortPredicate);
				std::vector<DistancePhoton>::iterator photonIterator;
				int photonsDone = 0;

				for (photonIterator = lookup.PhotonList.begin(); 
					 photonIterator != lookup.PhotonList.end();
					 ++photonIterator)
				{
					++photonsDone;

					float wpc = 1 - Vector3::Distance(p_intersection.Surface.PointWS, (*photonIterator).pPhoton->Position) / (k * r);

					float pdf;

					Vector3 photonDirection = -((*photonIterator).pPhoton->Direction);
					Vector3 bsdfIn, bsdfOut;

					BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, wOut, bsdfOut);
					BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, photonDirection, bsdfIn);
					
					Spectrum Ls = (*photonIterator).pPhoton->Power * pMaterial->F(p_intersection.Surface, bsdfOut, bsdfIn) * wpc;
					
					indirectL += Ls;
				}

				indirectL /= (1-2/(3*k)) * (Maths::PiTwo * r * photonsDone);
				
				//L += indirectL;
			} 
			/**/

			lookup.Reset();
			r = 0.1f;

			if (m_causticsMap.Lookup(p_intersection.Surface.PointWS, r, lookup))
			{
				Spectrum causticsL = 0;
				float k = 1.5f;

				std::sort(lookup.PhotonList.begin(), lookup.PhotonList.end(), lookup.SortPredicate);
				std::vector<DistancePhoton>::iterator photonIterator;
				int photonsDone = 0;

				for (photonIterator = lookup.PhotonList.begin(); 
					 photonIterator != lookup.PhotonList.end();
					 ++photonIterator)
				{
					++photonsDone;

					float wpc = 1 - Vector3::Distance(p_intersection.Surface.PointWS, (*photonIterator).pPhoton->Position) / (k * r);

					float pdf;

					Vector3 photonDirection = -((*photonIterator).pPhoton->Direction);
					Vector3 bsdfIn, bsdfOut;

					BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, wOut, bsdfOut);
					BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, photonDirection, bsdfIn);
					
					Spectrum Ls = (*photonIterator).pPhoton->Power * pMaterial->F(p_intersection.Surface, bsdfOut, bsdfIn) * wpc;
					
					causticsL += Ls;
				}

				causticsL /= (1-2/(3*k)) * (Maths::PiTwo * r * photonsDone);
				
				L += causticsL;
			}

			//L += pathThroughput * SampleAllLights(p_pScene, p_intersection, p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut, p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);
		}

		//----------------------------------------------------------------------------------------------
		// Sample lights for direct lighting
		// -- If the currently intersected primitive is a luminaire, do not sample it 
		//----------------------------------------------------------------------------------------------
		//if (!specularBounce)
			//L += pathThroughput * SampleAllLights(p_pScene, p_intersection, p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut, p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);
			
		//----------------------------------------------------------------------------------------------
		// Sample bsdf for next direction
		//----------------------------------------------------------------------------------------------
		// Generate random samples
		Spectrum f = SampleF(p_pScene, p_intersection, wOut, wIn, pdf, bxdfType);

		if (f.IsBlack() || pdf == 0.0f) break;

		specularBounce = ((int)(bxdfType & BxDF::Specular)) != 0;

		if (!specularBounce)
			break;
		/*
		sample = p_pScene->GetSampler()->Get2DSample();

		// Convert to surface coordinate system where (0,0,1) represents surface normal
		// Note: 
		// -- All Material/BSDF/BxDF operations are carried out in surface coordinates
		// -- All inputs must be in surface coordinates
		// -- All outputs are in surface coordinates

		BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, wOut, wOutLocal);

		// Sample new direction in wIn (remember we're tracing backwards)
		// -- wIn returns the sampled direction
		// -- pdf returns the reflectivity function's pdf at the sampled point
		// -- bxdfType returns the type of BxDF sampled
		Spectrum f = pMaterial->SampleF(p_intersection.Surface, wOutLocal, wInLocal, sample.U, sample.V, &pdf, BxDF::All_Combined, &bxdfType);

		// If the reflectivity or pdf are zero, terminate path
		if (f.IsBlack() || pdf == 0.0f) break;

		// Record if bounce is a specular bounce
		specularBounce = ((int)(bxdfType & BxDF::Specular)) != 0;

		// Convert back to world coordinates
		BSDF::SurfaceToWorld(p_intersection.WorldTransform, p_intersection.Surface, wInLocal, wIn);
		*/

		//if (!specularBounce) break;

		//----------------------------------------------------------------------------------------------
		// Adjust path for new bounce
		// -- ray is moved by a small epsilon in sampled direction
		// -- ray origin is set to point of intersection
		//----------------------------------------------------------------------------------------------
		ray.Min = 0.f;
		ray.Max = Maths::Maximum;
		ray.Origin = p_intersection.Surface.PointWS + wIn * m_fReflectEpsilon;
		ray.Direction = wIn;
		
		// Update path contribution at current stage
		pathThroughput *= f * Vector3::AbsDot(wIn, p_intersection.Surface.GeometryBasisWS.W) / pdf;

		//----------------------------------------------------------------------------------------------
		// Use Russian roulette to possibly terminate path
		//----------------------------------------------------------------------------------------------
		if (rayDepth > 3)
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