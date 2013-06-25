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
ICIntegrator::ICIntegrator(const std::string &p_strName, int p_nRayDepth, int p_nDivisions, int p_nShadowSampleCount, float p_fReflectEpsilon)
	: IIntegrator(p_strName)
	, m_nRayDepth(p_nRayDepth)
	, m_nDivisions(p_nDivisions)
	, m_nShadowSampleCount(p_nShadowSampleCount)
	, m_fReflectEpsilon(p_fReflectEpsilon)
{ }
//----------------------------------------------------------------------------------------------
ICIntegrator::ICIntegrator(int p_nRayDepth, int p_nDivisions, int p_nShadowSampleCount, float p_fReflectEpsilon)
	: IIntegrator()
	, m_nRayDepth(p_nRayDepth)
	, m_nDivisions(p_nDivisions)
	, m_nShadowSampleCount(p_nShadowSampleCount)
	, m_fReflectEpsilon(p_fReflectEpsilon)
{ }
//----------------------------------------------------------------------------------------------
bool ICIntegrator::Initialise(Scene *p_pScene, ICamera *p_pCamera)
{
	BOOST_ASSERT(p_pScene != nullptr && p_pScene->GetSpace() != nullptr);

	// m_irradianceCache.RootNode.Bounds.ComputeFromVolume(*(p_pScene->GetSpace()->GetBoundingVolume()));
	
	ISpace *pSpace = p_pScene->GetSpace(); pSpace->Build();

	Vector3 pointList[2];
	pointList[0] = pSpace->GetBoundingVolume()->GetMinExtent();
	pointList[1] = pSpace->GetBoundingVolume()->GetMaxExtent();

	pointList[0] *= (1.0f + Maths::Epsilon);
	pointList[1] *= (1.0f + Maths::Epsilon);

	m_irradianceCache.RootNode.Bounds.ComputeFromPoints((Vector3*)&pointList, 2);

	return true;
}
//----------------------------------------------------------------------------------------------
bool ICIntegrator::Shutdown(void)
{
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
					wOut, p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);

				// Set albedo
				p_pRadianceContext->Albedo = pMaterial->Rho(wOut, p_intersection.Surface);
				
				// Set indirect 
				p_pRadianceContext->Indirect = GetIrradiance(p_intersection, p_pScene) * p_pRadianceContext->Albedo * Maths::InvPi;
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
	
	// return p_pRadianceContext->Indirect * p_pRadianceContext->Albedo;
	return p_pRadianceContext->Direct + p_pRadianceContext->Indirect;

	// return p_pRadianceContext->Indirect;

	// Spectrum E = GetIrradiance(p_intersection, p_pScene);
	// return E;
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
	
	if (nearbyRecordList.size() > 0)
	{
		Spectrum num = 0.f;
		float den = 0.f;

		for (auto pair : nearbyRecordList)
		{
			num += pair.second->Irradiance * pair.first;
			den += pair.first;
		}

		num /= den;
		return num;
	}
	else
	{
		IrradianceCacheRecord *r = new IrradianceCacheRecord();
		ComputeRecord(p_intersection, p_pScene, *r);
		m_irradianceCache.Insert(&(m_irradianceCache.RootNode), r);
	
		return r->Irradiance;
	}
}
//----------------------------------------------------------------------------------------------
void ICIntegrator::ComputeRecord(const Intersection &p_intersection, Scene *p_pScene, IrradianceCacheRecord &p_record)
{
	Vector2 sample2D;

	Intersection isect;
	Vector3 wOutR;
	Ray r;

	Spectrum e = 0;
	float tdist = 0;
	int tnum = 0;

	for (int rayIndex = 0; rayIndex < m_nDivisions; ++rayIndex)
	{
		// Get samples for initial position and direction
		sample2D.X = QuasiRandomSequence::VanDerCorput(rayIndex);
		sample2D.Y = QuasiRandomSequence::Sobol2(rayIndex);

		BSDF::SurfaceToWorld(p_intersection.WorldTransform, p_intersection.Surface, Montecarlo::UniformSampleSphere(sample2D.U, sample2D.V), wOutR); 

		r.Set(p_intersection.Surface.PointWS + wOutR * m_fReflectEpsilon, wOutR, m_fReflectEpsilon, Maths::Maximum);

		p_pScene->Intersects(r, isect);
		
		e += SampleAllLights(p_pScene, isect, 
				isect.Surface.PointWS, isect.Surface.ShadingBasisWS.W, wOutR, 
				p_pScene->GetSampler(), isect.GetLight(), m_nShadowSampleCount);

		// tdist += isect.Surface.Distance; tnum++;
		tdist += 1.f / isect.Surface.Distance; tnum++;
	}

	p_record.Point = p_intersection.Surface.PointWS;
	p_record.Normal = p_intersection.Surface.ShadingBasisWS.W;
	p_record.Irradiance = e / tnum;
	p_record.RiClamp = p_record.Ri = tnum / tdist; //tdist / tnum;
	p_record.Rmin = 0;
	p_record.Rmax = Maths::Maximum;
}
//----------------------------------------------------------------------------------------------