//----------------------------------------------------------------------------------------------
//	Filename:	WavefrontSceneLoader.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
#pragma once

#include <iostream>
#include <fstream>
#include <map>

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

#include "System/IlluminaPRT.h"
#include  "Scene/SceneLoader.h"

/*
#include "System/EngineKernel.h"
#include "Shape/VertexFormats.h"
#include "Shape/Shape.h"
#include "Spectrum/Spectrum.h"
*/

namespace Illumina
{
	namespace Core
	{/*
		//----------------------------------------------------------------------------------------------
		class WavefrontSceneLoader
			: public ISceneLoader
		{
		protected:
			EngineKernel *m_pEngineKernel;

		public:
			WavefrontSceneLoader(EngineKernel *p_pEngineKernel);

			bool Import(const std::string &p_strFilename, Environment *p_pEnvironment, unsigned int p_generalFlags, unsigned int p_uiLoaderFlags);
			bool Export(const std::string &p_strFilename, Environment *p_pEnvironment, unsigned int p_generalFlags, unsigned int p_uiLoaderFlags);
		
		protected:
			bool LoadMaterials(const std::string &p_strFilename, Environment *p_pEnvironment);
			bool LoadGeometry(const std::string &p_strFilename, Environment *p_pEnvironment);
		}; */
		//----------------------------------------------------------------------------------------------
	}
}
