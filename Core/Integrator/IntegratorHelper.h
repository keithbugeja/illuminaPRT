//----------------------------------------------------------------------------------------------
//	Filename:	PointShader.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
// Todo: Generalise structures used... there is way too much repetition (copy-pasting and 
//	     tweaking) of functionality available elsewhere in the core.
//----------------------------------------------------------------------------------------------
#pragma once

#include <System/IlluminaPRT.h>
#include <System/FactoryManager.h>

#include <Geometry/Spline.h>
#include <Maths/Montecarlo.h>
#include <Sampler/LowDiscrepancySampler.h>
#include <Sampler/RandomSampler.h>
#include <Material/Material.h>
#include <Scene/Visibility.h>

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
struct VirtualPointSource
{
	Spectrum Contribution;
	Vector3	Position;
	Vector3 Normal;
	Vector3 Direction;
	bool Invalid;
};

//----------------------------------------------------------------------------------------------
template <class T>
class IntegratorHelper
{
protected:
	// For hemisphere sampling (distribution raytracing style)
	int m_nAzimuthStrata,
		m_nAltitudeStrata;

	// For virtual point source-based lighting
	std::vector<VirtualPointSource> m_virtualPointSourceList;

	RandomSampler m_sequenceSampler;

	LowDiscrepancySampler m_positionSampler,
		m_directionSampler, m_continueSampler;
	
	int m_nVPSources, m_nVPSets,
		m_nMaxVPBounces;

	float m_fMaxGTerm;

	// General parameters
	int	m_nShadowRays,
		m_nRayDepth;

	unsigned int m_unGeneratorSeed;

	float m_fReflectEpsilon;

	Scene *m_pScene;

	//----------------------------------------------------------------------------------------------
	// Shading type for individual points
	//----------------------------------------------------------------------------------------------
public:
	enum ShadingType
	{
		PointLit,
		PathTraced
	};
	
	//----------------------------------------------------------------------------------------------
	// One-time intialisation
	//----------------------------------------------------------------------------------------------
public:
	void Initialise(Scene *p_pScene, float p_fReflectEpsilon, int p_nRayDepth, int p_nShadowRayCount)
	{
		m_pScene = p_pScene;

		m_fReflectEpsilon = p_fReflectEpsilon;

		m_nRayDepth = p_nRayDepth;
		m_nShadowRays = p_nShadowRayCount;
	}

	//----------------------------------------------------------------------------------------------
	// Shader type specific arguments
	//----------------------------------------------------------------------------------------------
	void SetHemisphereDivisions(int p_nAzimuthStrata, int p_nAltitudeStrata) 
	{
		m_nAltitudeStrata = p_nAltitudeStrata;
		m_nAzimuthStrata = p_nAzimuthStrata;
	}

	void SetVirtualPointSources(int p_nMaxVirtualPointSources, int p_nVirtualPointSets, int p_nMaxBounces)
	{
		m_nVPSources = p_nMaxVirtualPointSources;
		m_nVPSets = p_nVirtualPointSets;
		m_nMaxVPBounces = p_nMaxBounces;
	}

	void SetGeometryTerm(float p_fMaxGeometryTerm)
	{
		m_fMaxGTerm = p_fMaxGeometryTerm;
	}

	//----------------------------------------------------------------------------------------------
	void SetGeneratorSeed(int p_unSeed)
	{
		m_unGeneratorSeed = p_unSeed;
	}

	//----------------------------------------------------------------------------------------------
	// Shader preparation
	//----------------------------------------------------------------------------------------------
	void Prepare(enum ShadingType p_eShadingType)
	{
		if (p_eShadingType == PointLit)
		{
			// Reset samplers
			m_positionSampler.Reset(m_unGeneratorSeed);
			m_directionSampler.Reset(m_unGeneratorSeed);
			m_continueSampler.Reset(m_unGeneratorSeed);
			m_sequenceSampler.Reset(m_unGeneratorSeed);

			// Clear point source list
			m_virtualPointSourceList.clear();

			TraceVirtualPointSources(m_virtualPointSourceList, m_nVPSources * m_nVPSets, m_nMaxVPBounces);
		}
	}

public:
	//----------------------------------------------------------------------------------------------
	// Shade points
	//----------------------------------------------------------------------------------------------
	void Shade(T* p_pPoint, ShadingType p_eShadingType)
	{
		//if (p_pPoint->Invalid)
		{
			if (p_eShadingType == PointLit)
				p_pPoint->Irradiance = LiPointLit(p_pPoint->Position, p_pPoint->Normal);
			else
				p_pPoint->Irradiance = LiPathTraced(p_pPoint->Position, p_pPoint->Normal);

			//p_pPoint->Invalid = false;
		}
	}

	/*
	void Shade(std::vector<T*> &p_pointList, ShadingType p_eShadingType)
	{
		if (p_eShadingType == PointLit)
		{
			for (auto point : p_pointList)
			{
				if (point->Invalid)
				{
					point->Irradiance = LiPointLit(point->Position, point->Normal);
					point->Invalid = false;
				}
			}
		}
		else
		{
			for (auto point : p_pointList)
			{
				if (point->Invalid)
				{
					point->Irradiance = LiPathTraced(point->Position, point->Normal);
					point->Invalid = false;
				}
			}
		}
	}
	*/

protected:
	//----------------------------------------------------------------------------------------------
	// Path-traced radiance computation
	//----------------------------------------------------------------------------------------------
	Spectrum LiPathTraced(Vector3 &p_position, Vector3 &p_normal)
	{
		Intersection isect;
		Vector3 direction;
		Vector2 sample2D;
		Ray ray;

		Spectrum E = 0;
	
		OrthonormalBasis basis; 
		basis.InitFromW(p_normal);
	
		// Cache this - it doesn't change!
		int mn = m_nAzimuthStrata * m_nAltitudeStrata;

		for (int altitudeIndex = 0; altitudeIndex < m_nAltitudeStrata; altitudeIndex++)
		{
			for (int azimuthIndex = 0; azimuthIndex < m_nAzimuthStrata; azimuthIndex++)
			{
				sample2D = m_pScene->GetSampler()->Get2DSample();

				direction = Montecarlo::CosineSampleHemisphere(sample2D.X, sample2D.Y, (float)altitudeIndex, (float)azimuthIndex, (float)m_nAltitudeStrata, (float)m_nAzimuthStrata); 
				direction = basis.Project(direction);

				ray.Set(p_position + direction * m_fReflectEpsilon, direction, m_fReflectEpsilon, Maths::Maximum);

				E += TracePath(ray);
			}
		}

		return E / mn;
	}

	/*
	 * Trace path and return radiance
	 */
	Spectrum TracePath(Ray &p_ray)
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
		
			if (!m_pScene->Intersects(ray, isect))
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
			L += pathThroughput * IIntegrator::SampleAllLights(m_pScene, isect, 
				isect.Surface.PointWS, isect.Surface.ShadingBasisWS.W, wOut, 
				m_pScene->GetSampler(), isect.GetLight(), m_nShadowRays);

			if (pathLength + 1 == m_nRayDepth) break;

			// Sample bsdf for next direction
			Vector2 sample = m_pScene->GetSampler()->Get2DSample();

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
				//float continueProbability = Maths::Min(0.5f, 0.33f * (pathThroughput[0] + pathThroughput[1] + pathThroughput[2]));
				float continueProbability = Maths::Min(1.0f, pathThroughput.Luminance());

				if (m_pScene->GetSampler()->Get1DSample() > continueProbability)
					break;

				pathThroughput /= continueProbability;
			}
		}

		return L * Maths::InvPi;
	}

	//----------------------------------------------------------------------------------------------
	// Point-lit radiance computation
	//----------------------------------------------------------------------------------------------
	Spectrum LiPointLit(Vector3 &p_position, Vector3 &p_normal)
	{
		VisibilityQuery occlusionQuery(m_pScene);
		Spectrum Le(0), f;
		Vector3 wIn;
		int samplesUsed = 1;
		float indirectScale = 1.2f, // 0.01f,
			minDist = 5.0f,
			cosTheta, cosAlpha;

		for (auto virtualPointSource : m_virtualPointSourceList)
		{
			wIn = virtualPointSource.Position - p_position;
			const float d2 = wIn.LengthSquared();
			wIn.Normalize();

			if ((cosTheta = virtualPointSource.Normal.Dot(-wIn)) > 0.0f && 
				(cosAlpha = p_normal.Dot(wIn)) > 0.0f)
			{
				occlusionQuery.SetSegment(p_position, m_fReflectEpsilon, virtualPointSource.Position, m_fReflectEpsilon);
				
				if (!occlusionQuery.IsOccluded())
				{
					const float distScale = Spline::SmoothStep(0.8f * minDist, 1.2f * minDist, d2);

					float G = (cosTheta * cosAlpha) / d2;
					// G = Maths::Min(G, m_fMaxGTerm);

					Le += virtualPointSource.Contribution *
							distScale *
							Maths::InvPiTwo * G;

					samplesUsed++;
				}
			}
		}

		return (Le * indirectScale) / (float)m_virtualPointSourceList.size();
	}

	/*
	 * Trace virtual point sources for point lit radiance
	 */
	void TraceVirtualPointSources(std::vector<VirtualPointSource> &p_virtualPointSourceList, int p_nMaxVPSources, int p_nMaxVPBounces)
	{
		// Get number of lights in scene
		const int lightCount = m_pScene->LightList.Size();
		
		// If scene holds no lights, then exit
		if (lightCount == 0)
			return;

		Ray lightRay;
		IMaterial *pMaterial;
		Intersection intersection;
		VirtualPointSource pointSource;
		BxDF::Type bxdfType = 
			BxDF::All_Combined;

		Spectrum contribution, 
			alpha, f;

		Vector3 normal, 
			wOut, wIn;

		Vector2 positionSample, 
			directionSample,
			sample;

		int intersections, index = 0;

		float continueProbability, pdf, 
			lightPdf = 1.f / lightCount;

		int lightIndex = 0;
		int secondary = 0;

		// Trace either a maximum number of paths or virtual point lights
		while (p_virtualPointSourceList.size() <= p_nMaxVPSources)
		{
			// Get samples for initial position and direction
			positionSample = m_positionSampler.Get2DSample();
			directionSample = m_directionSampler.Get2DSample();
		
			// Get initial radiance, position and direction
			alpha = m_pScene->LightList[lightIndex]->SampleRadiance(
				m_pScene, positionSample.U, positionSample.V,
				directionSample.U, directionSample.V, lightRay, pdf);

			// If pdf or radiance are zero, choose a new path
			if (pdf == 0.0f || alpha.IsBlack())
				continue;

			// Scale radiance by pdf
			alpha /= pdf * lightPdf;

			// Start tracing virtual point light path
			for (intersections = 1; !alpha.IsBlack() && m_pScene->Intersects(lightRay, intersection); ++intersections)
			{
				// No valid intersection
				if (intersection.HasMaterial() == false 
					|| intersection.Surface.Distance <= m_fReflectEpsilon 
					|| intersection.Surface.Distance == Maths::Maximum
					|| lightRay.Direction.Dot(intersection.Surface.ShadingBasisWS.W) >= 0)
					break;

				// Omega out
				wOut = -lightRay.Direction;
				
				// BSDF
				pMaterial = intersection.GetMaterial();

				// Create vpl at ray intersection point
				pointSource.Contribution = alpha * pMaterial->Rho(wOut, intersection.Surface) * Maths::InvPi;
				pointSource.Position = intersection.Surface.PointWS;
				pointSource.Normal = intersection.Surface.ShadingBasisWS.W;
				
				p_virtualPointSourceList.push_back(pointSource);

				// Sample contribution and new ray
				sample = m_directionSampler.Get2DSample();

				f = pMaterial->SampleF(intersection.Surface, wOut, wIn, sample.U, sample.V, &pdf, bxdfType);

				// If reflectivity or pdf are zero, end path
				if (f.IsBlack() || pdf == 0.0f)
					break;

				contribution = f * wIn.AbsDot(intersection.Surface.ShadingBasisWS.W) / pdf;
				continueProbability = Maths::Min(1.f, contribution.Luminance());

				if (m_continueSampler.Get1DSample() > continueProbability || intersections == p_nMaxVPBounces)
					break;
				
				alpha *= contribution / continueProbability;
				lightRay.Set(intersection.Surface.PointWS + m_fReflectEpsilon * wIn, wIn, m_fReflectEpsilon, Maths::Maximum);
			}

			lightIndex = (lightIndex + 1) % lightCount;
		}

		// Just in case we traced more than is required
		if (p_virtualPointSourceList.size() > p_nMaxVPSources)
			p_virtualPointSourceList.erase(p_virtualPointSourceList.begin() + p_nMaxVPSources, p_virtualPointSourceList.end());	

		// Divide contributions
		float sourceCount = (float)p_virtualPointSourceList.size();
		for (auto virtualPointSource : p_virtualPointSourceList)
			virtualPointSource.Contribution /= sourceCount;
	}
};
