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
WhittedIntegrator::WhittedIntegrator(int p_nMaxRayDepth, int p_nShadowSampleCount)
	: m_nShadowSampleCount(p_nShadowSampleCount)
	, m_nMaxRayDepth(p_nMaxRayDepth)
{ }
//----------------------------------------------------------------------------------------------
bool WhittedIntegrator::Initialise(Scene *p_pScene, ICamera *p_pCamera)
{
	//std::cout << "Path Tracing Integrator :: Initialise()" << std::endl;
	return true;
}
//----------------------------------------------------------------------------------------------
bool WhittedIntegrator::Shutdown(void)
{
	//std::cout << "Path Tracing Integrator :: Shutdown()" << std::endl;
	return true;
}
//----------------------------------------------------------------------------------------------
Spectrum WhittedIntegrator::Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection)
{
	VisibilityQuery visibilityQuery(p_pScene);

	Spectrum pathThroughput(1.0f), 
		L(0.0f);
	
	bool specularBounce = false;

	BxDF::Type bxdfType;

	Vector3 wIn, wOut; 
	Vector2 sample;

	Ray ray(p_ray); 

	float pdf;
	L=0;
	for (int rayDepth = 0; rayDepth < m_nMaxRayDepth; rayDepth++)
	{
		if(p_pScene->Intersects(ray, p_intersection))
		{
			//L[0] = p_intersection.Surface.ShadingBasisWS.W[0];
			//L[1] = p_intersection.Surface.ShadingBasisWS.W[1];
			//L[2] = p_intersection.Surface.ShadingBasisWS.W[2];
			
			VisibilityQuery query(p_pScene);

			for (size_t lightIndex = 0; lightIndex < p_pScene->LightList.Size(); lightIndex++)
			{
				Vector3 wIn; // = Vector3(0,16.5f,0) - p_intersection.Surface.PointWS;

				p_pScene->LightList[lightIndex]->SampleRadiance(p_intersection.Surface.PointWS, 0.5, 0.5, wIn, query);
				L = L + Maths::Max(0, Vector3::Dot(p_intersection.Surface.GeometryBasisWS.W, -wIn));
			}

			if (!p_intersection.HasMaterial())
				break;

			sample = p_pScene->GetSampler()->Get2DSample();
			Spectrum f = p_intersection.GetMaterial()->SampleF(p_intersection.Surface, wOut, wIn, sample.X, sample.Y, &pdf);

			L = L * f;

			L[0] = p_intersection.Surface.GeometryBasisWS.W[0];
			L[1] = p_intersection.Surface.GeometryBasisWS.W[1];
			L[2] = p_intersection.Surface.GeometryBasisWS.W[2];

			sample = p_pScene->GetSampler()->Get2DSample();
			BSDF::GenerateVectorInHemisphere(sample.X, sample.Y, wIn);
			wIn = p_intersection.Surface.GeometryBasisWS.Project(wIn);


			ray.Direction = wIn;
			ray.Origin = p_intersection.Surface.PointWS + p_intersection.Surface.GeometryBasisWS.W * 1E-5f;
			ray.Min = 1E-4f;
			ray.Max = Maths::Maximum;

			//for (size_t lightIdx = 0; lightIdx < p_pScene->LightList.Size(); lightIdx++)
			//{
			//	Vector3 wIn(30, 30, 30);

			//	wIn.Normalize();

			//	L = Spectrum(100) * Maths::Max(0, Vector3::Dot(p_intersection.Surface.GeometryBasisWS.W, wIn));
			//}
		}
	}

	return L;
}
//----------------------------------------------------------------------------------------------