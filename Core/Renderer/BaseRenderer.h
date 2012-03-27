//----------------------------------------------------------------------------------------------
//	Filename:	BaseRenderer.h
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
		class BaseRenderer 
			: public IRenderer
		{
		protected:			
			using IRenderer::m_pRadianceBuffer;
			using IRenderer::m_pIntegrator;
			using IRenderer::m_pDevice;
			using IRenderer::m_pFilter;
			using IRenderer::m_pScene;

			int m_nSampleCount;

		protected:
			BaseRenderer(const std::string &p_strName, Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL, 
				IDevice *p_pDevice = NULL, IFilter *p_pFilter = NULL, RadianceBuffer* p_pRadianceBuffer = NULL, int p_nSampleCount = 1);
			
			BaseRenderer(Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL, IDevice *p_pDevice = NULL, 
				IFilter *p_pFilter = NULL, RadianceBuffer* p_pRadianceBuffer = NULL, int p_nSampleCount = 1);

		public:
			// RadianceBuffer dimensions should equal device dimensions
			void Render(void);
			void Render(RadianceBuffer *p_pRadianceBuffer);
			
			// Render a window-region
			void RenderRegion(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight);
			void RenderRegion(RadianceBuffer *p_pRadianceBuffer, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, int p_nBufferX = 0, int p_nBufferY = 0);

			// Support for non-contiguous tile-rendering
			void RenderTile(RadianceBuffer *p_pRadianceBuffer, int p_nTileIndex, int p_nTileWidth, int p_nTileHeight);

			// Commit radiance buffer to output device
			void Commit(void);
			void Commit(RadianceBuffer *p_pRadianceBuffer);
		};
	}
}