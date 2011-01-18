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
		/*
		// Loads a scene into the environment
		class ISceneLoader
		{
			virtual LoadEnvironment(const std::string &p_strFilename, Environment &p_environment, unsigned int p_uiGeneralFlags = 0, unsigned int p_uiLoaderFlags) = 0;
			virtual LoadGeometry(const std::string &p_strFilename, 
		};

		class WavefrontSceneLoader : public ISceneLoader
		{

		};

		class Environment
		{
		protected:
			EngineKernel *m_pEngineKernel;
			IRenderer *m_pRenderer;
			Scene *m_pScene;

		public:
			Environment(EngineKernel *p_pEngineKernel);
			~Environment(void);

			Load(const std::string &p_strEnvironmentName);
			Unload(void);
		};
		*/
	}
}