//----------------------------------------------------------------------------------------------
//	Filename:	SceneLoader.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
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
				Settings	= 1 << 0,
				Geometry	= 1 << 1,
				Materials	= 1 << 2,
				Textures	= 1 << 3,
				Luminaires	= 1 << 4,
				World		= Geometry | Materials | Textures | Luminaires,
				All			= Settings | World
			};

		public:
			virtual bool Import(const std::string &p_strFilename, Environment *p_pEnvironment, unsigned int p_generalFlags, unsigned int p_uiLoaderFlags = 0) = 0;
			virtual bool Export(const std::string &p_strFilename, Environment *p_pEnvironment, unsigned int p_generalFlags, unsigned int p_uiLoaderFlags = 0) = 0;
		};
	}
}