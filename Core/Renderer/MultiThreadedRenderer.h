//----------------------------------------------------------------------------------------------
//	Filename:	MultithreadedRenderer.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Renderer/Renderer.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class MultithreadedRenderer 
			: public IRenderer
		{
		protected:			
			using IRenderer::m_pIntegrator;
			using IRenderer::m_pDevice;
			using IRenderer::m_pFilter;
			using IRenderer::m_pScene;

			int m_nSampleCount;

		public:
			MultithreadedRenderer(Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL, IDevice *p_pDevice = NULL, IFilter *p_pFilter = NULL, int p_nSampleCount = 1);
			MultithreadedRenderer(const std::string &p_strId, Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL, IDevice *p_pDevice = NULL, IFilter *p_pFilter = NULL, int p_nSampleCount = 1);

			void Render(void);
		};
	}
}