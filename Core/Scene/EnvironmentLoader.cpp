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
#include "System/Lexer.h"

#include "Scene/Scene.h"
#include "Scene/EnvironmentLoader.h"
#include "Scene/WavefrontSceneLoader.h"
#include "Scene/Environment.h"

#include "Scene/EmissivePrimitive.h"
#include "Scene/GeometricPrimitive.h"

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

	if (!ParseCameras()) std::cerr << "ParseCameras :: Completed with warnings!" << std::endl;
	if (!ParseLights()) std::cerr << "Light Parsing :: Completed with warnings!" << std::endl;
	if (!ParseFilters()) std::cerr << "Filter Parsing :: Completed with warnings!" << std::endl;
	if (!ParseDevices()) std::cerr << "Device Parsing :: Completed with warnings!" << std::endl;
	if (!ParseSamplers()) std::cerr << "Sampler Parsing:: Completed with warnings!" << std::endl;
	if (!ParseIntegrators()) std::cerr << "Integrator Parsing :: Completed with warnings!" << std::endl;
	if (!ParseRenderers()) std::cerr << "Renderer Parsing :: Completed with warnings!" << std::endl;
	if (!ParseMaterials()) std::cerr << "Material Parsing :: Completed with warnings!" << std::endl;
	if (!ParseShapes()) std::cerr << "Shape Parsing :: Completed with warnings!" << std::endl;
	if (!ParseEnvironment()) std::cerr << "Environment Parsing :: Completed with warnings!" << std::endl;

	return true;
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

	std::string strType, 
		strId;

	if (!m_parseTree.Root.FindByName("Cameras", cameras))
		return false;

	for (camerasIterator = cameras.begin();
		 camerasIterator != cameras.end();
		 ++camerasIterator)
	{
		ParseNode *pCamerasNode = *camerasIterator;

		// Go throug all shape entries in the shapes node
		if (!pCamerasNode->FindByName("Camera", cameraNodes))
			continue;
		
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
			argumentMap.GetArgument("Id", strId);
			argumentMap.GetArgument("Type", strType);

			// Filter some types like model filters
			ICamera *pCamera = m_pEngineKernel->GetCameraManager()->CreateInstance(strType, strId, argumentMap);
			std::cout << "Created : [" << pCamera->ToString() << "] : " << pCamera->GetName() << std::endl;
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

	std::string strType, 
		strId;

	if (!m_parseTree.Root.FindByName("Lights", lights))
		return false;

	for (lightsIterator = lights.begin();
		 lightsIterator != lights.end();
		 ++lightsIterator)
	{
		ParseNode *pLightsNode = *lightsIterator;

		// Go throug all shape entries in the shapes node
		if (!pLightsNode->FindByName("Light", lightNodes))
			continue;
		
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

	std::string strType, 
		strId;

	if (!m_parseTree.Root.FindByName("Filters", filters))
		return false;

	for (filtersIterator = filters.begin();
		 filtersIterator != filters.end();
		 ++filtersIterator)
	{
		ParseNode *pFiltersNode = *filtersIterator;

		// Go throug all shape entries in the shapes node
		if (!pFiltersNode->FindByName("Filter", filterNodes))
			continue;
		
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

	std::string strType, 
		strId;

	if (!m_parseTree.Root.FindByName("Samplers", samplers))
		return false;

	for (samplersIterator = samplers.begin();
		 samplersIterator != samplers.end();
		 ++samplersIterator)
	{
		ParseNode *pSamplersNode = *samplersIterator;

		// Go throug all shape entries in the shapes node
		if (!pSamplersNode->FindByName("Sampler", samplerNodes))
			continue;
		
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
bool EnvironmentLoader::ParseDevices(void)
{
	std::cout << "Parsing Devices..." << std::endl;

	ArgumentMap argumentMap;

	std::vector<ParseNode*> devices;
	std::vector<ParseNode*>::iterator devicesIterator;
	
	std::vector<ParseNode*> deviceNodes;
	std::vector<ParseNode*>::iterator deviceNodesIterator;

	std::string strType, 
		strId;

	if (!m_parseTree.Root.FindByName("Devices", devices))
		return false;

	for (devicesIterator = devices.begin();
		 devicesIterator != devices.end();
		 ++devicesIterator)
	{
		ParseNode *pDevicesNode = *devicesIterator;

		// Go throug all shape entries in the shapes node
		if (!pDevicesNode->FindByName("Device", deviceNodes))
			continue;
		
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

	std::string strType, 
		strId;

	if (!m_parseTree.Root.FindByName("Integrators", integrators))
		return false;

	for (integratorsIterator = integrators.begin();
		 integratorsIterator != integrators.end();
		 ++integratorsIterator)
	{
		ParseNode *pIntegratorsNode = *integratorsIterator;

		// Go throug all shape entries in the shapes node
		if (!pIntegratorsNode->FindByName("Integrator", integratorNodes))
			continue;
		
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
bool EnvironmentLoader::ParseMaterials(void)
{
	std::cout << "Parsing Materials..." << std::endl;

	ArgumentMap argumentMap;

	std::vector<ParseNode*> materials;
	std::vector<ParseNode*>::iterator materialsIterator;
	
	std::vector<ParseNode*> materialNodes;
	std::vector<ParseNode*>::iterator materialNodesIterator;

	std::string strType, 
		strId;

	if (!m_parseTree.Root.FindByName("Materials", materials))
		return false;

	for (materialsIterator = materials.begin();
		 materialsIterator != materials.end();
		 ++materialsIterator)
	{
		ParseNode *pMaterialsNode = *materialsIterator;

		// Go throug all shape entries in the shapes node
		if (!pMaterialsNode->FindByName("Material", materialNodes))
			continue;
		
		for (materialNodesIterator = materialNodes.begin();
			 materialNodesIterator != materialNodes.end();
			 ++materialNodesIterator)
		{
			ParseNode *pMaterialNode = *materialNodesIterator;
			pMaterialNode->GetArgumentMap(argumentMap);

			// If argument map does not specify geometry type or name/id, ignore entry
			if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
			{
				std::cout << "[Material] Warning :: Ignoring entry because it does not specify Id or Type..." << std::endl;
				continue;
			}

			// Get factory and create instance
			argumentMap.GetArgument("Id", strId);
			argumentMap.GetArgument("Type", strType);

			// Filter some types like model filters
			IMaterial *pMaterial = m_pEngineKernel->GetMaterialManager()->CreateInstance(strType, strId, argumentMap);
			std::cout << pMaterial->ToString() << ", " << pMaterial->GetName() << std::endl;
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

	std::string strType, 
		strId;

	IShape *pShape;

	if (!m_parseTree.Root.FindByName("Shapes", shapes))
		return false;

	for (shapesIterator = shapes.begin();
		 shapesIterator != shapes.end();
		 ++shapesIterator)
	{
		ParseNode *pShapesNode = *shapesIterator;

		// Go throug all shape entries in the shapes node
		if (!pShapesNode->FindByName("Shape", shapeNodes))
			continue;
		
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
			argumentMap.GetArgument("Id", strId);
			argumentMap.GetArgument("Type", strType);

			// Filter some types like model filters
			if (strType == "WavefrontModel")
			{
				std::string strFilename;
				argumentMap.GetArgument("Filename", strFilename);

				WavefrontSceneLoader wavefrontLoader(m_pEnvironment);
				wavefrontLoader.Import(strFilename, 0, &argumentMap);

				pShape = m_pEngineKernel->GetShapeManager()->RequestInstance(strId);
			}
			else
			{
				pShape = m_pEngineKernel->GetShapeManager()->CreateInstance(strType, strId, argumentMap);
				std::cout << pShape->ToString() << ", " << pShape->GetName() << std::endl;
			}

			pShape->ComputeBoundingVolume();
			pShape->Compile();
		}
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseRenderers(void)
{
	std::cout << "Parsing Renderers..." << std::endl;

	ArgumentMap argumentMap;

	std::vector<ParseNode*> renderers;
	std::vector<ParseNode*>::iterator renderersIterator;
	
	std::vector<ParseNode*> rendererNodes;
	std::vector<ParseNode*>::iterator rendererNodesIterator;

	std::string strType, strId, 
		strFilter, strDevice, strIntegrator;

	if (!m_parseTree.Root.FindByName("Renderers", renderers))
		return false;

	for (renderersIterator = renderers.begin();
		 renderersIterator != renderers.end();
		 ++renderersIterator)
	{
		ParseNode *pRenderersNode = *renderersIterator;

		// Go throug all shape entries in the shapes node
		if (!pRenderersNode->FindByName("Renderer", rendererNodes))
			continue;
		
		for (rendererNodesIterator = rendererNodes.begin();
			 rendererNodesIterator != rendererNodes.end();
			 ++rendererNodesIterator)
		{
			ParseNode *pRendererNode = *rendererNodesIterator;
			pRendererNode->GetArgumentMap(argumentMap);

			// If argument map does not specify geometry type or name/id, ignore entry
			if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
			{
				std::cout << "[Integrator] Warning :: Ignoring entry because it does not specify Id or Type..." << std::endl;
				continue;
			}

			// Get factory and create instance
			argumentMap.GetArgument("Id", strId);
			argumentMap.GetArgument("Type", strType);

			// Filter some types like model filters
			IRenderer *pRenderer = m_pEngineKernel->GetRendererManager()->CreateInstance(strType, strId, argumentMap);
			std::cout << pRenderer->ToString() << ", " << pRenderer->GetName() << std::endl;

			// Now we need to do some binding operations on the renderer
			std::cout << "Renderer : Performing binding operations..." << std::endl;

			argumentMap.GetArgument("Integrator", strIntegrator);
			IIntegrator *pIntegrator = m_pEngineKernel->GetIntegratorManager()->RequestInstance(strIntegrator);

			argumentMap.GetArgument("Filter", strFilter);
			IFilter* pFilter = m_pEngineKernel->GetFilterManager()->RequestInstance(strFilter);

			argumentMap.GetArgument("Device", strDevice);
			IDevice* pDevice = m_pEngineKernel->GetDeviceManager()->RequestInstance(strDevice);

			pRenderer->SetIntegrator(pIntegrator);
			pRenderer->SetDevice(pDevice);
			pRenderer->SetFilter(pFilter);

			std::cout << "Renderer : Bound [" << strIntegrator << ", " << strDevice << ", " << strFilter << "]" << std::endl;
		}
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseEnvironment(void)
{
	ArgumentMap argumentMap;

	std::string strRendererId, strSamplerId, strCameraId, strSpaceId,
		strMaterialId, strGeometryId, strLightId, strId, strType;

	std::vector<std::string> lightsIdList;

	IRenderer *pRenderer;
	IMaterial *pMaterial;
	ISampler *pSampler;
	ICamera *pCamera;
	IShape *pGeometry;
	ILight *pLight;
	ISpace *pSpace;

	//----------------------------------------------------------------------------------------------
	// Find environment node under root
	std::vector<ParseNode*> environmentNode;
	if (!m_parseTree.Root.FindByName("Environment", environmentNode))
		return false;

	// Read argument map from environment node
	environmentNode[0]->GetArgumentMap(argumentMap);

	//----------------------------------------------------------------------------------------------
	// Read renderer argument and set renderer
	if (!argumentMap.GetArgument("Renderer", strRendererId))
		return false;

	pRenderer = m_pEngineKernel->GetRendererManager()->RequestInstance(strRendererId);
	m_pEnvironment->SetRenderer(pRenderer);

	//----------------------------------------------------------------------------------------------
	// Parse Scene
	std::vector<ParseNode*> sceneNode;
	if (!environmentNode[0]->FindByName("Scene", sceneNode))
	{
		std::cerr << "EnvironmentLoader :: Missing Scene block within Environment block!" << std::endl;
		return false;
	}

	// Instantiate Scene
	Scene *pScene = new Scene();
	m_pEnvironment->SetScene(pScene);

	// We want to read sampler, camera, lights and space
	sceneNode[0]->GetArgumentMap(argumentMap);
	if (!argumentMap.GetArgument("Sampler", strSamplerId)) 
	{
		std::cerr << "EnvironmentLoader :: Missing Sampler within Scene block!" << std::endl;
		return false;
	} 
	else
	{
		pSampler = m_pEngineKernel->GetSamplerManager()->RequestInstance(strSamplerId);
		m_pEnvironment->SetSampler(pSampler);
	}

	if (!argumentMap.GetArgument("Camera", strCameraId))
	{
		std::cerr << "EnvironmentLoader :: Missing Camera within Scene block!" << std::endl;
		return false;
	}
	else
	{
		pCamera = m_pEngineKernel->GetCameraManager()->RequestInstance(strCameraId);
		m_pEnvironment->SetCamera(pCamera);
	}

	if (!argumentMap.GetArgument("Lights", lightsIdList))
	{
		std::cerr << "EnvironmentLoader :: Missing Lights within Scene block!" << std::endl;
		return false;
	}
	else
	{
		pLight = NULL;
		std::vector<std::string>::iterator lightIterator;

		for (lightIterator = lightsIdList.begin(); lightIterator != lightsIdList.end(); ++lightIterator)
		{
			pLight = m_pEngineKernel->GetLightManager()->RequestInstance(*lightIterator);
			pScene->LightList.PushBack(pLight);
		}
	}

	//----------------------------------------------------------------------------------------------
	// Parse Space node
	std::vector<ParseNode*> spaceNode;
	if (!sceneNode[0]->FindByName("Space", spaceNode))
	{
		std::cerr << "EnvironmentLoader :: Missing Scene block within Environment block!" << std::endl;
		return false;
	}

	// Read argument map from space node
	spaceNode[0]->GetArgumentMap(argumentMap);

	//
	// --> Continue here
	//
	pSpace = m_pEngineKernel->GetSpaceManager()->CreateInstance();
	m_pEnvironment->SetSpace(pSpace);

	// Now we parse the space block
	std::cout << "Parsing Primitives..." << std::endl;

	std::vector<ParseNode*> primitives;
	std::vector<ParseNode*> primitiveNodes;
	std::vector<ParseNode*>::iterator primitiveNodesIterator;

	sceneNode[0]->FindByName("Primitives", primitives);
	primitives[0]->FindByName("Primitive", primitiveNodes);
		
	for (primitiveNodesIterator = primitiveNodes.begin();
			primitiveNodesIterator != primitiveNodes.end();
			++primitiveNodesIterator)
	{
		ParseNode *primitiveNode = *primitiveNodesIterator;
		primitiveNode->GetArgumentMap(argumentMap);

		// If argument map does not specify geometry type or name/id, ignore entry
		if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
		{
			std::cout << "[Primitive] Warning :: Ignoring entry because it does not specify Id or Type..." << std::endl;
			continue;
		}

		// Get factory and create instance
		argumentMap.GetArgument("Id", strId);
		argumentMap.GetArgument("Type", strType);

		if (strType == "Emissive")
		{
			argumentMap.GetArgument("Material", strMaterialId);
			pMaterial = m_pEngineKernel->GetMaterialManager()->RequestInstance(strMaterialId);

			argumentMap.GetArgument("Geometry", strGeometryId);
			pGeometry = m_pEngineKernel->GetShapeManager()->RequestInstance(strGeometryId);

			argumentMap.GetArgument("Light", strLightId);
			pLight = m_pEngineKernel->GetLightManager()->RequestInstance(strLightId);

			EmissivePrimitive *pEmissive = new EmissivePrimitive();
			pEmissive->SetMaterial(pMaterial);
			pEmissive->SetShape(pGeometry);
			pEmissive->SetLight((IAreaLight*)pLight);
				
			pSpace->PrimitiveList.PushBack(pEmissive);
		}
		else if (strType == "Geometry")
		{
			argumentMap.GetArgument("Material", strMaterialId);
			pMaterial = m_pEngineKernel->GetMaterialManager()->RequestInstance(strMaterialId);

			argumentMap.GetArgument("Geometry", strGeometryId);
			pGeometry = m_pEngineKernel->GetShapeManager()->RequestInstance(strGeometryId);

			GeometricPrimitive *pGeometric = new GeometricPrimitive();
			pGeometric->SetMaterial(pMaterial);
			pGeometric->SetShape(pGeometry);

			pSpace->PrimitiveList.PushBack(pGeometric);
		}
	}

	std::cout << "Building Space..." << std::endl;
	pSpace->Build();

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
