//----------------------------------------------------------------------------------------------
//	Filename:	EngineKernel.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <map>

#include "System/EngineKernel.h"
#include "Material/MaterialManager.h"

namespace Illumina
{
	namespace Core
	{
		struct EngineKernelState
		{
			PlugInManager* m_pPlugInManager;
			MaterialManager* m_pMaterialManager;
			DummyManager* m_pDummyManager;

			EngineKernelState(EngineKernel* p_pEngineKernel)
				: m_pPlugInManager(new PlugInManager(p_pEngineKernel))
				, m_pMaterialManager(new MaterialManager())
				, m_pDummyManager(new DummyManager())
			{ }

			~EngineKernelState(void)
			{
				delete m_pDummyManager;
				delete m_pMaterialManager;
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
MaterialManager* EngineKernel::GetMaterialManager(void) const
{
	return m_pEngineKernelState->m_pMaterialManager;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
DummyManager* EngineKernel::GetDummyManager(void) const
{
	return m_pEngineKernelState->m_pDummyManager;
}