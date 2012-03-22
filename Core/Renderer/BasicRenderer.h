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
		class BasicRenderer 
			: public IRenderer
		{
		protected:			
			using IRenderer::m_pIntegrator;
			using IRenderer::m_pDevice;
			using IRenderer::m_pFilter;
			using IRenderer::m_pScene;

			int m_nSampleCount;

		public:
			BasicRenderer(const std::string &p_strName, Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL, IDevice *p_pDevice = NULL, IFilter *p_pFilter = NULL, int p_nSampleCount = 1);
			BasicRenderer(Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL, IDevice *p_pDevice = NULL, IFilter *p_pFilter = NULL, int p_nSampleCount = 1);

			void Render(void);
			void RenderRegion(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight);

			void Render(RadianceBuffer *p_pRadianceBuffer, int p_nBufferLeft = 0, int nBufferTop = 0);
			void RenderRegion(int p_nLeft, int p_nTop, int p_nWidth, int p_nHeight, RadianceBuffer *p_pRadianceBuffer, int p_nBufferLeft = 0, int nBufferTop = 0);
		};
	}
}