//----------------------------------------------------------------------------------------------
//	Filename:	Renderer.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "System/FactoryManager.h"

#include "Object/Object.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		// IRenderer : Abstract base class for renderers. 
		//----------------------------------------------------------------------------------------------
		class IRenderer 
			: public Object
		{
		protected:
			IIntegrator *m_pIntegrator;
			IDevice *m_pDevice;
			IFilter *m_pFilter;
			Scene *m_pScene;

		protected:
			IRenderer(const std::string &p_strName, Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL,
				IDevice *p_pDevice = NULL, IFilter *p_pFilter = NULL);

			IRenderer(Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL,
				IDevice *p_pDevice = NULL, IFilter *p_pFilter = NULL);

		public:
			virtual bool Initialise(void) { return true; }
			virtual bool Shutdown(void) { return false; }

			virtual void Render(void) = 0;
			virtual void RenderRegion(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight) { };

			virtual void RenderToAuxiliary(int p_nTileX, int p_nTileY, int p_nTileWidth, int p_nTileHeight, Spectrum *p_colourBuffer) = 0;

			void SetIntegrator(IIntegrator *p_pIntegrator);
			IIntegrator* GetIntegrator(void) const;

			void SetDevice(IDevice *p_pDevice);
			IDevice* GetDevice(void) const;

			void SetFilter(IFilter *p_pFilter);
			IFilter* GetFilter(void) const;

			void SetScene(Scene *p_pScene);
			Scene* GetScene(void) const;

			std::string ToString(void) const { return "IRenderer"; }
		};

		//----------------------------------------------------------------------------------------------
		// RendererManager : All Renderer factories must register with object.
		//----------------------------------------------------------------------------------------------
		typedef FactoryManager<IRenderer> RendererManager;
	}
}