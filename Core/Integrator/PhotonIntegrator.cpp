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

namespace Illumina
{
	namespace Core
	{
		template <class T>
		class KDTreePhotonLookupMethod
			: public IAccelerationStructureLookupMethod<T>
		{
		public:
			int points;
			Spectrum radiance;

			void Reset()
			{
				radiance = 0;
				points = 0;
			}

			bool operator()(const Vector3 &p_lookupPoint, float p_fMaxDistance, T &p_element) 
			{
				bool hit = false;
				const Photon &photon = (const Photon&)p_element;
				
				if ((photon.Position - p_lookupPoint).Length() < p_fMaxDistance)
				{
					hit = true;
					radiance += photon.Power;
					points++;
				}

				return hit; 
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
bool PhotonIntegrator::Initialise(Scene *p_pScene, ICamera *p_pCamera)
{
	std::cout << "Constructing photon map" << std::endl;

	VisibilityQuery visibilityQuery(p_pScene);
	
	Intersection intersection;

	float sample, pdf;
	int lightIdx, objIdx;
	Vector2 uvSample;
	Vector3 normal,
		objectPoint, 
		lightPoint, 
		wIn;

	// for each photon, we have to determine initial ray before using 
	// a montecarlo like russian roulette to propagate photons across
	// scene.


	// Shoot n photos
	for (int photon = 0; photon < m_nMaxPhotonCount; ++photon)
	{
		// Start by selecting a light source
		sample = p_pScene->GetSampler()->Get1DSample();
		lightIdx = Maths::Floor(sample * p_pScene->LightList.Size());

		// Select an object in the scene
		sample = p_pScene->GetSampler()->Get1DSample();
		objIdx = Maths::Floor(sample * p_pScene->GetSpace()->PrimitiveList.Size());

		// Sample object
		uvSample = p_pScene->GetSampler()->Get2DSample();
		IPrimitive *pPrimitive = p_pScene->GetSpace()->PrimitiveList[objIdx];
		objectPoint = pPrimitive->SamplePoint(uvSample.U, uvSample.V, normal);

		// Sample light
		uvSample = p_pScene->GetSampler()->Get2DSample();
		p_pScene->LightList[lightIdx]->SampleRadiance(uvSample.U, uvSample.V, lightPoint, normal, pdf);

		// Cast photon
		Vector3 photonDirection = Vector3::Normalize(objectPoint - lightPoint);
		//Vector3 photonDirection = Montecarlo::UniformSampleSphere(uvSample.U, uvSample.V);
		
		if (Vector3::Dot(photonDirection, normal) < 0.f)
			normal = -normal;

		Ray photonRay(lightPoint + normal * 1e-4f, photonDirection);



		if (p_pScene->Intersects(photonRay, intersection))
		{
			// Store photon intersection
			Photon photon;

			photon.Direction = photonDirection;
			photon.Position = intersection.Surface.PointWS;
			photon.Power = p_pScene->LightList[lightIdx]->Radiance(
				photon.Position, 
				intersection.Surface.ShadingBasisWS.W, 
				-photonDirection);

			m_photonMap.Insert(photon);
		}
	}

	std::cout << "Building photon map..." << std::endl;
	m_photonMap.Build();

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

	Spectrum pathThroughput(1.0f), 
		L(0.0f);

	IMaterial *pMaterial = NULL;

	bool specularBounce = false;

	BxDF::Type bxdfType;

	Vector3 wIn, wOut,
		wInLocal, wOutLocal; 
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
		
		lookup.Reset();

		if (m_photonMap.Lookup(p_intersection.Surface.PointWS, 1e-1f, lookup))
			L += lookup.radiance / lookup.points;

		return L;

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
		// Sample lights for direct lighting
		// -- If the currently intersected primitive is a luminaire, do not sample it 
		//----------------------------------------------------------------------------------------------
		if (!specularBounce)
			L += pathThroughput * SampleAllLights(p_pScene, p_intersection, p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut, p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);
			//L += pathThroughput * SampleAllLights(p_pScene, p_intersection, p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, wOut, p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);
			
		//----------------------------------------------------------------------------------------------
		// Sample bsdf for next direction
		//----------------------------------------------------------------------------------------------
		// Generate random samples
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
			float continueProbability = Maths::Min(0.5f, 0.33f * pathThroughput[0] + pathThroughput[1] + pathThroughput[2]);

			if (p_pScene->GetSampler()->Get1DSample() > continueProbability)
				break;
			pathThroughput /= continueProbability;
		}
	}

	return L;
}
//----------------------------------------------------------------------------------------------