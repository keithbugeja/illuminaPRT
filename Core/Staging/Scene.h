#pragma once

#include "Threading/List.h"
#include "Geometry/Ray.h"
#include "Light/Light.h"
#include "Space/Space.h"

namespace Illumina
{
	namespace Core
	{
		class Scene
		{
		private:
			ISpace *m_pSpace;
			List<ILight*> m_lightList;

		public:
			Scene(void);
			Scene(ISpace *p_pSpace);

			bool Intersects(const Ray &p_ray);
			bool Intersects(const Ray &p_ray, Intersection &p_intersection);
			
			ISpace* GetSpace(void);
			void SetSpace(ISpace *p_pSpace);

			List<ILight*>& GetLightList(void);
		};
	}
}