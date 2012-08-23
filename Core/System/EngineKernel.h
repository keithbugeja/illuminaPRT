//----------------------------------------------------------------------------------------------
//	Filename:	EngineKernel.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"

#include "System/PlugInManager.h"
#include "System/FactoryManager.h"
#include "System/DummyManager.h"

#include "Integrator/Integrator.h"
#include "Postproc/PostProcess.h"
#include "Renderer/Renderer.h"
#include "Material/Material.h"
#include "Texture/Texture.h"
#include "Sampler/Sampler.h"
#include "Camera/Camera.h"
#include "Device/Device.h"
#include "Filter/Filter.h"
#include "Space/Space.h"
#include "Shape/Shape.h"
#include "Light/Light.h"

namespace Illumina
{
	namespace Core
	{
		struct EngineKernelState;

		class EngineKernel
		{
		private:
			EngineKernelState *m_pEngineKernelState;

		public:
			EngineKernel();
			~EngineKernel();

			PlugInManager* GetPlugInManager(void) const;
			PostProcessManager* GetPostProcessManager(void) const;
			IntegratorManager* GetIntegratorManager(void) const;
			RendererManager* GetRendererManager(void) const;
			MaterialManager* GetMaterialManager(void) const;
			TextureManager* GetTextureManager(void) const;
			SamplerManager* GetSamplerManager(void) const;
			CameraManager* GetCameraManager(void) const;
			DeviceManager* GetDeviceManager(void) const;
			FilterManager* GetFilterManager(void) const;
			ShapeManager* GetShapeManager(void) const;
			SpaceManager* GetSpaceManager(void) const;
			LightManager* GetLightManager(void) const;
			DummyManager* GetDummyManager(void) const;
		};
	}
}