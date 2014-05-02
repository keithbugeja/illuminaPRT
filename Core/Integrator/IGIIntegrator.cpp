//----------------------------------------------------------------------------------------------
//	Filename:	IGIIntegrator.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iostream>
#include "Integrator/IGIIntegrator.h"

#include "Geometry/Ray.h"
#include "Geometry/Spline.h"
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
	, m_nMaxRayDepth(p_nMaxRayDepth)
	, m_nShadowSampleCount(p_nShadowSampleCount)
	, m_nIndirectSampleCount(p_nIndirectSampleCount)
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
	, m_nMaxRayDepth(p_nMaxRayDepth)
	, m_nShadowSampleCount(p_nShadowSampleCount)
	, m_nIndirectSampleCount(p_nIndirectSampleCount)
	, m_fReflectEpsilon(p_fReflectEpsilon)
	, m_fGTermMax(p_fGTermMax)
{ }
//----------------------------------------------------------------------------------------------
bool IGIIntegrator::Initialise(Scene *p_pScene, ICamera *p_pCamera)
{
	for (int nPointSets = 0; nPointSets < m_nTileArea; nPointSets++)
	{
		m_directionSamplerList.push_back(new LowDiscrepancySampler());
		m_positionSamplerList.push_back(new LowDiscrepancySampler());
		m_rouletteSamplerList.push_back(new LowDiscrepancySampler());
		m_bsdfSamplerList.push_back(new LowDiscrepancySampler());
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool IGIIntegrator::Shutdown(void)
{
	for (int nPointSets = 0; nPointSets < m_nTileArea; nPointSets++)
	{
		delete m_directionSamplerList[nPointSets];
		delete m_positionSamplerList[nPointSets];
		delete m_rouletteSamplerList[nPointSets];
	}

	m_directionSamplerList.clear();
	m_positionSamplerList.clear();
	m_rouletteSamplerList.clear();

	return true;
}
//----------------------------------------------------------------------------------------------
void IGIIntegrator::TraceVirtualPointLights(Scene *p_pScene, int p_nMaxPaths, int p_nMaxPointLights, int p_nMaxBounces, int p_nVirtualPointLightSetId, std::vector<VirtualPointLight> &p_virtualPointLightList)
{
	// Get number of lights in scene
	const int lightCount = p_pScene->LightList.Size();
		
	// If scene holds no lights, then exit
	if (lightCount == 0)
		return;

	Ray lightRay;
	IMaterial *pMaterial;
	Intersection intersection;
	VirtualPointLight pointLight;
	BxDF::Type bxdfType = 
		BxDF::All_Combined;

	Spectrum contribution, 
		alpha, f;

	Vector3 normal, 
		wOut, wIn,
		wOutLocal, wInLocal;

	Vector2 positionSample, 
			directionSample;

	float bsdfSample;
	
	int intersections, 
		lightIndex = 0,
		index = 0;

	float continueProbability, pdf, 
		lightPdf = 1.f / lightCount;

	// Trace either a maximum number of paths or virtual point lights
	//for (int lightIndex = 0, nPathIndex = p_nMaxPaths; nPathIndex > 0 && p_virtualPointLightList.size() < p_nMaxPointLights; --nPathIndex)
	while (p_virtualPointLightList.size() <= p_nMaxPointLights)
	{
		// Get samples for initial position and direction
		positionSample = m_positionSamplerList[p_nVirtualPointLightSetId]->Get2DSample();
		directionSample = m_directionSamplerList[p_nVirtualPointLightSetId]->Get2DSample();

		// Get initial radiance, position and direction
		alpha = p_pScene->LightList[lightIndex]->SampleRadiance(
			p_pScene, positionSample.U, positionSample.V, 
			directionSample.U, directionSample.V, lightRay, pdf);

		// If pdf or radiance are zero, choose a new path
		if (pdf == 0.0f || alpha.IsBlack())
			continue;

		// Scale radiance by pdf
		alpha /= pdf * lightPdf;

		// Start tracing virtual point light path
		// for (intersections = 1; p_pScene->Intersects(lightRay, intersection); ++intersections)
		for (intersections = 1; !alpha.IsBlack() && p_pScene->Intersects(lightRay, intersection); ++intersections)
		{
			// No valid intersection
			if (intersection.Surface.Distance <= m_fReflectEpsilon || intersection.Surface.Distance == Maths::Maximum)
				break;

			if (!intersection.HasMaterial())
				break;

			if (lightRay.Direction.Dot(intersection.Surface.ShadingBasisWS.W) >= 0)
				break;

			// Omega out
			wOut = -lightRay.Direction;

			// BSDF
			pMaterial = intersection.GetMaterial();

			// Create VPL at ray intersection point
			pointLight.Contribution = alpha * pMaterial->Rho(wOut, intersection.Surface) * Maths::InvPi;
			pointLight.Context = intersection;
			pointLight.Direction = wOut;

			// Push point light on list
			p_virtualPointLightList.push_back(pointLight);

			// Sample contribution and new ray
			BSDF::SurfaceToWorld(intersection.WorldTransform, intersection.Surface, wOut, wOutLocal);

			bsdfSample = m_bsdfSamplerList[p_nVirtualPointLightSetId]->Get1DSample();
			directionSample = m_directionSamplerList[p_nVirtualPointLightSetId]->Get2DSample();
			f = pMaterial->SampleF(intersection.Surface, wOutLocal, wInLocal, directionSample.U, directionSample.V, bsdfSample, &pdf, bxdfType);

			// If reflectivity or pdf are zero, end path
			if (f.IsBlack() || pdf == 0.0f) break;

			BSDF::WorldToSurface(intersection.WorldTransform, intersection.Surface, wInLocal, wIn);

			// Compute contribution of path
			contribution = f * wIn.AbsDot(intersection.Surface.ShadingBasisWS.W) / pdf;

			// Possibly terminate virtual light path with Russian roulette
			continueProbability = Maths::Min(1.f, contribution.Luminance());
			if (m_rouletteSamplerList[p_nVirtualPointLightSetId]->Get1DSample() > continueProbability || intersections >= p_nMaxBounces)
				break;
				
			// Modify contribution accordingly
			alpha *= contribution / continueProbability;

			// Set new ray position and direction
			lightRay.Set(intersection.Surface.PointWS + m_fReflectEpsilon * wIn, wIn, m_fReflectEpsilon, Maths::Maximum);
		}

		// Increment light index, and reset if we traversed all scene lights
		lightIndex = (lightIndex + 1) % lightCount;
	}

	// Just in case we traced more than is required
	if (p_virtualPointLightList.size() > p_nMaxPointLights)
		p_virtualPointLightList.erase(p_virtualPointLightList.begin() + p_nMaxPointLights, p_virtualPointLightList.end());

	// Divide contributions
	float sourceCount = 1.0f / (float)p_virtualPointLightList.size();
	for (auto virtualPointLight : p_virtualPointLightList)
		virtualPointLight.Contribution *= sourceCount;
}
//----------------------------------------------------------------------------------------------
bool IGIIntegrator::Prepare(Scene *p_pScene)
{
	// Just in case we're compiling for Windows and 
	// min and max are defined as macros
	#define __igi_max max
	#undef max

	int maxValue = std::numeric_limits<int>::max();

	// Restore max macro
	#define max __igi_max

	VirtualPointLightSet.clear();

	for (int pointLightSet = 0; pointLightSet < m_nTileArea; ++pointLightSet)
	{
		VirtualPointLightSet.push_back(std::vector<VirtualPointLight>());

		m_directionSamplerList[pointLightSet]->Reset((unsigned int)(maxValue * p_pScene->GetSampler()->Get1DSample()));
		m_positionSamplerList[pointLightSet]->Reset((unsigned int)(maxValue * p_pScene->GetSampler()->Get1DSample()));
		m_rouletteSamplerList[pointLightSet]->Reset((unsigned int)(maxValue * p_pScene->GetSampler()->Get1DSample()));

		TraceVirtualPointLights(p_pScene, m_nMaxPath, m_nMaxVPL, m_nMaxRayDepth, pointLightSet, VirtualPointLightSet.back());
	}

	return true;
}
//----------------------------------------------------------------------------------------------
Spectrum IGIIntegrator::Radiance(IntegratorContext *p_pContext, Scene *p_pScene, Intersection &p_intersection, RadianceContext *p_pRadianceContext)
{
	float samplesUsed;

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

	// Visibility query
	VisibilityQuery pointLightQuery(p_pScene);

	int surfX = (int)p_pContext->SurfacePosition.X,
		surfY = (int)p_pContext->SurfacePosition.Y,
		pointLightSetIndex = 
			Maths::FAbs(surfX % m_nTileWidth + (surfY % m_nTileWidth) * m_nTileWidth);

	// Initialise point light set
	std::vector<VirtualPointLight> &pointLightSet = 
		VirtualPointLightSet[pointLightSetIndex];

	std::vector<VirtualPointLight>::iterator pointLightIterator;
	
	// ---> Added 2/5/13
	// Initialise partition
	int partitionSize = pointLightSet.size() / p_pContext->SampleCount,
		partitionStart = p_pContext->SampleIndex * partitionSize,
		partitionEnd = partitionStart + partitionSize;
	// ---> Added 2/5/13

	// ---> Added 30/12/13
	float indirectScale = 1.f,
		minDist = 0.5f,
		cosTheta;
	// ---> Added 30/12/13

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
				
				// Initialise indirect
				p_pRadianceContext->Indirect = 0.f;
				
				
				for (samplesUsed = 1, pointLightIterator = pointLightSet.begin() + partitionStart; 
					 pointLightIterator != pointLightSet.begin() + partitionEnd/*pointLightSet.end()*/; ++pointLightIterator)
				{
					VirtualPointLight &pointLight = *pointLightIterator;

					/*
					wIn = pointLight.Context.Surface.PointWS - p_intersection.Surface.PointWS;
					const float d2 = wIn.LengthSquared();
					wIn.Normalize();

					if ((cosTheta = pointLight.Context.Surface.ShadingBasisWS.W.Dot(-wIn)) > 0.0f)
					{
						pointLightQuery.SetSegment(p_intersection.Surface.PointWS, m_fReflectEpsilon, pointLight.Context.Surface.PointWS, m_fReflectEpsilon);
				
						if (!pointLightQuery.IsOccluded())
						{
							const float distScale = Spline::SmoothStep(0.8f * minDist, 1.2f * minDist, d2);

							float G = cosTheta * p_intersection.Surface.ShadingBasisWS.W.Dot(wIn) / d2;
							G = Maths::Min(G, m_fGTermMax);

							p_pRadianceContext->Indirect += pointLight.Contribution *
								distScale *
								Maths::InvPiTwo * G;

							samplesUsed++;
						}
					}
					/* */

					/* */
					wIn = Vector3::Normalize(pointLight.Context.Surface.PointWS - p_intersection.Surface.PointWS);
					
					if (wIn.Dot(pointLight.Context.Surface.ShadingBasisWS.W) < 0.f)
					{
						// Sample reflectivity (we sample this first as it's a faster 
						// early out than an intersection test)
						Spectrum f = IIntegrator::F(p_pScene, p_intersection, wOut, wIn);
			
						if (f.IsBlack()) 
							continue;

						// Test immediately if the point light is occluded
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
							float d2 = Vector3::DistanceSquared(p_intersection.Surface.PointWS, pointLight.Context.Surface.PointWS);
							const float distScale = Spline::SmoothStep(0.8f * minDist, 1.2f * minDist, d2);
							d2 = 1.0f / d2;

							// float d2 = 1.f / Vector3::DistanceSquared(p_intersection.Surface.PointWS, 
							//	pointLight.Context.Surface.PointWS);
						#endif

						const float G = Maths::Min(
							Vector3::Dot(pointLight.Context.Surface.ShadingBasisWS.W, -wIn) * 
							Vector3::AbsDot(p_intersection.Surface.ShadingBasisWS.W, wIn) * d2,
							m_fGTermMax);
						
						p_pRadianceContext->Indirect += f * pointLight.Contribution * G * distScale;
						samplesUsed++;
					}
					/* */
				}

				p_pRadianceContext->Indirect = (p_pRadianceContext->Indirect * indirectScale) / (int)pointLightSet.size(); //(float)partitionSize;

				// p_pRadianceContext->Indirect = p_pRadianceContext->Indirect / (int)pointLightSet.size(); //samplesUsed;
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
	
	return /*p_pRadianceContext->Direct +*/ p_pRadianceContext->Indirect;
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
}
//----------------------------------------------------------------------------------------------