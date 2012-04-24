//----------------------------------------------------------------------------------------------
//	Filename:	TimeConstrainedRenderer.h
//	Author:		Keith Bugeja
//	Date:		27/04/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Renderer/BaseRenderer.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class TimeConstrainedRenderer 
			: public BaseRenderer
		{
		protected:			
			using IRenderer::m_pRadianceBuffer;
			using IRenderer::m_pIntegrator;
			using IRenderer::m_pDevice;
			using IRenderer::m_pFilter;
			using IRenderer::m_pScene;

			using BaseRenderer::m_nSampleCount;

		public:
			TimeConstrainedRenderer(const std::string &p_strName, Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL, 
				IDevice *p_pDevice = NULL, IFilter *p_pFilter = NULL, RadianceBuffer *p_pRadianceBuffer = NULL, int p_nSampleCount = 1);
			
			TimeConstrainedRenderer(Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL, IDevice *p_pDevice = NULL, 
				IFilter *p_pFilter = NULL, RadianceBuffer *p_pRadianceBuffer = NULL, int p_nSampleCount = 1);
		};
	}
}