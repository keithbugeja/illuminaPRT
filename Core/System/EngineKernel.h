//----------------------------------------------------------------------------------------------
//	Filename:	EngineKernel.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "PlugInManager.h"
#include "FactoryManager.h"

#include "DummyManager.h"

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
			DummyManager* GetDummyManager(void) const;
		};
	}
}