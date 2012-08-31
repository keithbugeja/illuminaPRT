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
#include "Scene/ParticleSceneLoader.h"
#include "Scene/Environment.h"

#include "Scene/EmissivePrimitive.h"
#include "Scene/GeometricPrimitive.h"

#include "Light/InfiniteAreaLight.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
ParseNode::ParseNode(const std::string &p_strName, const std::string &p_strValue)
	: Type(ParseNode::Leaf)
    , Name(p_strName)
	, Value(p_strValue)
{ }
//----------------------------------------------------------------------------------------------
ParseNode::ParseNode(const std::string &p_strName)
	: Type(ParseNode::Internal)
    , Name(p_strName)
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

	// Perform lexical analysis on current stream
	while(pLexer->ReadToken(nameToken))
	{
		//std::cout << nameToken.Value << " ";
		if (nameToken.Type == LexerToken::PropertyName)
		{
			if (!pLexer->ReadToken(valueToken))
				return false;
			
			//std::cout << valueToken.Value << " ";

			// Go deeper
			if (valueToken.Type == LexerToken::LeftCurly)
			{
				ParseNode *pNode = m_parseTree.RequestNode(nameToken.Value);
				nodeStack.top()->Children.push_back(pNode);
				nodeStack.push(pNode);
			}
			else
			{
				ParseNode *pNode = m_parseTree.RequestNode(nameToken.Value, valueToken.Value);
				nodeStack.top()->Children.push_back(pNode);
			}
		}
		else if (nameToken.Type == LexerToken::RightCurly)
		{
			nodeStack.pop();
		}
	}

	// Parse sections of generated parse-tree
	if (!ParseTextures()) std::cerr << "Texture Parsing :: Completed with warnings!" << std::endl;
	if (!ParseShapes()) std::cerr << "Shape Parsing :: Completed with warnings!" << std::endl;
	if (!ParseMaterials()) std::cerr << "Material Parsing :: Completed with warnings!" << std::endl;
	if (!ParseLights()) std::cerr << "Light Parsing :: Completed with warnings!" << std::endl;
	if (!ParseCameras()) std::cerr << "ParseCameras :: Completed with warnings!" << std::endl;
	if (!ParseFilters()) std::cerr << "Filter Parsing :: Completed with warnings!" << std::endl;
	if (!ParseSamplers()) std::cerr << "Sampler Parsing:: Completed with warnings!" << std::endl;
	if (!ParseDevices()) std::cerr << "Device Parsing :: Completed with warnings!" << std::endl;
	if (!ParseIntegrators()) std::cerr << "Integrator Parsing :: Completed with warnings!" << std::endl;
	if (!ParseRenderers()) std::cerr << "Renderer Parsing :: Completed with warnings!" << std::endl;
	if (!ParseEnvironment()) std::cerr << "Environment Parsing :: Completed with warnings!" << std::endl;

	// Release parse tree nodes
	m_parseTree.ReleaseNodes();

	return true;
}

//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::GetNodeList(const std::string &p_strCategoryName, const std::string &p_strInstanceName, std::vector<ParseNode*> &p_nodeList, ParseNode *p_pNode)
{
	std::vector<ParseNode*> categoryNodes;
	std::vector<ParseNode*>::iterator categoryIterator;
	
	std::vector<ParseNode*> instanceNodes;
	std::vector<ParseNode*>::iterator instanceIterator;

	if (p_pNode == NULL) p_pNode = &m_parseTree.Root;
	if (!p_pNode->FindByName(p_strCategoryName, categoryNodes))
		return false;

	p_nodeList.clear();

	for (categoryIterator = categoryNodes.begin();
		 categoryIterator != categoryNodes.end();
		 ++categoryIterator)
	{
		ParseNode *pCategoryNode = *categoryIterator;

		if (!pCategoryNode->FindByName(p_strInstanceName, instanceNodes))
			continue;

		p_nodeList.insert(p_nodeList.end(), instanceNodes.begin(), instanceNodes.end());
	}

	return (p_nodeList.size() != 0);
}

//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseCameras(void)
{
	ICamera *pCamera;
	ArgumentMap argumentMap;
	std::string strType, strId;
	
	std::vector<ParseNode*> cameraNodes;
	std::vector<ParseNode*>::iterator cameraNodesIterator;

	if (!GetNodeList("Cameras", "Camera", cameraNodes))
		return false;

	for (cameraNodesIterator = cameraNodes.begin();
			cameraNodesIterator != cameraNodes.end();
			++cameraNodesIterator)
	{
		ParseNode *pCameraNode = *cameraNodesIterator;
		pCameraNode->GetArgumentMap(argumentMap);

		// If argument map does not specify geometry type or name/id, ignore entry
		if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
		{
			std::cerr << "[Camera] Warning : Ignoring Shape entry because it does not specify Id or Type..." << std::endl;
			continue;
		}

		// Get shape factory and create instance
		argumentMap.GetArgument("Id", strId);
		argumentMap.GetArgument("Type", strType);

		// Filter some types like model filters
		try { pCamera = m_pEngineKernel->GetCameraManager()->CreateInstance(strType, strId, argumentMap); }
		catch (...) { std::cerr << "[Camera] Error : Cannot create instance." << std::endl; }
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseTextures(void)
{
	ITexture *pTexture;
	ArgumentMap argumentMap;
	std::string strType, strId;

	std::vector<ParseNode*> textureNodes;
	std::vector<ParseNode*>::iterator textureNodesIterator;

	if (!GetNodeList("Textures", "Texture", textureNodes))
		return false;

	for (textureNodesIterator = textureNodes.begin();
			textureNodesIterator != textureNodes.end();
			++textureNodesIterator)
	{
		ParseNode *pTextureNode = *textureNodesIterator;
		pTextureNode->GetArgumentMap(argumentMap);

		// If argument map does not specify geometry type or name/id, ignore entry
		if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
		{
			std::cerr << "[Texture] Warning : Ignoring entry because it does not specify Id or Type..." << std::endl;
			continue;
		}

		// Get shape factory and create instance
		argumentMap.GetArgument("Id", strId);
		argumentMap.GetArgument("Type", strType);

		// Try creating instance
		try { pTexture = m_pEngineKernel->GetTextureManager()->CreateInstance(strType, strId, argumentMap); }
		catch (...) { std::cerr << "[Texture] Error : Cannot create instance." << std::endl; }
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseLights(void)
{
	ILight *pLight;
	ArgumentMap argumentMap;
	std::string strType, strId;

	std::vector<ParseNode*> lightNodes;
	std::vector<ParseNode*>::iterator lightNodesIterator;

	if (!GetNodeList("Lights", "Light", lightNodes))
		return false;

	for (lightNodesIterator = lightNodes.begin();
			lightNodesIterator != lightNodes.end();
			++lightNodesIterator)
	{
		ParseNode *pLightNode = *lightNodesIterator;
		pLightNode->GetArgumentMap(argumentMap);

		// If argument map does not specify geometry type or name/id, ignore entry
		if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
		{
			std::cerr << "[Light] Warning : Ignoring entry because it does not specify Id or Type..." << std::endl;
			continue;
		}

		// Get shape factory and create instance
		argumentMap.GetArgument("Id", strId);
		argumentMap.GetArgument("Type", strType);

		// Try creating instance
		try { pLight = m_pEngineKernel->GetLightManager()->CreateInstance(strType, strId, argumentMap); }
		catch (...) { std::cerr << "[Light] Error : Cannot create instance." << std::endl; }

		// We have an arealight derived class
		try { if (argumentMap.GetArgument("Shape", strId)) ((IAreaLight*)pLight)->SetShape(m_pEngineKernel->GetShapeManager()->RequestInstance(strId)); } 
		catch (...) { std::cerr << "[Light] Error : Cannot assign shape instance to area light." << std::endl; }

		// We have an infinite area light
		try { if (argumentMap.GetArgument("Texture", strId)) ((InfiniteAreaLight*)pLight)->SetTexture(m_pEngineKernel->GetTextureManager()->RequestInstance(strId)); }
		catch (...) { std::cerr << "[Light] Error : Cannot assign texture instance to light." << std::endl; }
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseFilters(void)
{
	IFilter *pFilter;
	ArgumentMap argumentMap;
	std::string strType, strId;
	
	std::vector<ParseNode*> filterNodes;
	std::vector<ParseNode*>::iterator filterNodesIterator;

	if (!GetNodeList("Filters", "Filter", filterNodes))
		return false;

	for (filterNodesIterator = filterNodes.begin();
			filterNodesIterator != filterNodes.end();
			++filterNodesIterator)
	{
		ParseNode *pFilterNode = *filterNodesIterator;
		pFilterNode->GetArgumentMap(argumentMap);

		// If argument map does not specify geometry type or name/id, ignore entry
		if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
		{
			std::cerr << "[Filter] Warning : Ignoring entry because it does not specify Id or Type..." << std::endl;
			continue;
		}

		// Get factory and create instance
		argumentMap.GetArgument("Id", strId);
		argumentMap.GetArgument("Type", strType);

		// Try creating instance
		try { pFilter = m_pEngineKernel->GetFilterManager()->CreateInstance(strType, strId, argumentMap); } 
		catch (...) { std::cerr << "[Filter] Error : Cannot create instance." << std::endl; }
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseSamplers(void)
{
	ISampler *pSampler;
	ArgumentMap argumentMap;
	std::string strType, strId;

	std::vector<ParseNode*> samplerNodes;
	std::vector<ParseNode*>::iterator samplerNodesIterator;

	if (!GetNodeList("Samplers", "Sampler", samplerNodes))
		return false;

	for (samplerNodesIterator = samplerNodes.begin();
			samplerNodesIterator != samplerNodes.end();
			++samplerNodesIterator)
	{
		ParseNode *pSamplerNode = *samplerNodesIterator;
		pSamplerNode->GetArgumentMap(argumentMap);

		// If argument map does not specify geometry type or name/id, ignore entry
		if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
		{
			std::cerr << "[Sampler] Warning : Ignoring entry because it does not specify Id or Type..." << std::endl;
			continue;
		}

		// Get factory and create instance
		argumentMap.GetArgument("Id", strId);
		argumentMap.GetArgument("Type", strType);

		// Try creating instance
		try { pSampler = m_pEngineKernel->GetSamplerManager()->CreateInstance(strType, strId, argumentMap); } 
		catch (...) { std::cerr << "[Sampler] Error : Cannot create instance." << std::endl; }
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseDevices(void)
{
	IDevice *pDevice;
	ArgumentMap argumentMap;
	std::string strType, strId;
	
	std::vector<ParseNode*> deviceNodes;
	std::vector<ParseNode*>::iterator deviceNodesIterator;

	if (!GetNodeList("Devices", "Device", deviceNodes))
		return false;

	for (deviceNodesIterator = deviceNodes.begin();
			deviceNodesIterator != deviceNodes.end();
			++deviceNodesIterator)
	{
		ParseNode *pDeviceNode = *deviceNodesIterator;
		pDeviceNode->GetArgumentMap(argumentMap);

		// If argument map does not specify geometry type or name/id, ignore entry
		if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
		{
			std::cerr << "[Device] Warning : Ignoring entry because it does not specify Id or Type..." << std::endl;
			continue;
		}

		// Get factory and create instance
		argumentMap.GetArgument("Id", strId);
		argumentMap.GetArgument("Type", strType);

		// Try creating instance
		try { pDevice = m_pEngineKernel->GetDeviceManager()->CreateInstance(strType, strId, argumentMap); } 
		catch (...) { std::cerr << "[Device] Error : Cannot create instance." << std::endl; }
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseIntegrators(void)
{
	ArgumentMap argumentMap;
	IIntegrator *pIntegrator;
	std::string strType, strId;

	std::vector<ParseNode*> integratorNodes;
	std::vector<ParseNode*>::iterator integratorNodesIterator;

	if (!GetNodeList("Integrators", "Integrator", integratorNodes))
		return false;

	for (integratorNodesIterator = integratorNodes.begin();
			integratorNodesIterator != integratorNodes.end();
			++integratorNodesIterator)
	{
		ParseNode *pIntegratorNode = *integratorNodesIterator;
		pIntegratorNode->GetArgumentMap(argumentMap);

		// If argument map does not specify geometry type or name/id, ignore entry
		if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
		{
			std::cerr << "[Integrator] Warning : Ignoring entry because it does not specify Id or Type..." << std::endl;
			continue;
		}

		// Get factory and create instance
		argumentMap.GetArgument("Id", strId);
		argumentMap.GetArgument("Type", strType);

		// Filter some types like model filters
		try { pIntegrator = m_pEngineKernel->GetIntegratorManager()->CreateInstance(strType, strId, argumentMap); } 
		catch (...) { std::cerr << "[Integrator] Error : Cannot create instance." << std::endl; }
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseMaterials(void)
{
	IMaterial *pMaterial;
	ArgumentMap argumentMap;
	std::string strType, strId;

	std::vector<ParseNode*> materialNodes;
	std::vector<ParseNode*>::iterator materialNodesIterator;

	if (!GetNodeList("Materials", "Material", materialNodes))
		return false;
		
	for (materialNodesIterator = materialNodes.begin();
			materialNodesIterator != materialNodes.end();
			++materialNodesIterator)
	{
		ParseNode *pMaterialNode = *materialNodesIterator;
		pMaterialNode->GetArgumentMap(argumentMap);

		// If argument map does not specify geometry type or name/id, ignore entry
		if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
		{
			std::cerr << "[Material] Warning : Ignoring entry because it does not specify Id or Type..." << std::endl;
			continue;
		}

		// Get factory and create instance
		argumentMap.GetArgument("Id", strId);
		argumentMap.GetArgument("Type", strType);

		// Filter some types like model filters
		try { pMaterial = m_pEngineKernel->GetMaterialManager()->CreateInstance(strType, strId, argumentMap); } 
		catch (...) { std::cerr << "[Material] Error : Cannot create instance." << std::endl; }
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseShapes(void)
{
	IShape *pShape;
	ArgumentMap argumentMap;
	std::string strType, strId;

	std::vector<ParseNode*> shapeNodes;
	std::vector<ParseNode*>::iterator shapeNodesIterator;

	// No shape nodes found
	if (!GetNodeList("Shapes", "Shape", shapeNodes))
		return false;
		
	for (shapeNodesIterator = shapeNodes.begin();
			shapeNodesIterator != shapeNodes.end();
			++shapeNodesIterator)
	{
		ParseNode *pShapeNode = *shapeNodesIterator;
		pShapeNode->GetArgumentMap(argumentMap);

		// If argument map does not specify geometry type or name/id, ignore entry
		if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
		{
			std::cerr << "[Shape] Warning : Ignoring Shape entry because it does not specify Id or Type..." << std::endl;
			continue;
		}

		// Get shape factory and create instance
		argumentMap.GetArgument("Id", strId);
		argumentMap.GetArgument("Type", strType);

		// Filter some types like model filters
		try 
		{ 
			if (strType == "WavefrontModel")
			{
				std::string strFilename;
				argumentMap.GetArgument("Filename", strFilename);
				boost::trim(strFilename);

				WavefrontSceneLoader wavefrontLoader(m_pEnvironment);
				wavefrontLoader.Import(strFilename, 0, &argumentMap);

				pShape = m_pEngineKernel->GetShapeManager()->RequestInstance(strId);
			}
			else if (strType == "ParticleModel")
			{
				std::string strFilename;
				argumentMap.GetArgument("Filename", strFilename);
				boost::trim(strFilename);

				ParticleSceneLoader particleLoader(m_pEnvironment);
				particleLoader.Import(strFilename, 0, &argumentMap);

				pShape = m_pEngineKernel->GetShapeManager()->RequestInstance(strId);
			}
			else
			{
				pShape = m_pEngineKernel->GetShapeManager()->CreateInstance(strType, strId, argumentMap);
			}

			// Compute volume bounds and compile mesh (if supported)
			pShape->ComputeBoundingVolume();
			pShape->Compile();
		} 
		catch (...) { std::cerr << "[Shape] Error : Cannot create instance." << std::endl; }
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool EnvironmentLoader::ParseRenderers(void)
{
	IRenderer *pRenderer;
	ArgumentMap argumentMap;
	
	std::string strType, strId, 
		strFilter, strDevice, 
		strIntegrator;

	std::vector<ParseNode*> rendererNodes;
	std::vector<ParseNode*>::iterator rendererNodesIterator;

	if (!GetNodeList("Renderers", "Renderer", rendererNodes))
		return false;
		
	for (rendererNodesIterator = rendererNodes.begin();
			rendererNodesIterator != rendererNodes.end();
			++rendererNodesIterator)
	{
		ParseNode *pRendererNode = *rendererNodesIterator;
		pRendererNode->GetArgumentMap(argumentMap);

		// If argument map does not specify geometry type or name/id, ignore entry
		if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
		{
			std::cout << "[Renderer] Warning : Ignoring entry because it does not specify Id or Type..." << std::endl;
			continue;
		}

		// Get factory and create instance
		argumentMap.GetArgument("Id", strId);
		argumentMap.GetArgument("Type", strType);

		// Filter some types like model filters
		try { pRenderer = m_pEngineKernel->GetRendererManager()->CreateInstance(strType, strId, argumentMap); }
		catch (...) { std::cerr << "[Renderer] Error : Cannot create instance." << std::endl; }

		/*
		 * Now we need to do some binding operations on the renderer
		 */

		try {
			// Bind integrator
			argumentMap.GetArgument("Integrator", strIntegrator);
			IIntegrator *pIntegrator = m_pEngineKernel->GetIntegratorManager()->RequestInstance(strIntegrator);
			pRenderer->SetIntegrator(pIntegrator);
		} catch (...) { std::cerr << "[Renderer] Error : Unable to bind integrator [" << strIntegrator << "]" << std::endl; }

		try {
			// Bind filter
			argumentMap.GetArgument("Filter", strFilter);
			IFilter* pFilter = m_pEngineKernel->GetFilterManager()->RequestInstance(strFilter);
			pRenderer->SetFilter(pFilter);
		} catch (...) { std::cerr << "[Renderer] Error : Unable to bind filter [" << strFilter << "]" << std::endl; }

		try {
			// Bind device
			argumentMap.GetArgument("Device", strDevice);
			IDevice* pDevice = m_pEngineKernel->GetDeviceManager()->RequestInstance(strDevice);
			pRenderer->SetDevice(pDevice);
		} catch (...) { std::cerr << "[Renderer] Error : Unable to bind device [" << strDevice << "]" << std::endl; }
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

	if (!argumentMap.GetArgument("Id", strSpaceId))
		return false;

	if (!argumentMap.GetArgument("Type", strType))
		return false;

	pSpace = m_pEngineKernel->GetSpaceManager()->CreateInstance(strType, strSpaceId, argumentMap);
	m_pEnvironment->SetSpace(pSpace);

	// Now we parse the space block
	std::vector<ParseNode*> primitives;
	std::vector<ParseNode*> primitiveNodes;
	std::vector<ParseNode*>::iterator primitiveNodesIterator;

	if (!spaceNode[0]->FindByName("Primitives", primitives))
		return false;

	if (!primitives[0]->FindByName("Primitive", primitiveNodes))
		return false;
		
	for (primitiveNodesIterator = primitiveNodes.begin();
			primitiveNodesIterator != primitiveNodes.end();
			++primitiveNodesIterator)
	{
		ParseNode *primitiveNode = *primitiveNodesIterator;
		primitiveNode->GetArgumentMap(argumentMap);

		// If argument map does not specify geometry type or name/id, ignore entry
		if (!(argumentMap.ContainsArgument("Id") && argumentMap.ContainsArgument("Type")))
		{
			std::cerr << "[Primitive] Warning :: Ignoring entry because it does not specify Id or Type..." << std::endl;
			continue;
		}

		// Get factory and create instance
		argumentMap.GetArgument("Id", strId);
		argumentMap.GetArgument("Type", strType);

		if (strType == "Emissive")
		{
			argumentMap.GetArgument("Material", strMaterialId);
			pMaterial = m_pEngineKernel->GetMaterialManager()->RequestInstance(strMaterialId);

			argumentMap.GetArgument("Shape", strGeometryId);
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

			argumentMap.GetArgument("Shape", strGeometryId);
			pGeometry = m_pEngineKernel->GetShapeManager()->RequestInstance(strGeometryId);

			GeometricPrimitive *pGeometric = new GeometricPrimitive();
			pGeometric->SetMaterial(pMaterial);
			pGeometric->SetShape(pGeometry);

			pSpace->PrimitiveList.PushBack(pGeometric);
		}
	}

	// Initialse and build the space structure
	pSpace->Initialise();
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
