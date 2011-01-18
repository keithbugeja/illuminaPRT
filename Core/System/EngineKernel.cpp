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
			MaterialManager* m_pMaterialManager;
			TextureManager* m_pTextureManager;
			ShapeManager* m_pShapeManager;
			DummyManager* m_pDummyManager;

			EngineKernelState(EngineKernel* p_pEngineKernel)
				: m_pPlugInManager(new PlugInManager(p_pEngineKernel))
				, m_pMaterialManager(new MaterialManager())
				, m_pTextureManager(new TextureManager())
				, m_pShapeManager(new ShapeManager())
				, m_pDummyManager(new DummyManager())
			{ }

			~EngineKernelState(void)
			{
				delete m_pDummyManager;
				delete m_pShapeManager;
				delete m_pMaterialManager;
				delete m_pTextureManager;
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
TextureManager* EngineKernel::GetTextureManager(void) const
{
	return m_pEngineKernelState->m_pTextureManager;
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