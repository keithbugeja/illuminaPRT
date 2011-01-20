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

class WavefrontContext;

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
			std::map<std::string, int> m_vertexMap;

			std::vector<Vector3> m_positionList;
			std::vector<Vector3> m_normalList;
			std::vector<Vector2> m_uvList;

			EngineKernel *m_pEngineKernel;

		public:
			WavefrontSceneLoader(EngineKernel *p_pEngineKernel);

			bool Import(const std::string &p_strFilename, Environment *p_pEnvironment, unsigned int p_uiFlags, ArgumentMap* p_pArgumentMap = NULL);
			bool Export(const std::string &p_strFilename, Environment *p_pEnvironment, unsigned int p_uiFlags, ArgumentMap* p_pArgumentMap = NULL);
		
		protected:
			bool LoadMaterials(const std::string &p_strFilename, WavefrontContext &p_context);
			bool LoadGeometry(const std::string &p_strFilename, WavefrontContext &p_context);
		}; 
		//----------------------------------------------------------------------------------------------
	}
}
