//----------------------------------------------------------------------------------------------
//	Filename:	Environment.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
#include <fstream>
#include <vector>
#include <stack>
#include <map>

#include "Scene/Environment.h"
#include "Staging/Scene.h"
#include "Renderer/Renderer.h"
#include "System/Lexer.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class EnvironmentLoader
		{
		protected:
			EngineKernel *m_pEngineKernel;
			Environment *m_pEnvironment;
			std::stack<Lexer*> m_lexerStack;

		protected:
			void Push(Lexer *p_pLexer)
			{
				m_lexerStack.push(p_pLexer);
			}

			Lexer* Pop(void)
			{
				Lexer *temp = Top();
				m_lexerStack.pop();

				return temp;
			}

			Lexer* Top(void)
			{
				return m_lexerStack.size() == 0 ? NULL : m_lexerStack.top();
			}

		public:
			EnvironmentLoader(EngineKernel *p_pEngineKernel, Environment *p_pEnvironment)
				: m_pEngineKernel(p_pEngineKernel)
				, m_pEnvironment(p_pEnvironment)
			{ }

			bool Load(const std::string p_strFilename)
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

			bool Parse(void)
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
	
		protected:
			bool ParseInclude(void)
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

			bool ParseList(const std::string& p_strContainerName, std::vector<std::map<std::string, std::string>> p_containerList)
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

			bool ParseCameras(void)
			{
				std::cout << "[Cameras]" << std::endl;

				std::vector<std::map<std::string, std::string>> cameraList;
				bool result = ParseList("Camera", cameraList);
				return result;
			}
	
			bool ParseLights(void)
			{
				std::cout << "[Lights]" << std::endl;

				std::vector<std::map<std::string, std::string>> lightList;
				bool result = ParseList("Light", lightList);
				return result;
			}

			bool ParseFilters(void)
			{
				std::cout << "[Filters]" << std::endl;

				std::vector<std::map<std::string, std::string>> filterList;
				bool result = ParseList("Filter", filterList);
				return result;
			}
	
			bool ParseDevices(void)
			{
				std::cout << "[Devices]" << std::endl;

				std::vector<std::map<std::string, std::string>> deviceList;
				bool result = ParseList("Device", deviceList);
				return result;
			}

			bool ParseSamplers(void)
			{
				std::cout << "[Samplers]" << std::endl;

				std::vector<std::map<std::string, std::string>> samplerList;
				bool result = ParseList("Sampler", samplerList);
				return result;
			}

			bool ParseIntegrators(void)
			{
				std::cout << "[Integrators]" << std::endl;

				std::vector<std::map<std::string, std::string>> integratorList;
				bool result = ParseList("Integrator", integratorList);
				return result;
			}

			bool ParseRenderers(void)
			{
				std::cout << "[Renderers]" << std::endl;

				std::vector<std::map<std::string, std::string>> rendererList;
				bool result = ParseList("Renderer", rendererList);
				return result;
			}

			bool ParseGeometries(void)
			{
				std::cout << "[Geometries]" << std::endl;

				std::vector<std::map<std::string, std::string>> geometryList;
				bool result = ParseList("Geometry", geometryList);
				return result;
			}

			bool ParseMaterials(void)
			{
				std::cout << "[Materials]" << std::endl;

				std::vector<std::map<std::string, std::string>> materialList;
				bool result = ParseList("Material", materialList);
				return result;
			}

			bool ParseEnvironment(void)
			{
				return true;
			}
		};
	}
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
Environment::Environment(EngineKernel *p_pEngineKernel, IRenderer *p_pRenderer)
	: m_pEngineKernel(p_pEngineKernel)
	, m_pRenderer(p_pRenderer)
	, m_bIsInitialised(false)
{ }
//----------------------------------------------------------------------------------------------
bool Environment::IsInitialised(void) const {
	return m_bIsInitialised;
}
//----------------------------------------------------------------------------------------------
bool Environment::Initialise(void) 
{
	m_bIsInitialised = true;
	return true;
}
//----------------------------------------------------------------------------------------------
void Environment::Shutdown(void) {
	m_bIsInitialised = false;
}
//----------------------------------------------------------------------------------------------
void Environment::SetIntegrator(IIntegrator *p_pIntegrator) {
	m_pRenderer->SetIntegrator(p_pIntegrator);
}
//----------------------------------------------------------------------------------------------
IIntegrator* Environment::GetIntegrator(void) const {
	return m_pRenderer->GetIntegrator();
}
//----------------------------------------------------------------------------------------------
void Environment::SetSampler(ISampler *p_pSampler) {
	m_pRenderer->GetScene()->SetSampler(p_pSampler);
}
//----------------------------------------------------------------------------------------------
ISampler* Environment::GetSampler(void) const {
	return m_pRenderer->GetScene()->GetSampler();
}
//----------------------------------------------------------------------------------------------
void Environment::SetFilter(IFilter *p_pFilter) {
	m_pRenderer->SetFilter(p_pFilter);
}
//----------------------------------------------------------------------------------------------
IFilter* Environment::GetFilter(void) const {
	return m_pRenderer->GetFilter();
}
//----------------------------------------------------------------------------------------------
void Environment::SetDevice(IDevice *p_pDevice) {
	m_pRenderer->SetDevice(p_pDevice);
}
//----------------------------------------------------------------------------------------------
IDevice* Environment::GetDevice(void) const {
	return m_pRenderer->GetDevice();
}
//----------------------------------------------------------------------------------------------
void Environment::SetCamera(ICamera *p_pCamera) {
	m_pRenderer->GetScene()->SetCamera(p_pCamera);
}
//----------------------------------------------------------------------------------------------
ICamera* Environment::GetCamera(void) const {
	return m_pRenderer->GetScene()->GetCamera();
}
//----------------------------------------------------------------------------------------------
void Environment::SetSpace(ISpace *p_pSpace) {
	m_pRenderer->GetScene()->SetSpace(p_pSpace);
}
//----------------------------------------------------------------------------------------------
ISpace* Environment::GetSpace(void) const {
	return m_pRenderer->GetScene()->GetSpace();
}
//----------------------------------------------------------------------------------------------
void Environment::SetScene(Scene *p_pScene) {
	m_pRenderer->SetScene(p_pScene);
}
//----------------------------------------------------------------------------------------------
Scene* Environment::GetScene(void) const {
	return m_pRenderer->GetScene();
}
//----------------------------------------------------------------------------------------------
void Environment::SetRenderer(IRenderer *p_pRenderer) {
	m_pRenderer = p_pRenderer;
}
//----------------------------------------------------------------------------------------------
IRenderer* Environment::GetRenderer(void) const {
	return m_pRenderer;
}
//----------------------------------------------------------------------------------------------
bool Environment::Load(const std::string &p_strEnvironmentName)
{
	EnvironmentLoader loader(m_pEngineKernel, this);
	loader.Load(p_strEnvironmentName);

	return true;
}
bool Save(const std::string &p_strEnvironmentName)
{
	return true;
}