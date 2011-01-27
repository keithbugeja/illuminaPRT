//----------------------------------------------------------------------------------------------
//	Filename:	EnvironmentLoader.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
#include <vector>
#include <stack>
#include <map>

#include "System/IlluminaPRT.h"
#include "System/Lexer.h"
#include "Scene/SceneLoader.h"

//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class EnvironmentLoader : 
			public ISceneLoader
		{
		protected:
			using ISceneLoader::m_pEngineKernel;
			using ISceneLoader::m_pEnvironment;

			std::stack<Lexer*> m_lexerStack;

		protected:
			void Push(Lexer *p_pLexer);
			Lexer* Pop(void);
			Lexer* Top(void);
			
			bool Load(const std::string p_strFilename);

			bool Parse(void);
			bool ParseList(const std::string& p_strContainerName, std::vector<std::map<std::string, std::string>> p_containerList);

			bool ParseInclude(void);
			bool ParseCameras(void);
			bool ParseLights(void);
			bool ParseFilters(void);
			bool ParseDevices(void);
			bool ParseSamplers(void);
			bool ParseIntegrators(void);
			bool ParseRenderers(void);
			bool ParseGeometries(void);
			bool ParseMaterials(void);
			bool ParseEnvironment(void);

		public:
			EnvironmentLoader(EngineKernel *p_pEngineKernel, Environment *p_pEnvironment);

			bool Import(const std::string &p_strFilename, unsigned int p_uiFlags, ArgumentMap* p_pArgumentMap = NULL);
			bool Export(const std::string &p_strFilename, unsigned int p_uiFlags, ArgumentMap* p_pArgumentMap = NULL);
		};
	}
}
