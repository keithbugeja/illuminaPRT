//----------------------------------------------------------------------------------------------
//	Filename:	Integrator.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <iostream>
#include "Integrator/Integrator.h"

#include "Geometry/Ray.h"
#include "Geometry/Intersection.h"
#include "Spectrum/Spectrum.h"
#include "Staging/Visibility.h"
#include "Staging/Scene.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
Spectrum IIntegrator::EstimateDirectLighting(Scene *p_pScene, ILight *p_pLight, 
				const Vector3 &p_point, const Vector3 &p_normal, Vector3 &p_wOut)
{ 
	VisibilityQuery visibilityQuery(p_pScene);

	Spectrum Li = p_pLight->Radiance(p_point, p_wOut, visibilityQuery);
				
	if (!Li.IsBlack() /*&& !visibilityQuery.IsOccluded()*/)
	{
		//return Li * Maths::Max(0, Vector3::Dot(-p_wOut, p_normal));
		return Vector3::Dot(p_wOut, p_normal);
	}

	return 0;
}
//----------------------------------------------------------------------------------------------
//Vector3 IIntegrator::SampleHemisphere(const Transformation p_transform, float p_fU, float p_fV)
//{
//	Vector2 spherical(p_fU * Maths::PiTwo, p_fV * Maths::PiHalf);
//	return p_transform.Apply(OrthonormalBasis::FromSpherical(spherical));
//}
//----------------------------------------------------------------------------------------------
