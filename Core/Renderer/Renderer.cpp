//----------------------------------------------------------------------------------------------
//	Filename:	Renderer.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Renderer/Renderer.h"
#include "Integrator/Integrator.h"
#include "Device/Device.h"
#include "Filter/Filter.h"
#include "Scene/Scene.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
IRenderer::IRenderer(Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, 
	IFilter *p_pFilter, RadianceBuffer *p_pRadianceBuffer)
	: m_pRadianceBuffer(p_pRadianceBuffer)
	, m_pIntegrator(p_pIntegrator) 
	, m_pDevice(p_pDevice)
	, m_pFilter(p_pFilter)
	, m_pScene(p_pScene)
{ }
//----------------------------------------------------------------------------------------------
IRenderer::IRenderer(const std::string &p_strName, Scene *p_pScene, IIntegrator *p_pIntegrator, 
	IDevice *p_pDevice, IFilter *p_pFilter, RadianceBuffer *p_pRadianceBuffer)
	: Object(p_strName) 
	, m_pRadianceBuffer(p_pRadianceBuffer)
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
