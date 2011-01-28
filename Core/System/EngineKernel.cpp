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
			RendererManager* m_pRendererManager;
			MaterialManager* m_pMaterialManager;
			TextureManager* m_pTextureManager;
			CameraManager* m_pCameraManager;
			DeviceManager* m_pDeviceManager;
			ShapeManager* m_pShapeManager;
			DummyManager* m_pDummyManager;

			EngineKernelState(EngineKernel* p_pEngineKernel)
				: m_pPlugInManager(new PlugInManager(p_pEngineKernel))
				, m_pRendererManager(new RendererManager())
				, m_pMaterialManager(new MaterialManager())
				, m_pTextureManager(new TextureManager())
				, m_pCameraManager(new CameraManager())
				, m_pDeviceManager(new DeviceManager())
				, m_pShapeManager(new ShapeManager())
				, m_pDummyManager(new DummyManager())
			{ }

			~EngineKernelState(void)
			{
				delete m_pDummyManager;
				delete m_pShapeManager;
				delete m_pCameraManager;
				delete m_pDeviceManager;
				delete m_pMaterialManager;
				delete m_pTextureManager;
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
ShapeManager* EngineKernel::GetShapeManager(void) const
{
	return m_pEngineKernelState->m_pShapeManager;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
DummyManager* EngineKernel::GetDummyManager(void) const
{
	return m_pEngineKernelState->m_pDummyManager;
}