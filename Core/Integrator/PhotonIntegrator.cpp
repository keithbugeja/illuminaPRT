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
		nMaxIndirectPhotonCount = 100000,
		nMaxCausticsPhotonCount = 10000;

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
		lightIdx = (int)Maths::Floor(sample * p_pScene->LightList.Size());

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
			
			// Did we hit an S surface?
			bool hasSpecular = intersection.GetMaterial()->HasBxDFType(BxDF::Specular);

			// Record photon -> Can be nested
			Photon photon;

			photon.Direction = photonDirection;
			photon.Position = intersection.Surface.PointWS;
			photon.Power = photonPower;

			// If we hit a D surface, we record photon?
			if (!hasSpecular)
			{
				// Direct contribution
				if (intersections == 0)
				{
					if (nDirectCount > 0)
					{
						m_directMap.Insert(photon);
						nDirectCount--;
					}
				}
				else
				{
					// Caustics contribution
					if (lastBounceSpecular)
					{
						if (nCausticsCount > 0)
						{
							m_causticsMap.Insert(photon);
							nCausticsCount--;
						}
					}
					else // Indirect contribution
					{
						if (nIndirectCount > 0)
						{
							m_indirectMap.Insert(photon);
							nIndirectCount--;
						}
					}
				}
			}

			// If this is the (L(S|D)), record the last bounce directly
			if (intersections++ == 0)
				lastBounceSpecular = hasSpecular;
			else
				lastBounceSpecular &= hasSpecular;			

			if (intersections > m_nMaxRayDepth)
				break;

			float continueProbability = 0.33f * (f[0] + f[1] + f[2]),
				xi = p_pScene->GetSampler()->Get1DSample();

			if (xi > continueProbability)
				break;

			photonRay.Set(intersection.Surface.PointWS + wIn * m_fReflectEpsilon, wIn, 0.f, Maths::Maximum);

			//photonRay.Min = 0.f;
			//photonRay.Max = Maths::Maximum;
			//photonRay.Origin = intersection.Surface.PointWS + wIn * m_fReflectEpsilon;
			//photonRay.Direction = wIn;
			//Vector3::Inverse(photonRay.Direction, photonRay.DirectionInverseCache);

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
Spectrum PhotonIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection)
{
	return Radiance(p_pContext, p_pScene, p_ray, p_intersection, 0);
}
//----------------------------------------------------------------------------------------------
Spectrum PhotonIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, int p_nRayDepth)
{
	VisibilityQuery visibilityQuery(p_pScene);
	IMaterial *pMaterial = NULL;
	Ray ray(p_ray);

	BxDF::Type bxdfType;
	Vector3 wIn, wOut, wInLocal, wOutLocal; 
	Vector2 sample;

	float pdf;

	KDTreePhotonLookupMethod<Photon> lookup;

	Spectrum L(0), Ld(0), Ls(0), Lt(0);

	//----------------------------------------------------------------------------------------------
	// No intersection
	//----------------------------------------------------------------------------------------------
	if(!p_pScene->Intersects(ray, p_intersection))
		return 0;
	/*
	{
		if (p_nRayDepth == 0 || specularBounce) 
		{
			for (size_t lightIndex = 0; lightIndex < p_pScene->LightList.Size(); ++lightIndex)
				L += pathThroughput * p_pScene->LightList[lightIndex]->Radiance(-ray);
		}

		return L;
	} 
	*/

	//----------------------------------------------------------------------------------------------
	// Primitive has no material assigned - terminate
	//----------------------------------------------------------------------------------------------
	if (!p_intersection.HasMaterial()) 
		return 0;
		
	// Get material for intersection primitive
	pMaterial = p_intersection.GetMaterial();

	bool specularBounce = pMaterial->HasBxDFType(BxDF::Specular);

	//----------------------------------------------------------------------------------------------
	// Sample lights for specular / first bounce
	//----------------------------------------------------------------------------------------------
	wOut = -Vector3::Normalize(ray.Direction);

	// Add emitted light : only on first bounce or specular to avoid double counting
	if (p_intersection.IsEmissive())
	{
		if (p_nRayDepth == 0 || specularBounce)
		{
			// Add contribution from luminaire
			// -- Captures highlight on specular materials
			// -- Transmits light through dielectrics
			// -- Renders light primitive for first bounce intersections
				
			L += p_intersection.GetLight()->Radiance(p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, wOut);

			if (p_nRayDepth == 0) return L;
		}
	}

	//----------------------------------------------------------------------------------------------
	// If material has a diffuse component, compute using photon map
	//----------------------------------------------------------------------------------------------
	if (pMaterial->HasBxDFType(BxDF::Diffuse, false))
	{
		float r;
		//r = 0.4f;
		r = 1.0f;

		lookup.Reset();

		if (m_indirectMap.Lookup(p_intersection.Surface.PointWS, r, lookup))
		{
			if (lookup.PhotonCount > 8)
			{
				Spectrum indirectL = 0;
				float k = 1.5f;

				std::sort(lookup.PhotonList.begin(), lookup.PhotonList.end(), lookup.SortPredicate);
				std::vector<DistancePhoton>::iterator photonIterator;
				int photonsDone = 0;

				// Update r to match most distant photon
				r = lookup.PhotonList.back().Distance;

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
				
				L += indirectL;
			}
		} 

		lookup.Reset();
		r = 0.4f;

		if (m_causticsMap.Lookup(p_intersection.Surface.PointWS, r, lookup))
		{
			if (lookup.PhotonCount > 8)
			{
				Spectrum causticsL = 0;
				float k = 1.5f;

				std::sort(lookup.PhotonList.begin(), lookup.PhotonList.end(), lookup.SortPredicate);
				std::vector<DistancePhoton>::iterator photonIterator;
				int photonsDone = 0;

				// Update r to match most distant photon
				r = lookup.PhotonList.back().Distance;

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
		}

		L += SampleAllLights(p_pScene, p_intersection, p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut, p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);

		Ld = L;
	}

	if (p_nRayDepth >= m_nMaxRayDepth)
		return Ld;

	// Reflection
	if (pMaterial->HasBxDFType(BxDF::Type(BxDF::Specular | BxDF::Reflection)))
	{
		sample = p_pScene->GetSampler()->Get2DSample();
			
		Vector3 i,o;

		BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, wOut, o, false);
		Spectrum f = p_intersection.GetMaterial()->SampleF(p_intersection.Surface, o, i, sample.U, sample.V, &pdf, BxDF::Type(BxDF::Reflection), &bxdfType);
		BSDF::SurfaceToWorld(p_intersection.WorldTransform, p_intersection.Surface, i, wIn, false);

		ray.Set(p_intersection.Surface.PointWS + wIn * 1E-4f, wIn, 1E-4f, Maths::Maximum);

		//ray.Origin = p_intersection.Surface.PointWS + p_intersection.Surface.ShadingNormal * 1E-4f;
		//ray.Origin = p_intersection.Surface.PointWS + wIn * 1E-4f;
		//ray.Direction = wIn;
		//ray.Min = 1E-4f;
		//ray.Max = Maths::Maximum;
		//Vector3::Inverse(ray.Direction, ray.DirectionInverseCache);

		if (!f.IsBlack() && pdf != 0.f)
		{
			Intersection intersection;
			Ls = f * Radiance(p_pContext, p_pScene, ray, intersection, p_nRayDepth + 1);
		}
	} 

	// Refraction
	if (pMaterial->HasBxDFType(BxDF::Type(BxDF::Specular | BxDF::Transmission)))
	{
		sample = p_pScene->GetSampler()->Get2DSample();
			
		Vector3 i,o;

		BSDF::WorldToSurface(p_intersection.WorldTransform, p_intersection.Surface, wOut, o);
		Spectrum f = p_intersection.GetMaterial()->SampleF(p_intersection.Surface, o, i, sample.U, sample.V, &pdf, BxDF::Type(BxDF::Transmission), &bxdfType);
		BSDF::SurfaceToWorld(p_intersection.WorldTransform, p_intersection.Surface, i, wIn);

		ray.Set(p_intersection.Surface.PointWS + wIn * 1E-4f, wIn, 1E-4f, Maths::Maximum);

		//ray.Direction = wIn;
		//ray.Origin = p_intersection.Surface.PointWS  + wIn * 1E-4f;
		//ray.Min = 1e-4f;
		//ray.Max = Maths::Maximum;
		//Vector3::Inverse(ray.Direction, ray.DirectionInverseCache);

		if (!f.IsBlack() && pdf != 0.f)
		{
			Intersection intersection;
			Lt = f * Radiance(p_pContext, p_pScene, ray, intersection, p_nRayDepth + 1);
		}
	}
		
	return Ld + Ls + Lt;
	
	/* 
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
			r = 0.4f;
			
			lookup.Reset();
			/ ** /
			if (m_indirectMap.Lookup(p_intersection.Surface.PointWS, r, lookup))
			{
				Spectrum indirectL = 0;
				float k = 1.5f;

				std::sort(lookup.PhotonList.begin(), lookup.PhotonList.end(), lookup.SortPredicate);
				std::vector<DistancePhoton>::iterator photonIterator;
				int photonsDone = 0;

				// Update r to match most distant photon
				r = lookup.PhotonList.back().Distance;

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
				
				L += indirectL;
			} 
			/ ** /

			/ ** /
			lookup.Reset();
			r = 0.4f;

			if (m_causticsMap.Lookup(p_intersection.Surface.PointWS, r, lookup))
			{
				Spectrum causticsL = 0;
				float k = 1.5f;

				std::sort(lookup.PhotonList.begin(), lookup.PhotonList.end(), lookup.SortPredicate);
				std::vector<DistancePhoton>::iterator photonIterator;
				int photonsDone = 0;

				// Update r to match most distant photon
				r = lookup.PhotonList.back().Distance;

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
			/ ** /

			L += pathThroughput * SampleAllLights(p_pScene, p_intersection, p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut, p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);
		}

		//return L;

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

		/ *
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
		* /

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

	*/
}
//----------------------------------------------------------------------------------------------