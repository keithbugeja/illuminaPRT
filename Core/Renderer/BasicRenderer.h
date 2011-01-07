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
			IIntegrator *m_pIntegrator;
			ICamera *m_pCamera;
			IDevice *m_pDevice;
			IFilter *m_pFilter;
			Scene *m_pScene;

			int m_nSampleCount;

		public:
			BasicRenderer(Scene *p_pScene, ICamera *p_pCamera, IIntegrator *p_pIntegrator, IDevice *p_pDevice, IFilter *p_pFilter, int p_nSampleCount = 1);

			void Render(void);
		};
	}
}