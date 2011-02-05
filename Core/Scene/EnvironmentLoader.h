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
		//----------------------------------------------------------------------------------------------
		// Consider putting this into a separate namespace to avoid problems with naming conflicts
		//----------------------------------------------------------------------------------------------
		class ParseNode
		{
		public:
			enum NodeType
			{
				Internal,
				Leaf
			} Type;

		public:
			std::string Name;
			std::string Value;

			std::vector<ParseNode*> Children;

		public:
			ParseNode(const std::string &p_strName, const std::string &p_strValue);
			ParseNode(const std::string &p_strName);
			ParseNode(NodeType p_type = ParseNode::Leaf);

			bool FindByName(const std::string& p_strName, std::vector<ParseNode*> &p_nodeList);
			bool GetArgumentMap(ArgumentMap &p_argumentMap);
		};

		//----------------------------------------------------------------------------------------------
		class ParseTree
		{
		protected:
			std::vector<ParseNode*> m_reservedNodeList;

		public:
			ParseNode Root;

		public:
			ParseNode* RequestNode(const std::string &p_strName, const std::string &p_strValue);
			ParseNode* RequestNode(const std::string &p_strName);
			ParseNode* RequestNode(ParseNode::NodeType p_type = ParseNode::Leaf);

			void ReleaseNodes(void);
		};

		//----------------------------------------------------------------------------------------------
		class EnvironmentLoader : 
			public ISceneLoader
		{
		protected:
			using ISceneLoader::m_pEnvironment;

		protected:
			EngineKernel *m_pEngineKernel;
			std::stack<Lexer*> m_lexerStack;
			ParseTree m_parseTree;

		public:
			EnvironmentLoader(Environment *p_pEnvironment);

			bool Import(const std::string &p_strFilename, unsigned int p_uiFlags, ArgumentMap* p_pArgumentMap = NULL);
			bool Export(const std::string &p_strFilename, unsigned int p_uiFlags, ArgumentMap* p_pArgumentMap = NULL);

		protected:
			void Push(Lexer *p_pLexer);
			Lexer* Pop(void);
			Lexer* Top(void);
			
			bool Load(const std::string p_strFilename);

			bool Parse(void);
			bool ParseCameras(void);
			bool ParseLights(void);
			bool ParseFilters(void);
			bool ParseDevices(void);
			bool ParseSamplers(void);
			bool ParseIntegrators(void);
			bool ParseRenderers(void);
			bool ParseShapes(void);
			bool ParseMaterials(void);
			bool ParseEnvironment(void);
			
			bool GetNodeList(const std::string &p_strCategoryName, const std::string &p_strInstanceName, std::vector<ParseNode*> &p_nodeList, ParseNode *p_pNode = NULL);
		};
		//----------------------------------------------------------------------------------------------
	}
}
