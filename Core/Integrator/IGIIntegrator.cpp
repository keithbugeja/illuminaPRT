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
IGIIntegrator::IGIIntegrator(const std::string &p_strName, int p_nMaxVPL, int p_nMaxRayDepth, int p_nShadowSampleCount, float p_fReflectEpsilon)
	: IIntegrator(p_strName) 
	, m_nMaxVPL(p_nMaxVPL)
	, m_nShadowSampleCount(p_nShadowSampleCount)
	, m_nMaxRayDepth(p_nMaxRayDepth)
	, m_fReflectEpsilon(p_fReflectEpsilon)
{ }
//----------------------------------------------------------------------------------------------
IGIIntegrator::IGIIntegrator(int p_nMaxVPL, int p_nMaxRayDepth, int p_nShadowSampleCount, float p_fReflectEpsilon)
	: m_nMaxVPL(p_nMaxVPL) 
	, m_nShadowSampleCount(p_nShadowSampleCount)
	, m_nMaxRayDepth(p_nMaxRayDepth)
	, m_fReflectEpsilon(p_fReflectEpsilon)
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
void IGIIntegrator::TraceVPLs(Scene *p_pScene, int p_nLightIdx, int p_nVPLCount, std::vector<VirtualPointLight> &p_vplList)
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

	std::cout << p_nVPLCount << std::endl;

	Ray lightRay;

	for (int nVPLIndex = p_nVPLCount; nVPLIndex > 0; )
	{
		// Sample point on light source
		p_pScene->GetSampler()->Get2DSamples(pSample2D, 2);
		lightPoint = p_pScene->LightList[p_nLightIdx]->SamplePoint(pSample2D[0].U, pSample2D[0].V, normal, pdf);

		p_pScene->GetSampler()->Get2DSamples(pSample2D, 2);
		alpha = p_pScene->LightList[p_nLightIdx]->SampleRadiance(p_pScene, 
			pSample2D[0].U, pSample2D[0].V, pSample2D[1].U, pSample2D[1].V, lightRay, pdf);

		//OrthonormalBasis basis; normal = -normal; basis.InitFromW(normal);
		//sample2D = p_pScene->GetSampler()->Get2DSample();

		//lightDirection = Montecarlo::UniformSampleCone(sample2D.U, sample2D.V, 0.05f, basis);
		//alpha = p_pScene->LightList[p_nLightIdx]->Radiance(lightPoint, normal, lightDirection);

		//p_pScene->LightList[p_nLightIdx]->SampleRadiance(

		// Initialise ray
		//Ray lightRay(lightPoint + lightDirection * 1e-1f, lightDirection);

		int intersections = 0;

		// Do we have an intersection?
		while (p_pScene->Intersects(lightRay, intersection))
		{
			intersections++;

			wOut = -lightRay.Direction;
			pMaterial = intersection.GetMaterial();
			Spectrum Le = alpha * pMaterial->Rho(wOut, intersection.Surface) / Maths::Pi;

			VirtualPointLight vpl;

			vpl.Position = intersection.Surface.PointWS;
			vpl.Normal = intersection.Surface.ShadingBasisWS.W;
			vpl.Power = Le;
			vpl.Direction = wOut;
			vpl.SurfaceIntersection = intersection;

			p_vplList.push_back(vpl);

			Spectrum f = SampleF(p_pScene, intersection, wOut, wIn, pdf, bxdfType);
			
			if (f.IsBlack() || pdf == 0.0f)
				break;

			Spectrum alphaNew = alpha * f * Vector3::AbsDot(wIn, intersection.Surface.ShadingBasisWS.W) / pdf;
			float luminanceRatio = ((alphaNew[0] + alphaNew[1] + alphaNew[2]) * 0.33) / ((alpha[0] + alpha[1] + alpha[2]) * 0.33);

			if (p_pScene->GetSampler()->Get1DSample() > luminanceRatio)
				break;

			alpha = alphaNew / luminanceRatio;

			lightRay.Direction = wIn;
			lightRay.Origin = intersection.Surface.PointWS + wIn * 1e-3f;
			lightRay.Min = 1e-3f;
			lightRay.Max = Maths::Maximum;

			//VirtualPointLight vpl;

			//vpl.Position = intersection.Surface.PointWS;
			//vpl.Direction = lightDirection;
			//vpl.Normal = intersection.Surface.ShadingBasisWS.W;
			//vpl.Power = alpha * intersection.GetMaterial()->Rho(vpl.Direction, intersection.Surface) / Maths::Pi;
			//	// p_pScene->LightList[p_nLightIdx]->Radiance(intersection.Surface.PointWS, normal, lightDirection) * intersection.GetMaterial()->Rho(vpl.Direction, intersection.Surface) / Maths::Pi;
			//vpl.SurfaceIntersection = intersection;

			//p_vplList.push_back(vpl);

			std::cout << vpl.Position.ToString() << std::endl;
			std::cout << vpl.Power.ToString() << std::endl;
		}

		--nVPLIndex;
		std::cout << "Path [" << nVPLIndex << "] : Bounces [" << intersections << "]" << std::endl;
	}
}
//----------------------------------------------------------------------------------------------
bool IGIIntegrator::Prepare(Scene *p_pScene)
{
	VirtualPointLightList.clear();

	TraceVPLs(p_pScene, 0, m_nMaxVPL, VirtualPointLightList);

	return true;
}
//----------------------------------------------------------------------------------------------
Spectrum IGIIntegrator::Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection)
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
		// Sample lights for direct lighting
		// -- If the currently intersected primitive is a luminaire, do not sample it 
		//----------------------------------------------------------------------------------------------
		//if (!specularBounce)
			//L += pathThroughput * SampleAllLights(p_pScene, p_intersection, p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOut, p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);
			//L += pathThroughput * SampleAllLights(p_pScene, p_intersection, p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, wOut, p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);

		std::vector<VirtualPointLight>::iterator vplIterator;
		
		VisibilityQuery vplQuery(p_pScene);
		
		int contributions = 0;
		Spectrum E;

		for (vplIterator = VirtualPointLightList.begin(); vplIterator != VirtualPointLightList.end(); ++vplIterator)
		{
			const VirtualPointLight &vpl = *vplIterator;

			Vector3 distance = p_intersection.Surface.PointWS - vpl.SurfaceIntersection.Surface.PointWS;
			float d2 = distance.LengthSquared();

			wIn = Vector3::Normalize(vpl.SurfaceIntersection.Surface.PointWS - p_intersection.Surface.PointWS);
			Spectrum f = IIntegrator::F(p_pScene, p_intersection, wOut, wIn) * d2;
			
			// if (f.IsBlack()) continue;
			
			float cosX = Maths::Max(0, Vector3::Dot(wIn, p_intersection.Surface.ShadingBasisWS.W));
			float cosY = Maths::Max(0, Vector3::Dot(-wIn, vpl.SurfaceIntersection.Surface.ShadingBasisWS.W));

			float G = (cosX * cosY) / d2;

			// float G = (Vector3::AbsDot(wIn, p_intersection.Surface.ShadingBasisWS.W) * Vector3::AbsDot(wIn, vpl.Normal)) / d2;
			//float G = Maths::Max(0, Vector3::Dot(wIn, p_intersection.Surface.ShadingNormal)); // / Maths::Abs(d2);
			//float cx = Vector3::Dot(wIn, p_intersection.Surface.ShadingBasisWS.W),
				//cy = Vector3::Dot(wIn, vpl.Normal);

			//float G = Maths::Abs(cx * cy) / Maths::FAbs(d2);
			//float G = (Vector3::Dot(wIn, p_intersection.Surface.ShadingBasisWS.W)  * Vector3::Dot(wIn, vpl.Normal)) / Maths::FAbs(d2);

			//std::cout << "G: " << G << std::endl;

			Spectrum Llight = f * G * vpl.Power; // factor this out -> 1 / VirtualPointLightList.size();
			
			contributions++;

			vplQuery.SetSegment(p_intersection.Surface.PointWS, 1e-1f, vpl.Position, 1e-1f);

			if (!vplQuery.IsOccluded())
				E += Llight;
		}

		L = E / contributions;

		/*
		Spectrum E_x(0);

		std::vector<VirtualPointLight>::iterator vplIterator;
		
		for (vplIterator = VirtualPointLightList.begin(); vplIterator != VirtualPointLightList.end(); ++vplIterator)
		{
			const VirtualPointLight &vpl = *vplIterator;

			//Vector3 distanceVector = p_intersection.Surface.PointWS - vpl.Position;
			//float distanceSquared = distanceVector.LengthSquared();
			//
			//Vector3 wIn = Vector3::Normalize(distanceVector);
			//Spectrum f = IIntegrator::F(p_pScene, p_intersection, wOut, wIn) * distanceSquared;

			//float G = Vector3::AbsDot(wIn, p_intersection.Surface.ShadingBasisWS.W) * Vector3::AbsDot(wIn, vpl.Normal) / distanceSquared;
			//Spectrum E_x = f * G * vpl.Power / VirtualPointLightList.size();

			Vector3 x_xi = p_intersection.Surface.PointWS - vpl.Position;
			Vector3 x_xi_hat = Vector3::Normalize(x_xi);
			float distanceSquared = x_xi.LengthSquared();

			//float G = (Maths::Max(0, Vector3::Dot(-x_xi_hat, p_intersection.Surface.ShadingBasisWS.W)) * Maths::Max(0, Vector3::Dot(x_xi_hat, vpl.Normal))) / distanceSquared;
			//Spectrum f = IIntegrator::F(p_pScene, p_intersection, x_xi_hat, wOut) * distanceSquared;
			//Spectrum E_x = f * G * vpl.Power / VirtualPointLightList.size();

			////Spectrum f = IIntegrator::F(p_pScene, vpl.SurfaceIntersection, -vpl.Direction, x_xi_hat);
			
			float cosThetaX = Vector3::Dot(-x_xi_hat, p_intersection.Surface.ShadingBasisWS.W),
				cosThetaY = Vector3::Dot(x_xi_hat, vpl.Normal),
				cosThetaXY = cosThetaX * cosThetaY,
				G = (cosThetaX * cosThetaY) / Maths::Max(Maths::Epsilon, distanceSquared);

			//Spectrum f = IIntegrator::F(p_pScene, vpl.SurfaceIntersection, -vpl.Direction, x_xi_hat);
			Spectrum f = IIntegrator::F(p_pScene, p_intersection, x_xi_hat, wOut);
			
			//float E2L = Maths::Pi * 2;

			//std::cout << f.ToString() << std::endl;

			E_x = vpl.Power / distanceSquared; // (vpl.Power * G * f * distanceSquared) / VirtualPointLightList.size();

			//visibilityQuery.SetSegment(vpl.Position + vpl.Normal * 1e-01f, 1e-01f, p_intersection.Surface.PointWS + p_intersection.Surface.ShadingBasisWS.W * 1e-01f, 1e-01f);
			
			//if (!visibilityQuery.IsOccluded())
			{
				E_x[0] = Maths::Max(0, E_x[0]);
				E_x[1] = Maths::Max(0, E_x[1]);
				E_x[2] = Maths::Max(0, E_x[2]);

				L += E_x; // * f * f2;
			}
		}
		*/

		// std::cout << L.ToString() << std::endl;

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