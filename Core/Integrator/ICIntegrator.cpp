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

#include "Maths/Montecarlo.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
ICIntegrator::ICIntegrator(const std::string &p_strName, int p_nRayDepth, int p_nDivisions, int p_nShadowSampleCount, float p_fReflectEpsilon)
	: IIntegrator(p_strName) 
{ }
//----------------------------------------------------------------------------------------------
ICIntegrator::ICIntegrator(int p_nRayDepth, int p_nDivisions, int p_nShadowSampleCount, float p_fReflectEpsilon)
	: IIntegrator()
{ }
//----------------------------------------------------------------------------------------------
bool ICIntegrator::Initialise(Scene *p_pScene, ICamera *p_pCamera)
{
	BOOST_ASSERT(p_pScene != nullptr && p_pScene->GetSpace() != nullptr);

	m_irradianceCache.RootNode.Bounds.ComputeFromVolume(*(p_pScene->GetSpace()->GetBoundingVolume()));
	
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
	return GetIrradiance(p_intersection, p_pScene);
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
		p_pScene->GetSampler()->Get2DSamples(&sample2D, 2);

		BSDF::SurfaceToWorld(p_intersection.WorldTransform, p_intersection.Surface, Montecarlo::UniformSampleSphere(sample2D.U, sample2D.V), wOutR); 

		r.Set(p_intersection.Surface.PointWS + wOutR * m_fReflectEpsilon, wOutR, m_fReflectEpsilon, Maths::Maximum);

		p_pScene->Intersects(r, isect);
		
		e += SampleAllLights(p_pScene, p_intersection, 
				p_intersection.Surface.PointWS, p_intersection.Surface.ShadingBasisWS.W, wOutR, 
				p_pScene->GetSampler(), p_intersection.GetLight(), m_nShadowSampleCount);

		tdist += isect.Surface.Distance; tnum++;
	}

	p_record.Point = p_intersection.Surface.PointWS;
	p_record.Normal = p_intersection.Surface.ShadingBasisWS.W;
	p_record.Irradiance = e / tnum;
	p_record.Ri = tdist / tnum;
	p_record.RiClamp = p_record.Ri;
}
//----------------------------------------------------------------------------------------------