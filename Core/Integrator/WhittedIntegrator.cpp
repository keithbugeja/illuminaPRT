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
	Spectrum result;

	Vector3 wOut, 
		reflectionVector;

	Ray ray;

	const int samples = 1;
	m_nMaxRayDepth = 1;

	for (int s = 0; s < samples; s++)
	{
		ray = p_ray;

		for (int i = 0; i < m_nMaxRayDepth; i++)
		{
			bool bHit = p_pScene->Intersects(ray, p_intersection);
				
			if (bHit)
			{
				Spectrum light;

				result[0] = (p_intersection.Surface.BasisWS.U[0] + 1.0f) * 0.5f;
				result[1] = (p_intersection.Surface.BasisWS.U[1] + 1.0f) * 0.5f;
				result[2] = (p_intersection.Surface.BasisWS.U[2] + 1.0f) * 0.5f;
				
				//Vector3 wOut = Vector3(0,-50,0) - p_intersection.Surface.Point;
				//wOut.Normalize();
				//float d = Vector3::Dot(wOut, p_intersection.Surface.Normal);
				//result[0] = d;
				//result[1] = d;
				//result[2] = d;

				//light = IIntegrator::EstimateDirectLighting(p_pScene, p_pScene->LightList[0], p_intersection.Surface.PointWS, p_intersection.Surface.BasisWS.U, wOut);

				break;
				
				for (int j = 0; j < p_pScene->LightList.Size(); ++j)
				{
					light += IIntegrator::EstimateDirectLighting(p_pScene, p_pScene->LightList[j], p_intersection.Surface.PointWS, p_intersection.Surface.BasisWS.U, wOut);
				}

				if (light.IsBlack())
					break;

				// Need method to generate a point on the hemisphere
				Matrix3x3::Product(p_intersection.Surface.BasisWS.GetMatrix(), 
					OrthonormalBasis::FromSpherical(m_random.NextFloat() * Maths::PiTwo, m_random.NextFloat() * Maths::PiHalf),
					reflectionVector);

				ray.Direction = reflectionVector;
				ray.Origin = p_intersection.Surface.PointWS + reflectionVector * 0.0001f;

				result += light;
			}
			else
			{
				result = 0.f;
				break;
			}
		}
	}

	return result / samples;
}
//----------------------------------------------------------------------------------------------