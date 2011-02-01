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
#include "Scene/WavefrontSceneLoader.h"
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

	ParseCameras();
	ParseLights();
	ParseFilters();
	ParseDevices();
	ParseSamplers();
	ParseIntegrators();
	ParseShapes();

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
	std::cout << "Parsing Cameras..." << std::endl;

	ArgumentMap argumentMap;

	std::vector<ParseNode*> cameras;
	std::vector<ParseNode*>::iterator camerasIterator;
	
	std::vector<ParseNode*> cameraNodes;
	std::vector<ParseNode*>::iterator cameraNodesIterator;

	m_parseTree.Root.FindByName("Cameras", cameras);

	for (camerasIterator = cameras.begin();
		 camerasIterator != cameras.end();
		 ++camerasIterator)
	{
		ParseNode *pCamerasNode = *camerasIterator;

		// Go throug all shape entries in the shapes node
		pCamerasNode->FindByName("Camera", cameraNodes);
		
		for (cameraNodesIterator = cameraNodes.begin();
			 cameraNodesIterator != cameraNodes.end();
			 ++cameraNodesIterator)
		{
			ParseNode *pCameraNode = *cameraNodesIterator;
			pCameraNode->GetArgumentMap(argumentMap);

			// If argument map does not specify geometry type or name/id, ignore entry
			if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
			{
				std::cout << "[Camera] Warning :: Ignoring Shape entry because it does not specify Id or Type..." << std::endl;
				continue;
			}

			// Get shape factory and create instance
			std::string strType, strId;

			argumentMap.GetArgument("Id", strId);
			argumentMap.GetArgument("Type", strType);

			// Filter some types like model filters
			ICamera *pCamera = m_pEngineKernel->GetCameraManager()->CreateInstance(strType, strId, argumentMap);
			std::cout << pCamera->ToString() << ", " << pCamera->GetName() << std::endl;
		}
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseLights(void)
{
	std::cout << "Parsing Lights..." << std::endl;

	ArgumentMap argumentMap;

	std::vector<ParseNode*> lights;
	std::vector<ParseNode*>::iterator lightsIterator;
	
	std::vector<ParseNode*> lightNodes;
	std::vector<ParseNode*>::iterator lightNodesIterator;

	m_parseTree.Root.FindByName("Lights", lights);

	for (lightsIterator = lights.begin();
		 lightsIterator != lights.end();
		 ++lightsIterator)
	{
		ParseNode *pLightsNode = *lightsIterator;

		// Go throug all shape entries in the shapes node
		pLightsNode->FindByName("Light", lightNodes);
		
		for (lightNodesIterator = lightNodes.begin();
			 lightNodesIterator != lightNodes.end();
			 ++lightNodesIterator)
		{
			ParseNode *pLightNode = *lightNodesIterator;
			pLightNode->GetArgumentMap(argumentMap);

			// If argument map does not specify geometry type or name/id, ignore entry
			if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
			{
				std::cout << "[Light] Warning :: Ignoring Shape entry because it does not specify Id or Type..." << std::endl;
				continue;
			}

			// Get shape factory and create instance
			std::string strType, strId;

			argumentMap.GetArgument("Id", strId);
			argumentMap.GetArgument("Type", strType);

			// Filter some types like model filters
			ILight *pLight = m_pEngineKernel->GetLightManager()->CreateInstance(strType, strId, argumentMap);
			std::cout << pLight->ToString() << ", " << pLight->GetName() << std::endl;
		}
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseFilters(void)
{
	std::cout << "Parsing Filters..." << std::endl;

	ArgumentMap argumentMap;

	std::vector<ParseNode*> filters;
	std::vector<ParseNode*>::iterator filtersIterator;
	
	std::vector<ParseNode*> filterNodes;
	std::vector<ParseNode*>::iterator filterNodesIterator;

	m_parseTree.Root.FindByName("Filters", filters);

	for (filtersIterator = filters.begin();
		 filtersIterator != filters.end();
		 ++filtersIterator)
	{
		ParseNode *pFiltersNode = *filtersIterator;

		// Go throug all shape entries in the shapes node
		pFiltersNode->FindByName("Filter", filterNodes);
		
		for (filterNodesIterator = filterNodes.begin();
			 filterNodesIterator != filterNodes.end();
			 ++filterNodesIterator)
		{
			ParseNode *pFilterNode = *filterNodesIterator;
			pFilterNode->GetArgumentMap(argumentMap);

			// If argument map does not specify geometry type or name/id, ignore entry
			if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
			{
				std::cout << "[Filter] Warning :: Ignoring entry because it does not specify Id or Type..." << std::endl;
				continue;
			}

			// Get factory and create instance
			std::string strType, strId;

			argumentMap.GetArgument("Id", strId);
			argumentMap.GetArgument("Type", strType);

			// Filter some types like model filters
			IFilter *pFilter = m_pEngineKernel->GetFilterManager()->CreateInstance(strType, strId, argumentMap);
			std::cout << pFilter->ToString() << ", " << pFilter->GetName() << std::endl;
		}
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseSamplers(void)
{
	std::cout << "Parsing Samplers..." << std::endl;

	ArgumentMap argumentMap;

	std::vector<ParseNode*> samplers;
	std::vector<ParseNode*>::iterator samplersIterator;
	
	std::vector<ParseNode*> samplerNodes;
	std::vector<ParseNode*>::iterator samplerNodesIterator;

	m_parseTree.Root.FindByName("Samplers", samplers);

	for (samplersIterator = samplers.begin();
		 samplersIterator != samplers.end();
		 ++samplersIterator)
	{
		ParseNode *pSamplersNode = *samplersIterator;

		// Go throug all shape entries in the shapes node
		pSamplersNode->FindByName("Sampler", samplerNodes);
		
		for (samplerNodesIterator = samplerNodes.begin();
			 samplerNodesIterator != samplerNodes.end();
			 ++samplerNodesIterator)
		{
			ParseNode *pSamplerNode = *samplerNodesIterator;
			pSamplerNode->GetArgumentMap(argumentMap);

			// If argument map does not specify geometry type or name/id, ignore entry
			if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
			{
				std::cout << "[Sampler] Warning :: Ignoring entry because it does not specify Id or Type..." << std::endl;
				continue;
			}

			// Get factory and create instance
			std::string strType, strId;

			argumentMap.GetArgument("Id", strId);
			argumentMap.GetArgument("Type", strType);

			// Filter some types like model filters
			ISampler *pSampler = m_pEngineKernel->GetSamplerManager()->CreateInstance(strType, strId, argumentMap);
			std::cout << pSampler->ToString() << ", " << pSampler->GetName() << std::endl;
		}
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseShapes(void)
{
	std::cout << "Parsing Shapes..." << std::endl;

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
				std::cout << "[Shape] Warning :: Ignoring Shape entry because it does not specify Id or Type..." << std::endl;
				continue;
			}

			// Get shape factory and create instance
			std::string strType, strId;

			argumentMap.GetArgument("Id", strId);
			argumentMap.GetArgument("Type", strType);

			// Filter some types like model filters
			if (strType == "WavefrontModel")
			{
				std::string strFilename;
				argumentMap.GetArgument("Filename", strFilename);

				WavefrontSceneLoader wavefrontLoader(m_pEnvironment);
				wavefrontLoader.Import(strFilename, 0, &argumentMap);
			}
			else
			{
				IShape *pShape = m_pEngineKernel->GetShapeManager()->CreateInstance(strType, strId, argumentMap);
				std::cout << pShape->ToString() << ", " << pShape->GetName() << std::endl;
			}
		}
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseDevices(void)
{
	std::cout << "Parsing Devices..." << std::endl;

	ArgumentMap argumentMap;

	std::vector<ParseNode*> devices;
	std::vector<ParseNode*>::iterator devicesIterator;
	
	std::vector<ParseNode*> deviceNodes;
	std::vector<ParseNode*>::iterator deviceNodesIterator;

	m_parseTree.Root.FindByName("Devices", devices);

	for (devicesIterator = devices.begin();
		 devicesIterator != devices.end();
		 ++devicesIterator)
	{
		ParseNode *pDevicesNode = *devicesIterator;

		// Go throug all shape entries in the shapes node
		pDevicesNode->FindByName("Device", deviceNodes);
		
		for (deviceNodesIterator = deviceNodes.begin();
			 deviceNodesIterator != deviceNodes.end();
			 ++deviceNodesIterator)
		{
			ParseNode *pDeviceNode = *deviceNodesIterator;
			pDeviceNode->GetArgumentMap(argumentMap);

			// If argument map does not specify geometry type or name/id, ignore entry
			if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
			{
				std::cout << "[Device] Warning :: Ignoring entry because it does not specify Id or Type..." << std::endl;
				continue;
			}

			// Get factory and create instance
			std::string strType, strId;

			argumentMap.GetArgument("Id", strId);
			argumentMap.GetArgument("Type", strType);

			// Filter some types like model filters
			IDevice *pDevice = m_pEngineKernel->GetDeviceManager()->CreateInstance(strType, strId, argumentMap);
			std::cout << pDevice->ToString() << ", " << pDevice->GetName() << std::endl;
		}
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseIntegrators(void)
{
	std::cout << "Parsing Integrators..." << std::endl;

	ArgumentMap argumentMap;

	std::vector<ParseNode*> integrators;
	std::vector<ParseNode*>::iterator integratorsIterator;
	
	std::vector<ParseNode*> integratorNodes;
	std::vector<ParseNode*>::iterator integratorNodesIterator;

	m_parseTree.Root.FindByName("Integrators", integrators);

	for (integratorsIterator = integrators.begin();
		 integratorsIterator != integrators.end();
		 ++integratorsIterator)
	{
		ParseNode *pIntegratorsNode = *integratorsIterator;

		// Go throug all shape entries in the shapes node
		pIntegratorsNode->FindByName("Integrator", integratorNodes);
		
		for (integratorNodesIterator = integratorNodes.begin();
			 integratorNodesIterator != integratorNodes.end();
			 ++integratorNodesIterator)
		{
			ParseNode *pIntegratorNode = *integratorNodesIterator;
			pIntegratorNode->GetArgumentMap(argumentMap);

			// If argument map does not specify geometry type or name/id, ignore entry
			if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
			{
				std::cout << "[Integrator] Warning :: Ignoring entry because it does not specify Id or Type..." << std::endl;
				continue;
			}

			// Get factory and create instance
			std::string strType, strId;

			argumentMap.GetArgument("Id", strId);
			argumentMap.GetArgument("Type", strType);

			// Filter some types like model filters
			IIntegrator *pIntegrator = m_pEngineKernel->GetIntegratorManager()->CreateInstance(strType, strId, argumentMap);
			std::cout << pIntegrator->ToString() << ", " << pIntegrator->GetName() << std::endl;
		}
	}

	return true;
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
