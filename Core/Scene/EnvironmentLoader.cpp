//----------------------------------------------------------------------------------------------
//	Filename:	EnvironmentLoader.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
#include <fstream>
#include <vector>
#include <stack>
#include <map>

#include "System/EngineKernel.h"
#include "Scene/EnvironmentLoader.h"
#include "Scene/Environment.h"
#include "System/Lexer.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
ParseNode::ParseNode(const std::string &p_strName, const std::string &p_strValue)
	: Name(p_strName)
	, Value(p_strValue)
	, Type(ParseNode::Leaf)
{ }
//----------------------------------------------------------------------------------------------
ParseNode::ParseNode(const std::string &p_strName)
	: Name(p_strName)
	, Type(ParseNode::Internal)
{ }
//----------------------------------------------------------------------------------------------
ParseNode::ParseNode(ParseNode::NodeType p_type)
	: Type(p_type)
{ }
//----------------------------------------------------------------------------------------------
bool ParseNode::FindByName(const std::string& p_strName, std::vector<ParseNode*> &p_nodeList)
{
	p_nodeList.clear();

	std::vector<ParseNode*>::iterator nodeIterator;

	for (nodeIterator = Children.begin(); 
		 nodeIterator != Children.end();
		 ++nodeIterator)
	{
		if ((*nodeIterator)->Name == p_strName)
			p_nodeList.push_back(*nodeIterator);
	}

	return (p_nodeList.size() != 0);
}
//----------------------------------------------------------------------------------------------
bool ParseNode::GetArgumentMap(ArgumentMap &p_argumentMap)
{
	std::map<std::string, std::string> argumentMap;
	std::vector<ParseNode*>::iterator nodeIterator;

	for (nodeIterator = Children.begin(); 
		 nodeIterator != Children.end();
		 ++nodeIterator)
	{
		if ((*nodeIterator)->Type == ParseNode::Leaf)
			argumentMap[(*nodeIterator)->Name] = (*nodeIterator)->Value;
	}

	p_argumentMap.Initialise(argumentMap);

	return (argumentMap.size() != 0);
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
ParseNode* ParseTree::RequestNode(const std::string &p_strName, const std::string &p_strValue)
{
	ParseNode *pNode = new ParseNode(p_strName, p_strValue);
	m_reservedNodeList.push_back(pNode);
	return pNode;
}
//----------------------------------------------------------------------------------------------
ParseNode* ParseTree::RequestNode(const std::string &p_strName)
{
	ParseNode *pNode = new ParseNode(p_strName);
	m_reservedNodeList.push_back(pNode);
	return pNode;
}
//----------------------------------------------------------------------------------------------
ParseNode* ParseTree::RequestNode(ParseNode::NodeType p_type)
{
	ParseNode *pNode = new ParseNode(p_type);
	m_reservedNodeList.push_back(pNode);
	return pNode;
}
//----------------------------------------------------------------------------------------------
void ParseTree::ReleaseNodes(void)
{
	for (int nodeIndex = 0; nodeIndex < m_reservedNodeList.size(); ++nodeIndex)
	{
		delete m_reservedNodeList.at(nodeIndex);
	}

	m_reservedNodeList.clear();
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
EnvironmentLoader::EnvironmentLoader(Environment *p_pEnvironment)
	: ISceneLoader(p_pEnvironment) 
{ 
	BOOST_ASSERT(p_pEnvironment != NULL);
	m_pEngineKernel = p_pEnvironment->GetEngineKernel();
}
//----------------------------------------------------------------------------------------------
Lexer* EnvironmentLoader::Top(void) {
	return m_lexerStack.size() == 0 ? NULL : m_lexerStack.top();
}
//----------------------------------------------------------------------------------------------
void EnvironmentLoader::Push(Lexer *p_pLexer) {
	m_lexerStack.push(p_pLexer);
}
//----------------------------------------------------------------------------------------------
Lexer* EnvironmentLoader::Pop(void)
{
	Lexer *temp = Top();
	m_lexerStack.pop();

	return temp;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::Load(const std::string p_strFilename)
{
	// Open filestream
	std::ifstream filestream;
	filestream.open(p_strFilename.c_str());

	if (!filestream.is_open())
		return false;

	// Create lexer for input stream
	Lexer lexer(&filestream);

	// Push lexer on stack
	Push(&lexer);

	// Parse current lexer
	Parse();

	// Pop lexer once parsing is complete
	Pop();

	// Close filestream
	filestream.close();

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::Parse(void)
{
	m_parseTree.ReleaseNodes();

	std::stack<ParseNode*> nodeStack;
	nodeStack.push(&m_parseTree.Root);

	LexerToken nameToken,
		valueToken;

	Lexer *pLexer = Top();

	while(pLexer->ReadToken(nameToken))
	{
		if (nameToken.Type == LexerToken::PropertyName)
		{
			std::cout << "Property : [" << nameToken.Value << "]" << std::endl;

			if (!pLexer->ReadToken(valueToken))
				return false;
			
			// Go deeper
			if (valueToken.Type == LexerToken::LeftCurly)
			{
				ParseNode *pNode = m_parseTree.RequestNode(nameToken.Value);
				nodeStack.top()->Children.push_back(pNode);
				nodeStack.push(pNode);

				std::cout << " { " << std::endl;
			}
			else
			{
				ParseNode *pNode = m_parseTree.RequestNode(nameToken.Value, valueToken.Value);
				nodeStack.top()->Children.push_back(pNode);

				std::cout << "[" << nameToken.Value << " = " << valueToken.Value << "]" << std::endl;
			}
		}
		else if (nameToken.Type == LexerToken::RightCurly)
		{
			nodeStack.pop();

			std::cout << " } " << std::endl;
		}
	}

	ParseShapes();

	/*
	LexerToken token;
	Lexer *pLexer = Top();

	while(pLexer->ReadToken(token))
	{
		if (token.Type == LexerToken::PropertyName)
		{
			if (token.Value == "Include") ParseInclude();
			else if (token.Value == "Cameras") ParseCameras();
			else if (token.Value == "Lights") ParseLights();
			else if (token.Value == "Filters") ParseFilters();
			else if (token.Value == "Devices") ParseDevices();
			else if (token.Value == "Samplers") ParseSamplers();
			else if (token.Value == "Integrators") ParseIntegrators();
			else if (token.Value == "Renderers") ParseRenderers();
			else if (token.Value == "Geometries") ParseGeometries();
			else if (token.Value == "Materials") ParseMaterials();
			else if (token.Value == "Environment") ParseEnvironment();
		}
	}
	*/
	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseList(const std::string& p_strContainerName, std::vector<std::map<std::string, std::string>> p_containerList)
{
	Lexer *pLexer = Top();
	LexerToken token, 
		nameToken, 
		valueToken;

	// {
	if (!pLexer->ReadToken(LexerToken::LeftCurly, token)) 
		return false;

	while (true)
	{
		// ContainerName { ... } 
		// -- or --
		// }

		// EOS
		if (!pLexer->ReadToken(token))
			return false;

		// }
		if (token.Type == LexerToken::RightCurly)
			return true;

		// ContainerName
		if (token.Type == LexerToken::PropertyName && token.Value == p_strContainerName)
		{
			if (!pLexer->ReadToken(LexerToken::LeftCurly, token))
				return false;

			std::cout << "Pushing " << p_strContainerName << " object ..." << std::endl;
			p_containerList.push_back(std::map<std::string, std::string>());

			while (true)
			{
				if (!pLexer->ReadToken(LexerToken::PropertyName, nameToken))
				{
					if (nameToken.Type == LexerToken::RightCurly) 
						break;

					return false;
				}

				if (!pLexer->ReadToken(LexerToken::PropertyValue, valueToken))
					return false;

				std::cout << "[" << nameToken.Value << " = " << valueToken.Value << "]" << std::endl;
				p_containerList.back()[nameToken.Value] = valueToken.Value;
			}
		}
	}
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseInclude(void)
{
	Lexer *pLexer = Top();
	LexerToken token;
				
	if (!pLexer->ReadToken(LexerToken::LeftCurly, token))
		return false;
				
	while (true)
	{
		if (!pLexer->ReadToken(token)) 
			return false; 

		if (token.Type == LexerToken::RightCurly)
			return true;

		if (token.Value != "Include")
			return false;

		if (!pLexer->ReadToken(token))
			return false;

		std::cout << "Including file [" << token.Value << "] ..." << std::endl;
		Load(token.Value);
	}
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseCameras(void)
{
	std::cout << "[Cameras]" << std::endl;

	std::vector<std::map<std::string, std::string>> cameraList;
	bool result = ParseList("Camera", cameraList);
	return result;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseLights(void)
{
	std::cout << "[Lights]" << std::endl;

	std::vector<std::map<std::string, std::string>> lightList;
	bool result = ParseList("Light", lightList);
	return result;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseFilters(void)
{
	std::cout << "[Filters]" << std::endl;

	std::vector<std::map<std::string, std::string>> filterList;
	bool result = ParseList("Filter", filterList);
	return result;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseDevices(void)
{
	std::cout << "[Devices]" << std::endl;

	std::vector<std::map<std::string, std::string>> deviceList;
	bool result = ParseList("Device", deviceList);
	return result;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseSamplers(void)
{
	std::cout << "[Samplers]" << std::endl;

	std::vector<std::map<std::string, std::string>> samplerList;
	bool result = ParseList("Sampler", samplerList);
	return result;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseIntegrators(void)
{
	std::cout << "[Integrators]" << std::endl;

	std::vector<std::map<std::string, std::string>> integratorList;
	bool result = ParseList("Integrator", integratorList);
	return result;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseRenderers(void)
{
	std::cout << "[Renderers]" << std::endl;

	std::vector<std::map<std::string, std::string>> rendererList;
	bool result = ParseList("Renderer", rendererList);
	return result;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseShapes(void)
{
	ArgumentMap argumentMap;

	std::vector<ParseNode*> shapes;
	std::vector<ParseNode*>::iterator shapesIterator;
	
	std::vector<ParseNode*> shapeNodes;
	std::vector<ParseNode*>::iterator shapeNodesIterator;

	m_parseTree.Root.FindByName("Shapes", shapes);

	for (shapesIterator = shapes.begin();
		 shapesIterator != shapes.end();
		 ++shapesIterator)
	{
		ParseNode *pShapesNode = *shapesIterator;

		// Go throug all shape entries in the shapes node
		pShapesNode->FindByName("Shape", shapeNodes);
		
		for (shapeNodesIterator = shapeNodes.begin();
			 shapeNodesIterator != shapeNodes.end();
			 ++shapeNodesIterator)
		{
			ParseNode *pShapeNode = *shapeNodesIterator;
			pShapeNode->GetArgumentMap(argumentMap);

			// If argument map does not specify geometry type or name/id, ignore entry
			if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
			{
				std::cout << "Warning :: Ignoring Shape entry because it does not specify Id or Type..." << std::endl;
				continue;
			}

			// Get shape factory and create instance
			std::string strType, strId;

			argumentMap.GetArgument("Id", strId);
			argumentMap.GetArgument("Type", strType);

			m_pEngineKernel->GetShapeManager()->CreateInstance(strType, strId, argumentMap);
		}
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseMaterials(void)
{
	std::cout << "[Materials]" << std::endl;

	std::vector<std::map<std::string, std::string>> materialList;
	bool result = ParseList("Material", materialList);
	return result;
}
/*
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseScene(void)
{
	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseSpace(void)
{
	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParsePrimitives(void)
{
	return true;
}
*/
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseEnvironment(void)
{
	Lexer *pLexer = Top();
	LexerToken token;
	
	// {
	if (!pLexer->ReadToken(LexerToken::LeftCurly, token))
		return false;

	while (true)
	{
		// Finished parsing Environment block
		if (!pLexer->ReadToken(LexerToken::PropertyName, token))
			return (token.Type == LexerToken::RightCurly);

		// Parse Scene
		if (token.Value == "Scene")
		{
		}
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::Import(const std::string &p_strFilename, unsigned int p_uiFlags, ArgumentMap* p_pArgumentMap)
{
	return Load(p_strFilename);
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::Export(const std::string &p_strFilename, unsigned int p_uiFlags, ArgumentMap* p_pArgumentMap)
{
	return false;
}
//----------------------------------------------------------------------------------------------