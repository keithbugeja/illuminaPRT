//----------------------------------------------------------------------------------------------
//	Filename:	Renderer.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Renderer/Renderer.h"
#include "Integrator/Integrator.h"
#include "Staging/Scene.h"
#include "Device/Device.h"
#include "Filter/Filter.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
IRenderer::IRenderer(Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter)
	: m_pIntegrator(p_pIntegrator) 
	, m_pDevice(p_pDevice)
	, m_pFilter(p_pFilter)
	, m_pScene(p_pScene)
{ }
//----------------------------------------------------------------------------------------------
IRenderer::IRenderer(const std::string &p_strId, Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter)
	: Object(p_strId) 
	, m_pIntegrator(p_pIntegrator) 
	, m_pDevice(p_pDevice)
	, m_pFilter(p_pFilter)
	, m_pScene(p_pScene)
{ }
//----------------------------------------------------------------------------------------------
void IRenderer::SetIntegrator(IIntegrator *p_pIntegrator) {
	m_pIntegrator = p_pIntegrator;
}
//----------------------------------------------------------------------------------------------
IIntegrator* IRenderer::GetIntegrator(void) const {
	return m_pIntegrator;
}
//----------------------------------------------------------------------------------------------
void IRenderer::SetDevice(IDevice *p_pDevice) {
	m_pDevice = p_pDevice;
}
//----------------------------------------------------------------------------------------------
IDevice* IRenderer::GetDevice(void) const {
	return m_pDevice;
}
//----------------------------------------------------------------------------------------------
void IRenderer::SetFilter(IFilter *p_pFilter) {
	m_pFilter = p_pFilter;
}
//----------------------------------------------------------------------------------------------
IFilter* IRenderer::GetFilter(void) const {
	return m_pFilter;
}
//----------------------------------------------------------------------------------------------
void IRenderer::SetScene(Scene *p_pScene) {
	m_pScene = p_pScene;
}
//----------------------------------------------------------------------------------------------
Scene* IRenderer::GetScene(void) const {
	return m_pScene;
}
//----------------------------------------------------------------------------------------------
