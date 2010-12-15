#pragma once

#include "Geometry/Ray.h"

namespace Illumina
{
	namespace Core
	{
		class Scene;
		class ICamera;
		class Intersection;

		class IIntegrator
		{
		public:
			virtual bool Initialise(Scene *p_pScene, ICamera *p_pCamera) = 0;
			virtual bool Shutdown(void) = 0;

			virtual Spectrum Radiance(Scene *p_pScene, const Ray &p_ray, Intersection &p_intersection) = 0;
		};
	}
}