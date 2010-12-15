//----------------------------------------------------------------------------------------------
//	Filename:	BasicRenderer.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Renderer/BasicRenderer.h"
#include "Integrator/Integrator.h"
#include "Geometry/Intersection.h"
#include "Spectrum/Spectrum.h"
#include "Camera/Camera.h"
#include "Device/Device.h"
#include "Staging/Scene.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
BasicRenderer::BasicRenderer(Scene *p_pScene, ICamera *p_pCamera, IIntegrator *p_pIntegrator, IDevice *p_pDevice)
	: m_pScene(p_pScene)
	, m_pCamera(p_pCamera)
	, m_pIntegrator(p_pIntegrator)
	, m_pDevice(p_pDevice)
{ }
//----------------------------------------------------------------------------------------------
void BasicRenderer::Render(void)
{
	Intersection intersection;

	int height = m_pDevice->GetHeight(),
		width = m_pDevice->GetWidth();

	m_pDevice->BeginFrame();

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; x++)
		{
			Ray ray = m_pCamera->GetRay(((float)x) / width, ((float)y) / height, 0.5f, 0.5f);
			Spectrum Li = m_pIntegrator->Radiance(m_pScene, ray, intersection);
			m_pDevice->Set(x, height - (y + 1), Li);
		}
	}

	m_pDevice->EndFrame();
}
//----------------------------------------------------------------------------------------------