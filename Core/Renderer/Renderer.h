#pragma once

#include "Staging/Scene.h"
#include "Camera/Camera.h"
#include "Integrator/Integrator.h"
#include "Device/Device.h"

namespace Illumina
{
	namespace Core
	{
		class IRenderer
		{
		public:
			virtual void Render(void) = 0;
		};

		class BasicRenderer : public IRenderer
		{
		protected:			
			Scene *m_pScene;
			ICamera *m_pCamera;
			IIntegrator *m_pIntegrator;
			IDevice *m_pDevice;

		public:
			BasicRenderer(Scene *p_pScene, ICamera *p_pCamera, IIntegrator *p_pIntegrator, IDevice *p_pDevice)
				: m_pScene(p_pScene)
				, m_pCamera(p_pCamera)
				, m_pIntegrator(p_pIntegrator)
				, m_pDevice(p_pDevice)
			{ }

			void Render(void)
			{
				Intersection intersection;

				Ray ray;

				int height = m_pDevice->GetHeight(),
					width = m_pDevice->GetWidth();

				m_pDevice->BeginFrame();

				for (int y = 0; y < height; ++y)
				{
					for (int x = 0; x < width; x++)
					{
						m_pCamera->GetRay(((float)x) / width, ((float)y) / height, 0.5f, 0.5f, ray);
						Spectrum Li = m_pIntegrator->Radiance(m_pScene, ray, intersection);
						m_pDevice->Set(x, y, Li);
					}
				}

				m_pDevice->EndFrame();
			};
		};
	}
}