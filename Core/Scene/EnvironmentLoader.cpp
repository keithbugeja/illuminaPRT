//----------------------------------------------------------------------------------------------
//	Filename:	EnvironmentLoader.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
#include <fstream>
#include <vector>
#include <stack>
#include <map>

#include "Scene/EnvironmentLoader.h"
#include "Scene/Environment.h"
#include "System/Lexer.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
EnvironmentLoader::EnvironmentLoader(EngineKernel *p_pEngineKernel, Environment *p_pEnvironment)
	: ISceneLoader(p_pEngineKernel, p_pEnvironment) 
{ }
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
bool EnvironmentLoader::ParseGeometries(void)
{
	std::cout << "[Geometries]" << std::endl;

	std::vector<std::map<std::string, std::string>> geometryList;
	bool result = ParseList("Geometry", geometryList);
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
			if (!pLexer->ReadToken(LexerToken::LeftCurly, token))
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
