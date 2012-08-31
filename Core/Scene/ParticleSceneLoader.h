//----------------------------------------------------------------------------------------------
//	Filename:	ParticleSceneLoader.h
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
#include "System/EngineKernel.h"
#include "Scene/SceneLoader.h"

struct ParticleContext;

//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		class ParticleSceneLoader
			: public ISceneLoader
		{
		protected:
			using ISceneLoader::m_pEnvironment;

		protected:
			EngineKernel* m_pEngineKernel;

		public:
			ParticleSceneLoader(Environment *p_pEnvironment);

			bool Import(const std::string &p_strFilename, unsigned int p_uiFlags, ArgumentMap* p_pArgumentMap = NULL);
			bool Export(const std::string &p_strFilename, unsigned int p_uiFlags, ArgumentMap* p_pArgumentMap = NULL);
		
		protected:
			bool LoadMaterials(const std::string &p_strFilename, ParticleContext &p_context);
			bool LoadGeometry(const std::string &p_strFilename, ParticleContext &p_context);
			int Tokenise(std::string &p_strText, char *p_pSeparators, std::vector<std::string> &p_tokenList);
		}; 
		//----------------------------------------------------------------------------------------------
	}
}
