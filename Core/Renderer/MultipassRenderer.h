//----------------------------------------------------------------------------------------------
//	Filename:	MultithreadedRenderer.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	To Do: 
//	 Generalise shading and discontinuity processes
//	 Allow for a pipeline of post-process effects
//----------------------------------------------------------------------------------------------
#pragma once

#include "Renderer/Renderer.h"
#include "Integrator/Integrator.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class MultipassRenderer 
			: public IRenderer
		{
		protected:			
			using IRenderer::m_pIntegrator;
			using IRenderer::m_pDevice;
			using IRenderer::m_pFilter;
			using IRenderer::m_pScene;

			bool m_bUseCombinedPass;

			int m_nWidth, 
				m_nHeight;
			
			int m_nSampleCount;

			int m_nDBSize;

			float m_fDBCos,
				m_fDBDist;

		protected:
			Intersection *m_pGeometryBuffer;

		public:
			MultipassRenderer(const std::string &p_strName, Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL, IDevice *p_pDevice = NULL, IFilter *p_pFilter = NULL, int p_nSampleCount = 1, bool p_bCombined = true, int p_nDBSize = 3, float p_fDBDist = 10.f, float p_fDBCos = 0.75f);
			MultipassRenderer(Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL, IDevice *p_pDevice = NULL, IFilter *p_pFilter = NULL, int p_nSampleCount = 1, bool p_bCombined = true, int p_nDBSize = 3, float p_fDBDist = 10.f, float p_fDBCos = 0.75f);

			//MultipassRenderer(const std::string &p_strName, Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL, IDevice *p_pDevice = NULL, IFilter *p_pFilter = NULL, int p_nSampleCount = 1);
			//MultipassRenderer(Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL, IDevice *p_pDevice = NULL, IFilter *p_pFilter = NULL, int p_nSampleCount = 1);

			bool Initialise(void);
			bool Shutdown(void);

			void Render(void);
			void RenderToAuxiliary(int p_nTileX, int p_nTileY, int p_nTileWidth, int p_nTileHeight, Spectrum *p_colourBuffer);
		
		protected:
			void ComputeIntersectionPass(Intersection *p_pGeometryBuffer, int p_nTileX, int p_nTileY, int p_nTileWidth, int p_nTileHeight);
			void ComputeShadingPass(Intersection *p_pGeometryBuffer, int p_nTileX, int p_nTileY, int p_nTileWidth, int p_nTileHeight);
			void ComputeCombinedPass(Intersection *p_pGeometryBuffer, int p_nTileX, int p_nTileY, int p_nTileWidth, int p_nTileHeight);
			void ComputeDirectPass(IntegratorContext *p_pContext, Scene *p_pScene, Intersection &p_intersection); 
		};
	}
}