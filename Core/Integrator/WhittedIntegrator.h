#pragma once

#include "Integrator/Integrator.h"

namespace Illumina
{
	namespace Core
	{
		class WhittedIntegrator : public IIntegrator
		{
		public:
			bool Initialise(Scene *p_pScene, ICamera *p_pCamera)
			{
				std::cout << "Whitted Integrator :: Initialise()" << std::endl;
				return true;
			}

			bool Shutdown(void)
			{
				std::cout << "Whitted Integrator :: Shutdown()" << std::endl;
				return true;
			}

			Spectrum Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection)
			{
				Spectrum result;
				bool bHit = p_pScene->Intersects(p_ray, p_intersection);
				
				if (bHit)
				{
					DifferentialSurface &surface = p_intersection.GetDifferentialSurface();
					List<ILight*> &lightList = p_pScene->GetLightList();

					Vector3 wOut;
					VisibilityQuery visibilityQuery(p_pScene);

					for (int j = 0; j < lightList.Size(); ++j)
					{
						Spectrum lightLi = lightList[j]->Radiance(surface.Point, wOut, visibilityQuery);
							
						if (!visibilityQuery.IsOccluded())
						{
							wOut.Normalize();
							result += lightLi * Maths::Max(0, Vector3::Dot(wOut, surface.Normal));
						}
					}
				}

				return result;
			}
		};
	}
}