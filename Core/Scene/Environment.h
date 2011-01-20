//----------------------------------------------------------------------------------------------
//	Filename:	SceneLoader.h
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
		class Environment
		{
		protected:
			EngineKernel *m_pEngineKernel;
			IRenderer *m_pRenderer;
			ISampler *m_pSampler;
			ISpace *m_pSpace;
			Scene *m_pScene;

		public:
			Environment(EngineKernel *p_pEngineKernel);
			~Environment(void);

			//Load(const std::string &p_strEnvironmentName);
			//Unload(void);
		};
	}
}