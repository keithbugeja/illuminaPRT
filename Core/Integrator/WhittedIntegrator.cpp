//----------------------------------------------------------------------------------------------
//	Filename:	WhittedIntegrator.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iostream>
#include "Integrator/WhittedIntegrator.h"

#include "Geometry/Ray.h"
#include "Geometry/Intersection.h"
#include "Sampler/JitterSampler.h"
#include "Material/Material.h"
#include "Spectrum/Spectrum.h"
#include "Staging/Visibility.h"
#include "Staging/Scene.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
WhittedIntegrator::WhittedIntegrator(int p_nMaxRayDepth, int p_nShadowSampleCount, bool p_bShowNormals)
	: m_nMaxRayDepth(p_nMaxRayDepth)
	, m_nShadowSampleCount(p_nShadowSampleCount)
	, m_bShowNormals(p_bShowNormals)
{ }
//----------------------------------------------------------------------------------------------
bool WhittedIntegrator::Initialise(Scene *p_pScene, ICamera *p_pCamera)
{
	std::cout << "Whitted Integrator :: Initialise()" << std::endl;
	return true;
}
//----------------------------------------------------------------------------------------------
bool WhittedIntegrator::Shutdown(void)
{
	std::cout << "Whitted Integrator :: Shutdown()" << std::endl;
	return true;
}
//----------------------------------------------------------------------------------------------
Spectrum WhittedIntegrator::Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection)
{
	/*
	Color TracePath(Ray r,depth) {
		if(depth == MaxDepth)
			return Black;  // bounced enough times
 
		r.FindNearestObject();
		if(r.hitSomething == false)
			return Black;  // nothing was hit
 
		Material m = r.thingHit->material;
		Color emittance = m.emittance;
 
		// pick a random direction from here and keep going
		Ray newRay;
		newRay.origin = r.pointWhereObjWasHit;
		newRay.direction = RandomUnitVectorInHemisphereOf(r.normalWhereObjWasHit);
		float cos_omega = DotProduct(newRay.direction, r.normalWhereObjWasHit);
   
		Color BDRF = m.reflectance*cos_omega;
		Color reflected = TracePath(newRay,depth+1);
 
		return emittance + ( BDRF * cos_omega * reflected );
	}
	*/

	const int samples = 5;
	m_nMaxRayDepth = 1;

	Vector3 wOut, wIn,
		reflectionVector;

	Vector2 sample;
	
	Spectrum result = 0,
		diffuse;
	
	Ray ray;

	//m_bShowNormals = true;

	VisibilityQuery visibilityQuery(p_pScene);

	for (int s = 0; s < samples; s++)
	{
		ray = p_ray;

		for (int i = 0; i < m_nMaxRayDepth; i++)
		{
			if(p_pScene->Intersects(ray, p_intersection))
			{
				if (m_bShowNormals)
				{
					Vector3 normal = p_intersection.Surface.GeometryBasisWS.W;
				
					result[0] = (normal[0] + 1.0f) / 2;
					result[1] = (normal[1] + 1.0f) / 2;
					result[2] = (normal[2] + 1.0f) / 2;

					break;
				}

				// We encoutered a light
				if (p_intersection.IsEmissive())
				{
					p_pScene->GetSampler()->Get2DSamples(&sample, 1);

					result += p_intersection.GetLight()->Radiance(
						p_intersection.Surface.PointWS, sample.U, sample.V, wOut, visibilityQuery);
				}

				// Sample all scene lights
				result += SampleAllLights(p_pScene, p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.W, p_pScene->GetSampler(), p_intersection.GetLight(), 1);

				//Need method to generate a point on the hemisphere
				//p_pScene->GetSampler()->Get2DSamples(&sample, 1);
				//ray.Direction = p_intersection.Surface.GeometryBasisWS.Project(OrthonormalBasis::FromSpherical(sample.U * Maths::PiHalf * 0.1f, sample.V * Maths::PiTwo));
				//Vector3::Reflect(ray.Direction, p_intersection.Surface.GeometryBasisWS.W, ray.Direction);
				//ray.Origin = p_intersection.Surface.PointWS;
				//ray.Min = 0.001f;
				//ray.Max = Maths::Maximum;

				/**/

				////
				////// Get BSDF for current point of intersection
				////BSDF bsdf = intersection.Primitive.GetBSDF(intersection.SurfaceGeometry, intersection.SurfaceGeometry);

				////// Add emissive component
				////radiance += intersection.Le(wOut);

				////// Add direct lighting contribution
				////radiance += SampleAllLights(intersection.SurfaceGeometry.Point, intersection.SurfaceGeometry.Normal, wOut, bsdf, m_shadowSampleCount);

				//Spectrum light;
				//
				//for (int lightIdx = 0; lightIdx < p_pScene->LightList.Size(); ++lightIdx)
				//{
				//	light = IIntegrator::EstimateDirectLighting(p_pScene, p_pScene->LightList[lightIdx], 
				//		p_intersection.Surface.PointWS, p_intersection.Surface.GeometryBasisWS.V, wOut);
				//}

				////p_intersection.GetMaterial()->Diffuse(p_intersection.Surface, p_intersection.Surface.PointWS, wIn, reflectionVector, diffuse);

				////Need method to generate a point on the hemisphere
				//Matrix3x3::Product(p_intersection.Surface.GeometryBasisWS.GetMatrix(), 
				//	OrthonormalBasis::FromSpherical(m_random.NextFloat() * Maths::PiTwo, m_random.NextFloat() * Maths::PiHalf),
				//	reflectionVector);

				////Vector3::Reflect(ray.Direction, p_intersection.Surface.GeometryBasisWS.V, reflectionVector);
				//ray.Direction = reflectionVector;
				//ray.Origin = p_intersection.Surface.PointWS + ray.Direction * 0.0001f;

				//result += /*diffuse * */ (light / (i + 1));
			}
			else
				break;
		}
	}

	return result / samples;
}
//----------------------------------------------------------------------------------------------