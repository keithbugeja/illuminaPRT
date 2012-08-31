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
#include "System/EngineKernel.h"
#include "Scene/SceneLoader.h"

struct WavefrontContext;

//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		class WavefrontSceneLoader
			: public ISceneLoader
		{
		protected:
			using ISceneLoader::m_pEnvironment;

		protected:
			EngineKernel* m_pEngineKernel;

			std::map<std::string, int> m_vertexMap;
			std::vector<Vector3> m_positionList;
			std::vector<Vector3> m_normalList;
			std::vector<Vector2> m_uvList;

		public:
			WavefrontSceneLoader(Environment *p_pEnvironment);

			bool Import(const std::string &p_strFilename, unsigned int p_uiFlags, ArgumentMap* p_pArgumentMap = NULL);
			bool Export(const std::string &p_strFilename, unsigned int p_uiFlags, ArgumentMap* p_pArgumentMap = NULL);
		
		protected:
			bool LoadMaterials(const std::string &p_strFilename, WavefrontContext &p_context);
			bool LoadGeometry(const std::string &p_strFilename, WavefrontContext &p_context);
			int Tokenise(std::string &p_strText, char *p_pSeparators, std::vector<std::string> &p_tokenList);
		}; 
		//----------------------------------------------------------------------------------------------
	}
}
