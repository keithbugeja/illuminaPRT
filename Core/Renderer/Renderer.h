//----------------------------------------------------------------------------------------------
//	Filename:	Renderer.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class IRenderer
		{
		protected:
			IIntegrator *m_pIntegrator;
			IDevice *m_pDevice;
			IFilter *m_pFilter;
			Scene *m_pScene;

		protected:
			IRenderer(Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL,
				IDevice *p_pDevice = NULL, IFilter *p_pFilter = NULL);

		public:
			virtual bool Initialise(void) { return true; }
			virtual bool Shutdown(void) { return false; }

			virtual void Render(void) = 0;

			void SetIntegrator(IIntegrator *p_pIntegrator);
			IIntegrator* GetIntegrator(void) const;

			void SetDevice(IDevice *p_pDevice);
			IDevice* GetDevice(void) const;

			void SetFilter(IFilter *p_pFilter);
			IFilter* GetFilter(void) const;

			void SetScene(Scene *p_pScene);
			Scene* GetScene(void) const;
		};
	}
}