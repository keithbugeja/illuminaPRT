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
		// Output Device [Image Devices, etc]
		// Camera type / position etc
		// Renderer type
		// Sampler type
		// Integrator type
		// Acceleration structure to use
		// Space structure to use
		// Geometry
		// Materials
		// Textures
		// Primitives
		// Lights

		/**/
		// Loads a scene into the environment
		
		class ISceneLoader
		{

		public:
			enum Flags
			{
				Environment	= 1 << 0,
				Geometry	= 1 << 1,
				Materials	= 1 << 2,
				Textures	= 1 << 3,
				Luminaires	= 1 << 4,
				World		= Geometry | Materials | Textures | Luminaires,
				Scene		= Environment | World;
			};

		public:
			ISceneLoader(EngineKernel *p_pEngineKernel);

			virtual Import(const std::string &p_strFilename, Environment *p_pEnvironment, Flags p_generalFlags = Scene, unsigned int p_uiLoaderFlags) = 0;
			virtual Export(const std::string &p_strFilename, Environment *p_pEnvironment, Flags p_generalFlags = Scene, unsigned int p_uiLoaderFlags) = 0;
		};


		/*
		class WavefrontSceneLoader : public ISceneLoader
		{

		};

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

			Load(const std::string &p_strEnvironmentName);
			Unload(void);
		};
		*/
	}
}