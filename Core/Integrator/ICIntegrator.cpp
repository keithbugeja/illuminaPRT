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
ICIntegrator::ICIntegrator(const std::string &p_strName, int p_nShadowSampleCount, float p_fReflectEpsilon)
	: IIntegrator(p_strName) 
{ }
//----------------------------------------------------------------------------------------------
ICIntegrator::ICIntegrator(int p_nShadowSampleCount, float p_fReflectEpsilon)
	: IIntegrator()
{ }
//----------------------------------------------------------------------------------------------
bool ICIntegrator::Initialise(Scene *p_pScene, ICamera *p_pCamera)
{
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
	return 0.f;
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