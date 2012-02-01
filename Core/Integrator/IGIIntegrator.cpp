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
void IGIIntegrator::TraceVPLs(Scene *p_pScene, int p_nLightIdx, int p_nVPLPaths, int p_nMaxVPLs, int p_nMaxBounces, std::vector<VirtualPointLight> &p_vplList)
{
	Intersection intersection;
	IMaterial *pMaterial;
	BxDF::Type bxdfType;

	Spectrum alpha;

	Vector3 lightPoint,
		lightDirection,
		normal, wIn, wOut;

	Vector2 pSample2D[2];

	float pdf;
	int intersections;

	Ray lightRay;

	// std::cout << "Total paths : [" << p_nVPLPaths << "], Max VPLs : [" << p_nMaxVPLs << "], Max Bounces = [" << p_nMaxBounces << "]" << std::endl;

	for (int nVPLIndex = p_nVPLPaths; nVPLIndex > 0; )
	{
		// Sample light for ray, pdf and radiance along ray
		p_pScene->GetSampler()->Get2DSamples(pSample2D, 2);
		alpha = p_pScene->LightList[p_nLightIdx]->SampleRadiance(p_pScene, 
			pSample2D[0].U, pSample2D[0].V, pSample2D[1].U, pSample2D[1].V, lightRay, pdf);

		lightRay.Origin += lightRay.Direction * 1e-1f;

		if (pdf == 0.0f || alpha.IsBlack())
			continue;

		alpha /= pdf;

		// Do we have an intersection?
		for (intersections = 1; p_pScene->Intersects(lightRay, intersection); ++intersections)
		{
			wOut = -lightRay.Direction;
			pMaterial = intersection.GetMaterial();
			Spectrum Le = alpha * pMaterial->Rho(wOut, intersection.Surface) / Maths::Pi;

			VirtualPointLight vpl;

			vpl.Context = intersection;
			vpl.Direction = wOut;
			vpl.Power = Le;

			p_vplList.push_back(vpl);

			Spectrum f = SampleF(p_pScene, intersection, wOut, wIn, pdf, bxdfType);
			
			if (f.IsBlack() || pdf == 0.0f)
				break;

			if (intersections > p_nMaxBounces)
				break;

			Spectrum contribScale = f * Vector3::AbsDot(wIn, intersection.Surface.ShadingBasisWS.W) / pdf;

			// Possibly terminate virtual light path with Russian roulette
			float rrProb = Maths::Min(1.f, (contribScale[0] + contribScale[1] + contribScale[2]) * 0.33f);
			if (p_pScene->GetSampler()->Get1DSample() > rrProb)
					break;

			alpha *= contribScale / rrProb;

			//lightRay.Set(intersection.Surface.PointWS + wIn * 1E-3f, wIn, 1E-3f, Maths::Maximum);
			lightRay.Set(intersection.Surface.PointWS + wIn * m_fReflectEpsilon, wIn, 0.f, Maths::Maximum);
			
			//lightRay.Direction = wIn;
			//lightRay.Origin = intersection.Surface.PointWS + wIn * 1e-3f;
			//lightRay.Min = 1e-3f;
			//lightRay.Max = Maths::Maximum;
			//Vector3::Inverse(lightRay.Direction, lightRay.DirectionInverseCache);
		}

		--nVPLIndex;

		// std::cout << "Path [" << nVPLIndex << "] : Bounces [" << intersections << "]" << std::endl;
	}

	if (p_vplList.size() > p_nMaxVPLs)
	{
		// std::cout << "Trimming VPL list from [" << p_vplList.size() << "] to [" << p_nMaxVPLs << "]" << std::endl;
		p_vplList.erase(p_vplList.begin() + p_nMaxVPLs, p_vplList.end());
	}

	// std::cout << "VPL Count : [" << p_vplList.size() << "]" << std::endl; 
}
//----------------------------------------------------------------------------------------------
bool IGIIntegrator::Prepare(Scene *p_pScene)
{
	//VirtualPointLightList.clear();
	//TraceVPLs(p_pScene, 0, m_nMaxVPL, VirtualPointLightList);

	// Assume we're using a set of 9 VPL Lists
	for (int set = 0; set < m_nTileArea; ++set)
	{
		VirtualPointLightSet.push_back(std::vector<VirtualPointLight>());
		TraceVPLs(p_pScene, 0, m_nMaxPath, m_nMaxVPL, m_nMaxRayDepth, VirtualPointLightSet.back());
	}

	return true;
}
//----------------------------------------------------------------------------------------------
Spectrum IGIIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection)
{
	VisibilityQuery visibilityQuery(p_pScene),
		vplQuery(p_pScene);

	Spectrum pathThroughput(1.0f), 
		L(0.0f),
		E(0.0f);

	IMaterial *pMaterial = NULL;

	bool specularBounce = false;

	BxDF::Type bxdfType;

	Vector3 wIn, wOut,
		wInLocal, wOutLocal; 

	Vector2 sample;

	Ray ray(p_ray); 

	float pdf;
	int setId;

	if (m_nTileWidth == 1) 
		setId = 0;
	else
		setId = (int)p_pContext->SurfacePosition.X % m_nTileWidth + ((int)(p_pContext->SurfacePosition.Y) % m_nTileWidth) * m_nTileWidth;
	
	std::vector<VirtualPointLight> &vpll = VirtualPointLightSet[setId];
		

	//for (int rayDepth = 0; rayDepth < m_nMaxRayDepth; rayDepth++)
	for (int rayDepth = 0; rayDepth < 1; rayDepth++)
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
		// Sample lights for direct lighting
		// -- If the currently intersected primitive is a luminaire, do not sample it 
		//----------------------------------------------------------------------------------------------
		if (!specularBounce)
			L += pathThroughput * SampleAllLights(p_pScene, p_intersection, p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut, p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);

		E = 0;

		if (m_nIndirectSampleCount == 0)
		{
			std::vector<VirtualPointLight>::iterator vplIterator;
			
			for (vplIterator = vpll.begin(); vplIterator != vpll.end(); ++vplIterator)
			{
				const VirtualPointLight &vpl = *vplIterator;

				Vector3 distance = p_intersection.Surface.PointWS - vpl.Context.Surface.PointWS;
				float d2 = distance.LengthSquared();

				wIn = Vector3::Normalize(vpl.Context.Surface.PointWS - p_intersection.Surface.PointWS);
				Spectrum f = IIntegrator::F(p_pScene, p_intersection, wOut, wIn);;
			
				if (f.IsBlack()) 
					continue;
			
				float cosX = Maths::Max(0, Vector3::Dot(wIn, p_intersection.Surface.ShadingBasisWS.W));
				float cosY = Maths::Max(0, Vector3::Dot(-wIn, vpl.Context.Surface.ShadingBasisWS.W));
				float G = Maths::Min((cosX * cosY) / d2, 0.01f);

				Spectrum Llight = f * G * vpl.Power;
			
				vplQuery.SetSegment(p_intersection.Surface.PointWS, 1e-1f, vpl.Context.Surface.PointWS, 1e-1f);

				if (!vplQuery.IsOccluded())
					E += Llight;
			}

			L += E / vpll.size();
		}
		else
		{
			int stride = Maths::Max(1, vpll.size() / m_nIndirectSampleCount),
				contributions = 0;

			for (int vplIndex = 0; vplIndex < vpll.size(); vplIndex += stride)
			{
				contributions++;

				int sampledVPLIndex = (rand() % stride) + vplIndex;
			
				if (sampledVPLIndex >= vpll.size())
					break;

				const VirtualPointLight &vpl = vpll[sampledVPLIndex];


				Vector3 distance = p_intersection.Surface.PointWS - vpl.Context.Surface.PointWS;
				float d2 = distance.LengthSquared();

				wIn = Vector3::Normalize(vpl.Context.Surface.PointWS - p_intersection.Surface.PointWS);
				Spectrum f = IIntegrator::F(p_pScene, p_intersection, wOut, wIn);;
			
				if (f.IsBlack()) 
					continue;
			
				float cosX = Maths::Max(0, Vector3::Dot(wIn, p_intersection.Surface.ShadingBasisWS.W));
				float cosY = Maths::Max(0, Vector3::Dot(-wIn, vpl.Context.Surface.ShadingBasisWS.W));
				float G = Maths::Min((cosX * cosY) / d2, 0.01f);

				Spectrum Llight = f * G * vpl.Power;

				vplQuery.SetSegment(p_intersection.Surface.PointWS, 1e-1f, vpl.Context.Surface.PointWS, 1e-1f);

				if (!vplQuery.IsOccluded())
					E += Llight;
			}

			L += E / contributions;
		}

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
		ray.Set(p_intersection.Surface.PointWS + wIn * m_fReflectEpsilon, wIn, 0.f, Maths::Maximum);

		//ray.Min = 0.f;
		//ray.Max = Maths::Maximum;
		//ray.Origin = p_intersection.Surface.PointWS + wIn * m_fReflectEpsilon;
		//ray.Direction = wIn;
		//Vector3::Inverse(ray.Direction, ray.DirectionInverseCache);
		
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

		return L;
	}

	return L;
}
//----------------------------------------------------------------------------------------------