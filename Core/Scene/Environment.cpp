//----------------------------------------------------------------------------------------------
//	Filename:	Environment.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
#include "Scene/Scene.h"
#include "Scene/Environment.h"
#include "Scene/EnvironmentLoader.h"

#include "Renderer/Renderer.h"

using namespace Illumina::Core;
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
EngineKernel* Environment::GetEngineKernel(void) const {
	return m_pEngineKernel;
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
	EnvironmentLoader loader(this);
	return loader.Import(p_strEnvironmentName, 0);
}
//----------------------------------------------------------------------------------------------
bool Save(const std::string &p_strEnvironmentName)
{
	return true;
}
//----------------------------------------------------------------------------------------------
