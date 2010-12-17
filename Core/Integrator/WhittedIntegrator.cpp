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
#include "Spectrum/Spectrum.h"
#include "Staging/Visibility.h"
#include "Staging/Scene.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
WhittedIntegrator::WhittedIntegrator(int p_nMaxRayDepth, int p_nShadowSampleCount)
	: m_nMaxRayDepth(p_nMaxRayDepth)
	, m_nShadowSampleCount(p_nShadowSampleCount)
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
	Spectrum light, 
		result;

	Vector3 wOut, 
		reflectionVector;

	Ray ray(p_ray);

	for (int i = 0; i < m_nMaxRayDepth; i++)
	{
		bool bHit = p_pScene->Intersects(ray, p_intersection);
				
		if (bHit)
		{
			VisibilityQuery visibilityQuery(p_pScene);

			Vector3 normal(p_intersection.Surface.BasisWS.U);
			normal.Y = -normal.Y;

			for (int j = 0; j < p_pScene->LightList.Size(); ++j)
			{
				light = IIntegrator::EstimateDirectLighting(p_pScene, p_pScene->LightList[j], p_intersection.Surface.PointWS, normal, wOut);
			}

			// Need method to generate a point on the hemisphere

			Vector3::Reflect(ray.Direction, normal, reflectionVector);
			ray.Direction = reflectionVector;
			ray.Origin = p_intersection.Surface.PointWS + reflectionVector * 0.0001f;

			result += light;
		}
	}

	return result;
}
//----------------------------------------------------------------------------------------------