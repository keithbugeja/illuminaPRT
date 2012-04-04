//----------------------------------------------------------------------------------------------
//	Filename:	IGIIntegrator.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iostream>
#include "Integrator/IGIIntegrator.h"

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

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
IGIIntegrator::IGIIntegrator(const std::string &p_strName, int p_nMaxVPL, int p_nMaxPath, int p_nTileWidth, float p_fGTermMax, int p_nMaxRayDepth, int p_nShadowSampleCount, int p_nIndirectSampleCount, float p_fReflectEpsilon)
	: IIntegrator(p_strName) 
	, m_nMaxVPL(p_nMaxVPL)
	, m_nMaxPath(p_nMaxPath)
	, m_nTileWidth(p_nTileWidth)
	, m_nTileArea(p_nTileWidth * p_nTileWidth)
	, m_nShadowSampleCount(p_nShadowSampleCount)
	, m_nIndirectSampleCount(p_nIndirectSampleCount)
	, m_nMaxRayDepth(p_nMaxRayDepth)
	, m_fReflectEpsilon(p_fReflectEpsilon)
	, m_fGTermMax(p_fGTermMax)
{ }
//----------------------------------------------------------------------------------------------
IGIIntegrator::IGIIntegrator(int p_nMaxVPL, int p_nMaxPath, int p_nTileWidth, float p_fGTermMax, int p_nMaxRayDepth, int p_nShadowSampleCount, int p_nIndirectSampleCount, float p_fReflectEpsilon)
	: IIntegrator()
	, m_nMaxVPL(p_nMaxVPL)
	, m_nMaxPath(p_nMaxPath)
	, m_nTileWidth(p_nTileWidth)
	, m_nTileArea(p_nTileWidth * p_nTileWidth)
	, m_nShadowSampleCount(p_nShadowSampleCount)
	, m_nIndirectSampleCount(p_nIndirectSampleCount)
	, m_nMaxRayDepth(p_nMaxRayDepth)
	, m_fReflectEpsilon(p_fReflectEpsilon)
	, m_fGTermMax(p_fGTermMax)
{ }
//----------------------------------------------------------------------------------------------
bool IGIIntegrator::Initialise(Scene *p_pScene, ICamera *p_pCamera)
{
	return true;
}
//----------------------------------------------------------------------------------------------
bool IGIIntegrator::Shutdown(void)
{
	return true;
}
//----------------------------------------------------------------------------------------------
void IGIIntegrator::TraceVirtualPointLights(Scene *p_pScene, int p_nMaxPaths, int p_nMaxPointLights, int p_nMaxBounces, std::vector<VirtualPointLight> &p_virtualPointLightList)
{
	VirtualPointLight pointLight;
	Intersection intersection;
	IMaterial *pMaterial;
	BxDF::Type bxdfType;
	Ray lightRay;

	Spectrum contribution, 
		alpha, f;

	Vector3 normal, 
		wOut, wIn;

	Vector2 pSample2D[2];

	float continueProbability, 
		pdf;

	int intersections;

	// Trace either a maximum number of paths or virtual point lights
	for (int lightIndex = 0, nPathIndex = p_nMaxPaths; nPathIndex > 0 && p_virtualPointLightList.size() < p_nMaxPointLights; --nPathIndex)
	{
		// Get samples for initial position and direction
		p_pScene->GetSampler()->Get2DSamples(pSample2D, 2);
		
		// Get initial radiance, position and direction
		alpha = p_pScene->LightList[lightIndex]->SampleRadiance(
			p_pScene, pSample2D[0].U, pSample2D[0].V, 
			pSample2D[1].U, pSample2D[1].V, lightRay, pdf);

		// std::cout << "Lightray : " << lightRay.Direction.ToString() << std::endl;

		// If pdf or radiance are zero, choose a new path
		if (pdf == 0.0f || alpha.IsBlack())
			continue;

		// Scale radiance by pdf
		alpha /= pdf;

		// Adjust ray origin to avoid intersecting light geometry
		lightRay.Origin += lightRay.Direction * 1e-1f;

		// Start tracing virtual point light path
		for (intersections = 1; p_pScene->Intersects(lightRay, intersection); ++intersections)
		{
			wOut = -lightRay.Direction;
			pMaterial = intersection.GetMaterial();
			Spectrum Le = alpha * pMaterial->Rho(wOut, intersection.Surface) / Maths::Pi;

			// Set point light parameters
			pointLight.Context = intersection;
			pointLight.Direction = wOut;
			pointLight.Contribution = Le;

			// Push point light on list
			p_virtualPointLightList.push_back(pointLight);

			// Sample new direction
			f = SampleF(p_pScene, intersection, wOut, wIn, pdf, bxdfType);
			
			// If reflectivity or pdf are zero, end path
			if (f.IsBlack() || pdf == 0.0f || intersections > p_nMaxBounces)
				break;

			// Compute contribution of path
			contribution = f * Vector3::AbsDot(wIn, intersection.Surface.ShadingBasisWS.W) / pdf;

			// Possibly terminate virtual light path with Russian roulette
			continueProbability = Maths::Min(1.f, (contribution[0] + contribution[1] + contribution[2]) * 0.33f);
			if (p_pScene->GetSampler()->Get1DSample() > continueProbability)
					break;

			// Modify contribution accordingly
			alpha *= contribution / continueProbability;

			// Set new ray position and direction
			lightRay.Set(intersection.Surface.PointWS + wIn * m_fReflectEpsilon, wIn, 0.f, Maths::Maximum);
			//lightRay.Set(intersection.Surface.PointWS, wIn, m_fReflectEpsilon); 
		}

		// Increment light index, and reset if we traversed all scene lights
		if (++lightIndex == p_pScene->LightList.Size())
			lightIndex = 0;
	}

	// Just in case we traced more than is required
	if (p_virtualPointLightList.size() > p_nMaxPointLights)
		p_virtualPointLightList.erase(p_virtualPointLightList.begin() + p_nMaxPointLights, p_virtualPointLightList.end());
}
//----------------------------------------------------------------------------------------------
bool IGIIntegrator::Prepare(Scene *p_pScene)
{
	VirtualPointLightSet.clear();
	// p_pScene->GetSampler()->Reset();

	for (int pointLightSet = m_nTileArea; pointLightSet != 0; --pointLightSet)
	{
		VirtualPointLightSet.push_back(std::vector<VirtualPointLight>());
		TraceVirtualPointLights(p_pScene, m_nMaxPath, m_nMaxVPL, m_nMaxRayDepth, VirtualPointLightSet.back());
	}

	return true;
}
//----------------------------------------------------------------------------------------------
Spectrum IGIIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, Intersection &p_intersection, RadianceContext *p_pRadianceContext)
{
	float samplesUsed, 
		pdf;

	Vector3 wIn;

	// Avoid having to perform multiple checks for a NULL radiance context
	RadianceContext radianceContext;

	if (p_pRadianceContext == NULL)
		p_pRadianceContext = &radianceContext;
	
	// Initialise context
	p_pRadianceContext->Indirect = 
		p_pRadianceContext->Direct = 
		p_pRadianceContext->Albedo = 0.f;

	// Visibility query
	VisibilityQuery pointLightQuery(p_pScene);

	// Initialise point light set
	std::vector<VirtualPointLight> &pointLightSet =
		VirtualPointLightSet[(m_nTileWidth > 1) ? (int)p_pContext->SurfacePosition.X % m_nTileWidth + ((int)(p_pContext->SurfacePosition.Y) % m_nTileWidth) * m_nTileWidth : 0];

	std::vector<VirtualPointLight>::iterator pointLightIterator;
	
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

			// Reset sample - we want exactly the same sequence over and again
			// p_pScene->GetSampler()->Reset();
			
			if (!p_intersection.IsEmissive())
			{
				// Sample direct lighting
				p_pRadianceContext->Direct = SampleAllLights(p_pScene, p_intersection, 
					p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, 
					wOut, p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);

				// Set albedo
				p_pRadianceContext->Albedo = pMaterial->Rho(wOut, p_intersection.Surface);
				
				/*
				for (samplesUsed = 1, pointLightIterator = pointLightSet.begin(); 
					 pointLightIterator != pointLightSet.end(); ++pointLightIterator)
				{
					VirtualPointLight &pointLight = *pointLightIterator;

					float d2 = Vector3::DistanceSquared(p_intersection.Surface.PointWS, pointLight.Context.Surface.PointWS);
					wIn = Vector3::Normalize(pointLight.Context.Surface.PointWS - p_intersection.Surface.PointWS);
					float G =	Vector3::AbsDot(wIn, p_intersection.Surface.ShadingBasisWS.W) * 
								Vector3::AbsDot(wIn, pointLight.Context.Surface.ShadingBasisWS.W) / d2;
					G = Maths::Min(G, m_fGTermMax);
					Spectrum f = IIntegrator::F(p_pScene, p_intersection, wOut, wIn);

					if (f.IsBlack()) 
						continue;

					Spectrum Lcontrib = f * pointLight.Contribution * G;

					//if (Llight.y() < rrThreshold) {
					//	float continueProbability = .1f;
					//	if (rng.RandomFloat() > continueProbability)
					//		continue;
					//	Llight /= continueProbability;
					//}

					pointLightQuery.SetSegment(p_intersection.Surface.PointWS, wIn, Maths::Sqrt(d2), 1e-3f);

					// Ignore if such is the case.
					if (pointLightQuery.IsOccluded()) 
						continue; 

					p_pRadianceContext->Indirect += Lcontrib;
					samplesUsed++;
				} */

				/*
				float d2 = DistanceSquared(p, vl.p);
				Vector wi = Normalize(vl.p - p);
				float G = AbsDot(wi, n) * AbsDot(wi, vl.n) / d2;
				G = min(G, gLimit);
				Spectrum f = bsdf->f(wo, wi);
				if (G == 0.f || f.IsBlack()) continue;
				Spectrum Llight = f * G * vl.pathContrib / nLightPaths;
				RayDifferential connectRay(p, wi, ray, isect.rayEpsilon,
										   sqrtf(d2) * (1.f - vl.rayEpsilon));
				Llight *= renderer->Transmittance(scene, connectRay, NULL, rng, arena);

				// Possibly skip virtual light shadow ray with Russian roulette
				if (Llight.y() < rrThreshold) {
					float continueProbability = .1f;
					if (rng.RandomFloat() > continueProbability)
						continue;
					Llight /= continueProbability;
				}

				// Add contribution from _VirtualLight_ _vl_
				if (!scene->IntersectP(connectRay))
					L += Llight;
				*/

				/**/
				for (samplesUsed = 1, pointLightIterator = pointLightSet.begin(); 
					 pointLightIterator != pointLightSet.end(); ++pointLightIterator)
				{
					VirtualPointLight &pointLight = *pointLightIterator;

					wIn = Vector3::Normalize(pointLight.Context.Surface.PointWS - p_intersection.Surface.PointWS);
					
					if (wIn.Dot(pointLight.Context.Surface.ShadingBasisWS.W) < 0.f)
					{
						// Sample reflectivity (we sample this first as its a faster 
						// early out than an intersection test)
						Spectrum f = IIntegrator::F(p_pScene, p_intersection, wOut, wIn);;
			
						if (f.IsBlack()) 
							continue;

						// Test immediately if the point light is occluded
						//pointLightQuery.SetSegment(p_intersection.Surface.PointWS, 0, pointLight.Context.Surface.PointWS, 0);
						pointLightQuery.SetSegment(p_intersection.Surface.PointWS, 1e-4f, pointLight.Context.Surface.PointWS, 1e-4f);

						// Ignore if such is the case.
						if (pointLightQuery.IsOccluded()) 
							continue; 

						// Compute geometry term
						#if (defined(SSE_ENABLED))
							__m128 surfacePoint		= _mm_load_ps(p_intersection.Surface.PointWS.Element);
							__m128 lightPosition	= _mm_load_ps(pointLight.Context.Surface.PointWS.Element); 

							__m128 distance	= _mm_sub_ps(surfacePoint, lightPosition);
							float d2 = _mm_rcp_ss(_mm_dp_ps(distance, distance, 0x71)).m128_f32[0];
						#else
							float d2 = 1.f / Vector3::DistanceSquared(p_intersection.Surface.PointWS, 
								pointLight.Context.Surface.PointWS);
						#endif
			
						const float G = Maths::Min(
							Vector3::Dot(pointLight.Context.Surface.ShadingBasisWS.W, -wIn) * 
							Vector3::AbsDot(p_intersection.Surface.ShadingBasisWS.W, wIn) * d2,
							m_fGTermMax);
						
						p_pRadianceContext->Indirect += f * pointLight.Contribution * G;
						samplesUsed++;
					}
				}
				/**/

				p_pRadianceContext->Indirect = p_pRadianceContext->Indirect / pointLightSet.size(); //samplesUsed;
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
		Ray ray(p_intersection.Surface.RayOriginWS, -p_intersection.Surface.RayDirectionWS);

		for (size_t lightIndex = 0; lightIndex < p_pScene->LightList.Size(); ++lightIndex)
			p_pRadianceContext->Direct += p_pScene->LightList[lightIndex]->Radiance(ray);
	}

	p_pRadianceContext->Final = 
		p_pRadianceContext->Direct + p_pRadianceContext->Indirect;

	return p_pRadianceContext->Final;
}
//----------------------------------------------------------------------------------------------
Spectrum IGIIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection, RadianceContext *p_pRadianceContext)
{
	// Compute intersection step
	if (!p_pScene->Intersects(Ray(p_ray), p_intersection))
	{
		p_intersection.Surface.RayOriginWS = p_ray.Origin;
		p_intersection.Surface.RayDirectionWS = p_ray.Direction;
	}

	return Radiance(p_pContext, p_pScene, p_intersection, p_pRadianceContext);

	/*
	VisibilityQuery pointLightQuery(p_pScene);

	Vector3 wIn;
	Vector2 sample;

	float pdf;

	// Copy-construct a new ray object to keep original's constness
	Ray ray(p_ray); 

	// Avoid having to perform multiple checks for a NULL radiance context
	RadianceContext radianceContext;

	if (p_pRadianceContext == NULL)
		p_pRadianceContext = &radianceContext;
	
	// Initialise context
	p_pRadianceContext->Indirect = 
		p_pRadianceContext->Direct = 
		p_pRadianceContext->Albedo = 0.f;

	std::vector<VirtualPointLight> &pointLightSet =
		VirtualPointLightSet[(m_nTileWidth > 1) ? (int)p_pContext->SurfacePosition.X % m_nTileWidth + ((int)(p_pContext->SurfacePosition.Y) % m_nTileWidth) * m_nTileWidth : 0];
	
	std::vector<VirtualPointLight>::iterator pointLightIterator;

	//----------------------------------------------------------------------------------------------
	// No intersection
	//----------------------------------------------------------------------------------------------
	if(!p_pScene->Intersects(ray, p_intersection))
	{
		for (size_t lightIndex = 0; lightIndex < p_pScene->LightList.Size(); ++lightIndex)
			p_pRadianceContext->Direct += p_pScene->LightList[lightIndex]->Radiance(-ray);
	}
	else
	{
		p_pRadianceContext->SetSpatialContext(&p_intersection);

		if (p_intersection.HasMaterial()) 
		{
			// Get material for intersection primitive
			IMaterial *pMaterial = p_intersection.GetMaterial();

			//----------------------------------------------------------------------------------------------
			// Sample lights for specular / first bounce
			//----------------------------------------------------------------------------------------------
			Vector3 wOut = -Vector3::Normalize(ray.Direction);

			// Add emitted light : only on first bounce or specular to avoid double counting
			if (!p_intersection.IsEmissive())
			{
				//----------------------------------------------------------------------------------------------
				// Sample lights for direct lighting
				// -- If the currently intersected primitive is a luminaire, do not sample it 
				//----------------------------------------------------------------------------------------------
				p_pRadianceContext->Direct = SampleAllLights(p_pScene, p_intersection, 
					p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut, 
					p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);

				p_pRadianceContext->Albedo = pMaterial->Rho(wOut, p_intersection.Surface);

				//----------------------------------------------------------------------------------------------
				// Sample indirect from virtual point lights
				//----------------------------------------------------------------------------------------------
				for (pointLightIterator = pointLightSet.begin(); pointLightIterator != pointLightSet.end(); ++pointLightIterator)
				{
					const VirtualPointLight &pointLight = *pointLightIterator;

					float distanceSquared = Vector3::DistanceSquared(
						p_intersection.Surface.PointWS, 
						pointLight.Context.Surface.PointWS);
				
					wIn = Vector3::Normalize(pointLight.Context.Surface.PointWS - p_intersection.Surface.PointWS);
						
					Spectrum f = IIntegrator::F(p_pScene, p_intersection, wOut, wIn);;
			
					if (f.IsBlack()) 
						continue;

					pointLightQuery.SetSegment(p_intersection.Surface.PointWS, 1e-1f, 
						pointLight.Context.Surface.PointWS, 1e-1f);

					if (pointLightQuery.IsOccluded())
						continue;

					float cosX = Maths::Max(0, Vector3::Dot(wIn, p_intersection.Surface.ShadingBasisWS.W));
					float cosY = Maths::Max(0, Vector3::Dot(-wIn, pointLight.Context.Surface.ShadingBasisWS.W));
					float G = Maths::Min((cosX * cosY) / distanceSquared, 0.01f);

					p_pRadianceContext->Indirect += f * G * pointLight.Power;
				}

				p_pRadianceContext->Indirect = p_pRadianceContext->Indirect / pointLightSet.size();
			}
			else
			{
				p_pRadianceContext->Direct = p_intersection.GetLight()->Radiance(p_intersection.Surface.PointWS, 
					p_intersection.Surface.GeometryBasisWS.W, wOut);
			}
		}
	}

	p_pRadianceContext->Final = 
		p_pRadianceContext->Direct + p_pRadianceContext->Indirect;

	return p_pRadianceContext->Final;
	*/
}
//----------------------------------------------------------------------------------------------