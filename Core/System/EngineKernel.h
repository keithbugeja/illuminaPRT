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

#include "Material/MaterialManager.h"
#include "Texture/TextureManager.h"

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
			MaterialManager* GetMaterialManager(void) const;
			TextureManager* GetTextureManager(void) const;
			DummyManager* GetDummyManager(void) const;
		};
	}
}