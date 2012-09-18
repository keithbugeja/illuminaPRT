//----------------------------------------------------------------------------------------------
//	Filename:	DistributedRenderer.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
/*
#pragma once

#include "boost/mpi.hpp"
namespace mpi = boost::mpi;

#include "Renderer/BaseRenderer.h"
#include "Geometry/Vector2.h"
#include "Image/Image.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class DistributedRenderer 
			: public BaseRenderer
		{
		protected:			
			using IRenderer::m_pIntegrator;
			using IRenderer::m_pDevice;
			using IRenderer::m_pFilter;
			using IRenderer::m_pScene;

			using BaseRenderer::m_nSampleCount;

			int m_nTileWidth,
				m_nTileHeight;

			// MPI Context information
			mpi::environment* m_pMPIEnvironment;
			mpi::communicator* m_pMPICommunicator;
		
		public:
			DistributedRenderer(Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL, IDevice *p_pDevice = NULL, IFilter *p_pFilter = NULL, 
				RadianceBuffer *p_pRadianceBuffer = NULL, int p_nSampleCount = 1, int p_nTileWidth = 8, int p_nTileHeight = 8);

			DistributedRenderer(const std::string &p_strName, Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL, IDevice *p_pDevice = NULL, IFilter *p_pFilter = NULL, 
				RadianceBuffer *p_pRadianceBuffer = NULL, int p_nSampleCount = 1, int p_nTileWidth = 8, int p_nTileHeight = 8);

			bool Initialise(void);
			bool Shutdown(void);
			
			void Render(void);
			void RenderDebug(void);
			
		};
	}
}
*/