//----------------------------------------------------------------------------------------------
//	Filename:	Environment.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
#include "boost/spirit/include/qi.hpp"

#include "Scene/Environment.h"
#include "Staging/Scene.h"
#include "Renderer/Renderer.h"

using namespace Illumina::Core;

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

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
bool Environment::Initialise(std::string *p_pCameraArgs, 
	std::string *p_pRendererArgs,
	std::string *p_pIntegratorArgs,
	std::string *p_pFilterArgs,
	std::string *p_pDeviceArgs,
	std::string *p_pSpaceArgs,
	std::string *p_pSamplerArgs)
{
	


	// Instantiatiable objects always have id + type
	// Include can be called at root only

	// Need to parse:
	// Include, Camera, Light, Material, Geometry, Sampler, Integrator, Filter, Device, Environment, Renderer, Scene, Primitive, Transform

	// Lexemes
	// Literals w/o quotes = Keywords
	// { } = block / multiple parameter string
	// '=' defines key, value pair
	// ',' separates multiple parameters
	

	// identifier = char + (char | digit)*
	// string = '"' + (char | digit)* + '"'
	// vector4 = '{' + double + ',' + double + ',' + double + ',' + double + '}'
	// vector3 = '{' + double + ',' + double + ',' + double + '}'
	// vector2 = '{' + double + ',' + double + ',' + '}'



	// [Camera:Type] Params -> ArgumentMap
	// [Integrator:Type] Params
	// [Sampler:Type] Params
	// [Filter:Type] Params
	// [Renderer:Type] Params
	// [Device:Type] Params
	// [Space:Type] Params

	m_bIsInitialised = true;
	return true;
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
