//----------------------------------------------------------------------------------------------
//	Filename:	Scene.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Threading/List.h"
#include "Geometry/Ray.h"
#include "Light/Light.h"
#include "Space/Space.h"
#include "Sampler/Sampler.h"
//----------------------------------------------------------------------------------------------

namespace Illumina
{
	namespace Core
	{
		class Scene
		{
		private:
			ISpace *m_pSpace;
			ICamera *m_pCamera;
			ISampler *m_pSampler;
			
		public:
			List<ILight*> LightList;			

		public:
			Scene(void);
			Scene(ISpace *p_pSpace, ISampler *p_pSampler);

			bool Intersects(const Ray &p_ray);
			bool Intersects(const Ray &p_ray, IPrimitive *p_pExclude);
			bool Intersects(const Ray &p_ray, Intersection &p_intersection);
			bool Intersects(const Ray &p_ray, Intersection &p_intersection, IPrimitive *p_pExclude);
			
			ISpace* GetSpace(void) const;
			void SetSpace(ISpace *p_pSpace);

			ICamera* GetCamera(void) const;
			void SetCamera(ICamera *p_pCamera);

			ISampler* GetSampler(void) const;
			void SetSampler(ISampler *p_pSampler);
		};
	}
}