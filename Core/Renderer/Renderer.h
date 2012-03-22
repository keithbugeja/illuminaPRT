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
			RadianceBuffer *m_pRadianceBuffer;
			IIntegrator *m_pIntegrator;
			IDevice *m_pDevice;
			IFilter *m_pFilter;
			Scene *m_pScene;

		protected:
			IRenderer(const std::string &p_strName, Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL,
				IDevice *p_pDevice = NULL, IFilter *p_pFilter = NULL, RadianceBuffer *p_pRadianceBuffer = NULL);

			IRenderer(Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL, IDevice *p_pDevice = NULL, 
				IFilter *p_pFilter = NULL, RadianceBuffer *p_pRadianceBuffer = NULL);

		public:
			virtual bool Initialise(void) { return true; }
			virtual bool Shutdown(void) { return false; }

			// Render to radiance buffer (full render)
			// Render to radiance buffer (partial render)

			//virtual void Flush(void) = 0;
			//virtual void FlushRegion(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight) { };

			virtual void Render(void) = 0;
			virtual void RenderRegion(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight) { };

			virtual void Render(RadianceBuffer *p_pRadianceBuffer, int p_nBufferLeft = 0, int nBufferTop = 0) { }
			virtual void RenderRegion(int p_nLeft, int p_nTop, int p_nWidth, int p_nHeight, RadianceBuffer *p_pRadianceBuffer, int p_nBufferLeft = 0, int nBufferTop = 0) { }

			void SetRadianceBuffer(RadianceBuffer *p_pRadianceBuffer);
			RadianceBuffer *GetRadianceBuffer(void) const;

			void SetIntegrator(IIntegrator *p_pIntegrator);
			IIntegrator* GetIntegrator(void) const;

			void SetDevice(IDevice *p_pDevice);
			IDevice* GetDevice(void) const;

			void SetFilter(IFilter *p_pFilter);
			IFilter* GetFilter(void) const;

			void SetScene(Scene *p_pScene);
			Scene* GetScene(void) const;

			std::string ToString(void) const { return "IRenderer"; }

			// -- > Temporary
			virtual void PostProcess(RadianceBuffer *p_pRadianceBuffer) { }
			virtual void PostProcessRegion(RadianceBuffer *p_pRadianceBuffer) { }
			// -- > Temporary
		};

		//----------------------------------------------------------------------------------------------
		// RendererManager : All Renderer factories must register with object.
		//----------------------------------------------------------------------------------------------
		typedef FactoryManager<IRenderer> RendererManager;
	}
}