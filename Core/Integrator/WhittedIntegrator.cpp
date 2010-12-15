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
	bool bHit = p_pScene->Intersects(p_ray, p_intersection);
				
	if (bHit)
	{
		Vector3 wOut;
		VisibilityQuery visibilityQuery(p_pScene);

		for (int j = 0; j < p_pScene->LightList.Size(); ++j)
		{
			Spectrum lightLi = p_pScene->LightList[j]->Radiance(p_intersection.Surface.Point, wOut, visibilityQuery);				

			if (!visibilityQuery.IsOccluded())
			{
				wOut.Normalize();
				result += lightLi * Maths::Max(0, Vector3::Dot(wOut, p_intersection.Surface.Normal));
			}
		}
	}

	return result;
}
//----------------------------------------------------------------------------------------------