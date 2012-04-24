//----------------------------------------------------------------------------------------------
//	Filename:	TimeConstrainedRenderer.cpp
//	Author:		Keith Bugeja
//	Date:		27/04/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Renderer/TimeConstrainedRenderer.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
TimeConstrainedRenderer::TimeConstrainedRenderer(const std::string &p_strName, Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, RadianceBuffer *p_pRadianceBuffer, int p_nSampleCount)
	: BaseRenderer(p_strName, p_pScene, p_pIntegrator, p_pDevice, p_pFilter, p_pRadianceBuffer, p_nSampleCount)
{ }
//----------------------------------------------------------------------------------------------
TimeConstrainedRenderer::TimeConstrainedRenderer(Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, RadianceBuffer *p_pRadianceBuffer, int p_nSampleCount)
	: BaseRenderer(p_pScene, p_pIntegrator, p_pDevice, p_pFilter, p_pRadianceBuffer, p_nSampleCount)
{ }
