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
				
	if (!Li.IsBlack() && !visibilityQuery.IsOccluded())
	{
		p_wOut.Normalize();
		return Li * Maths::Max(0, Vector3::Dot(p_wOut, p_normal));
	}

	return 0;
}
//----------------------------------------------------------------------------------------------
