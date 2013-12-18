//----------------------------------------------------------------------------------------------
//	Filename:	PointShader.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
// Todo: Generalise structures used... there is way too much repetition (copy-pasting and 
//	     tweaking) of functionality available elsewhere in the core.
//----------------------------------------------------------------------------------------------
#pragma once

#include <Maths/Montecarlo.h>
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
class PointShader
{
protected:
	int m_nAzimuthStrata,
		m_nAltitudeStrata;

	std::vector<VirtualPointSource> m_virtualPointSourceList;
	int m_nMaxVPSources,
		m_nMaxVPPaths;
	float m_fMaxGTerm;

	int	m_nShadowRayCount,
		m_nRayDepth;

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
		m_nShadowRayCount = p_nShadowRayCount;
	}

	//----------------------------------------------------------------------------------------------
	// Shader type specific arguments
	//----------------------------------------------------------------------------------------------
	void SetHemisphereDivisions(int p_nAzimuthStrata, int p_nAltitudeStrata) 
	{
		m_nAltitudeStrata = p_nAltitudeStrata;
		m_nAzimuthStrata = p_nAzimuthStrata;		
	}

	void SetVirtualPointSources(int p_nMaxVirtualPointSources, int p_nMaxVirtualPointPaths)
	{
		m_nMaxVPSources = p_nMaxVirtualPointSources;
		m_nMaxVPPaths = p_nMaxVirtualPointPaths;
	}

	void SetGeometryTerm(float p_fMaxGeometryTerm)
	{
		m_fMaxGTerm = p_fMaxGeometryTerm;
	}

	//----------------------------------------------------------------------------------------------
	// Shader preparation
	//----------------------------------------------------------------------------------------------
	void Prepare(enum ShadingType p_eShadingType)
	{
		if (p_eShadingType == PointLit)
			TraceVirtualPointSources(m_virtualPointSourceList, m_nMaxVPSources, m_nMaxVPPaths);
	}

public:
	//----------------------------------------------------------------------------------------------
	// Shade points
	//----------------------------------------------------------------------------------------------
	void Shade(std::vector<T*> &p_pointList, ShadingType p_eShadingType)
	{
		double total = Platform::GetTime();

		if (p_eShadingType == PointLit)
		{
			//std::cout << "Shading point cloud using LiPointLit..." << std::endl;
			for (auto point : p_pointList)
			{
				if (point->Invalid)
				{
					//double timestart = Platform::GetTime();
					point->Irradiance = LiPointLit(point->Position, point->Normal);
					point->Invalid = false;
					//std::cout << "Shaded in [" << Platform::ToSeconds(Platform::GetTime() - timestart) << "]" << std::endl;
				}
			}
		}
		else
		{
			//std::cout << "Shading point cloud using LiPathTraced..." << std::endl;

			for (auto point : p_pointList)
			{
				if (point->Invalid)
				{
					//double timestart = Platform::GetTime();
					point->Irradiance = LiPathTraced(point->Position, point->Normal);
					point->Invalid = false;
					// std::cout << "Shaded in [" << Platform::ToSeconds(Platform::GetTime() - timestart) << "]" << std::endl;
				}
			}
		}
		//std::cout << "Shaded [" << p_pointList.size() << "] in [" << Platform::ToSeconds(Platform::GetTime() - total) << "]" << std::endl;
	}

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
				m_pScene->GetSampler(), isect.GetLight(), m_nShadowRayCount);

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
				float continueProbability = Maths::Min(0.5f, 0.33f * (pathThroughput[0] + pathThroughput[1] + pathThroughput[2]));

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
		float indirectScale = 1.f, // 0.01f,
			minDist = 0.5f,
			cosTheta;

		for (auto virtualPointSource : m_virtualPointSourceList)
		{
			wIn = virtualPointSource.Position - p_position;
			const float d2 = wIn.LengthSquared();
			wIn.Normalize();

			if ((cosTheta = virtualPointSource.Normal.Dot(-wIn)) > 0.0f)
			{
				occlusionQuery.SetSegment(p_position, m_fReflectEpsilon, virtualPointSource.Position, m_fReflectEpsilon);
				
				if (!occlusionQuery.IsOccluded())
				{
					const float distScale = SmoothStep(0.8f * minDist, 1.2f * minDist, d2);

					float G = cosTheta * p_normal.Dot(wIn) / d2;
					G = Maths::Min(G, m_fMaxGTerm);

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
	 * Sigmoid function for smoothing values in range min-max
	 */
	static float SmoothStep(float min, float max, float value)
	{
		float v = (value - min) / (max - min);
		if (v < 0.0f) v = 0.0f;
		if (v > 1.0f) v = 1.0f;
		return v * v * (-2.f * v  + 3.f);
	}

	/*
	 * Trace virtual point sources for point lit radiance
	 */
	void TraceVirtualPointSources(std::vector<VirtualPointSource> &p_virtualPointSourceList, int p_nMaxVPSources, int p_nMaxVPPaths)
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

		RandomSampler sequenceSampler;

		LowDiscrepancySampler positionSampler,
			directionSampler, continueSampler;

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
			positionSample = positionSampler.Get2DSample();
			directionSample = directionSampler.Get2DSample();
		
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
				if (intersection.Surface.Distance <= m_fReflectEpsilon || intersection.Surface.Distance == Maths::Maximum)
					break;

				if (!intersection.HasMaterial())
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
				sample = directionSampler.Get2DSample();

				f = pMaterial->SampleF(intersection.Surface, wOut, wIn, sample.U, sample.V, &pdf, bxdfType);

				// If reflectivity or pdf are zero, end path
				if (f.IsBlack() || pdf == 0.0f)
					break;

				contribution = f * wIn.AbsDot(intersection.Surface.ShadingBasisWS.W) / pdf;
				continueProbability = Maths::Min(1.f, contribution.Luminance());

				if (continueSampler.Get1DSample() > continueProbability || intersections == 10)
					break;
				
				alpha *= contribution / continueProbability;
				lightRay.Set(intersection.Surface.PointWS + m_fReflectEpsilon * wIn, wIn, m_fReflectEpsilon, Maths::Maximum);

				if (intersections > 1)
					secondary++;
			}

			lightIndex = (lightIndex + 1) % lightCount;

			/*
			// Increment light index, and reset if we traversed all scene lights
			if (++lightIndex == m_pScene->LightList.Size())
				lightIndex = 0;
			*/
		}

		// Just in case we traced more than is required
		if (p_virtualPointSourceList.size() > p_nMaxVPSources)
			p_virtualPointSourceList.erase(p_virtualPointSourceList.begin() + p_nMaxVPSources, p_virtualPointSourceList.end());	

		// Divide contributions
		float sourceCount = (float)p_virtualPointSourceList.size();
		for (auto virtualPointSource : p_virtualPointSourceList)
			virtualPointSource.Contribution /= sourceCount;

		// std::cout << "Secondary photons : " << secondary << std::endl;
	}
};
