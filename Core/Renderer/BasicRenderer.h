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
			Scene *m_pScene;

		public:
			BasicRenderer(Scene *p_pScene, ICamera *p_pCamera, IIntegrator *p_pIntegrator, IDevice *p_pDevice);

			void Render(void);
		};
	}
}