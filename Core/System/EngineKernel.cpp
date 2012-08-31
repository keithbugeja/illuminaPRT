//----------------------------------------------------------------------------------------------
//	Filename:	EngineKernel.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <map>

#include "System/EngineKernel.h"

namespace Illumina
{
	namespace Core
	{
		struct EngineKernelState
		{
			PlugInManager* m_pPlugInManager;
			PostProcessManager *m_pPostProcessManager;
			IntegratorManager* m_pIntegratorManager;
			RendererManager* m_pRendererManager;
			MaterialManager* m_pMaterialManager;
			TextureManager* m_pTextureManager;
			SamplerManager* m_pSamplerManager;
			CameraManager* m_pCameraManager;
			DeviceManager* m_pDeviceManager;
			FilterManager* m_pFilterManager;
			ShapeManager* m_pShapeManager;
			SpaceManager* m_pSpaceManager;
			LightManager* m_pLightManager;
			// DummyManager* m_pDummyManager;

			EngineKernelState(EngineKernel* p_pEngineKernel)
				: m_pPlugInManager(new PlugInManager(p_pEngineKernel))
				, m_pPostProcessManager(new PostProcessManager())
				, m_pIntegratorManager(new IntegratorManager())
				, m_pRendererManager(new RendererManager())
				, m_pMaterialManager(new MaterialManager())
				, m_pTextureManager(new TextureManager())
				, m_pSamplerManager(new SamplerManager())
				, m_pCameraManager(new CameraManager())
				, m_pDeviceManager(new DeviceManager())
				, m_pFilterManager(new FilterManager())
				, m_pShapeManager(new ShapeManager())
				, m_pSpaceManager(new SpaceManager())
				, m_pLightManager(new LightManager())
				// , m_pDummyManager(new DummyManager())
			{ }

			~EngineKernelState(void)
			{
				// delete m_pDummyManager;
				delete m_pLightManager;
				delete m_pSpaceManager;
				delete m_pShapeManager;
				delete m_pFilterManager;
				delete m_pCameraManager;
				delete m_pDeviceManager;
				delete m_pSamplerManager;
				delete m_pMaterialManager;
				delete m_pTextureManager;
				delete m_pIntegratorManager;
				delete m_pPostProcessManager;
				delete m_pRendererManager;
				delete m_pPlugInManager;
			}
		};
	}
}

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
EngineKernel::EngineKernel(void)
	: m_pEngineKernelState(new EngineKernelState(this))
{ }
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
EngineKernel::~EngineKernel(void)
{
	delete m_pEngineKernelState;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
PlugInManager* EngineKernel::GetPlugInManager(void) const
{
	return m_pEngineKernelState->m_pPlugInManager;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
PostProcessManager* EngineKernel::GetPostProcessManager(void) const
{
	return m_pEngineKernelState->m_pPostProcessManager;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
IntegratorManager* EngineKernel::GetIntegratorManager(void) const
{
	return m_pEngineKernelState->m_pIntegratorManager;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
RendererManager* EngineKernel::GetRendererManager(void) const
{
	return m_pEngineKernelState->m_pRendererManager;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
CameraManager* EngineKernel::GetCameraManager(void) const
{
	return m_pEngineKernelState->m_pCameraManager;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
MaterialManager* EngineKernel::GetMaterialManager(void) const
{
	return m_pEngineKernelState->m_pMaterialManager;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
TextureManager* EngineKernel::GetTextureManager(void) const
{
	return m_pEngineKernelState->m_pTextureManager;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
DeviceManager* EngineKernel::GetDeviceManager(void) const
{
	return m_pEngineKernelState->m_pDeviceManager;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
FilterManager* EngineKernel::GetFilterManager(void) const
{
	return m_pEngineKernelState->m_pFilterManager;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
SamplerManager* EngineKernel::GetSamplerManager(void) const
{
	return m_pEngineKernelState->m_pSamplerManager;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
SpaceManager* EngineKernel::GetSpaceManager(void) const
{
	return m_pEngineKernelState->m_pSpaceManager;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
ShapeManager* EngineKernel::GetShapeManager(void) const
{
	return m_pEngineKernelState->m_pShapeManager;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
LightManager* EngineKernel::GetLightManager(void) const
{
	return m_pEngineKernelState->m_pLightManager;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
/*
DummyManager* EngineKernel::GetDummyManager(void) const
{
	return m_pEngineKernelState->m_pDummyManager;
}
 */