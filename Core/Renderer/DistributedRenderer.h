//----------------------------------------------------------------------------------------------
//	Filename:	DistributedRenderer.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "boost/mpi.hpp"
namespace mpi = boost::mpi;

#include "Renderer/Renderer.h"
#include "Geometry/Vector2.h"
#include "Image/Image.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class DistributedRenderer 
			: public IRenderer
		{
		protected:			
			using IRenderer::m_pIntegrator;
			using IRenderer::m_pDevice;
			using IRenderer::m_pFilter;
			using IRenderer::m_pScene;

			int m_nTileWidth,
				m_nTileHeight;

			int m_nSampleCount;

			// MPI Context information
			mpi::environment* m_pMPIEnvironment;
			mpi::communicator* m_pMPICommunicator;
		
		public:
			DistributedRenderer(Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL, IDevice *p_pDevice = NULL, IFilter *p_pFilter = NULL, 
				int p_nSampleCount = 1, int p_nTileWidth = 8, int p_nTileHeight = 8);

			DistributedRenderer(const std::string &p_strName, Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL, IDevice *p_pDevice = NULL, IFilter *p_pFilter = NULL, 
				int p_nSampleCount = 1, int p_nTileWidth = 8, int p_nTileHeight = 8);

			bool Initialise(void);
			bool Shutdown(void);

			void Render(void);
			void RenderDebug(void);
			void RenderToAuxiliary(int p_nTileX, int p_nTileY, int p_nTileWidth, int p_nTileHeight, Spectrum *p_colourBuffer) { throw new Exception ("Method not implemented!"); }
		};
	}
}