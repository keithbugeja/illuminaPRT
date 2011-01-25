//----------------------------------------------------------------------------------------------
//	Filename:	BasicRenderer.h
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
		class BasicRenderer : public IRenderer
		{
		protected:			
			using IRenderer::m_pIntegrator;
			using IRenderer::m_pDevice;
			using IRenderer::m_pFilter;
			using IRenderer::m_pScene;

			int m_nSampleCount;

		public:
			BasicRenderer(Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, int p_nSampleCount = 1);

			void Render(void);
		};
	}
}