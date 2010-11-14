//----------------------------------------------------------------------------------------------
//	Filename:	PlugInManager.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "PlugInManager.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
/// PlugInManager Constructor
///	\param p_pEngineKernel Pointer to an Engine Kernel object, to which plug-ins can attach.
//----------------------------------------------------------------------------------------------
PlugInManager::PlugInManager(EngineKernel* p_pEngineKernel)
	: m_pEngineKernel(p_pEngineKernel)
{ 
	BOOST_ASSERT(p_pEngineKernel != NULL);
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
void PlugInManager::AddPlugIn(IPlugIn *p_pPlugIn)
{
	if (ContainsPlugIn(p_pPlugIn))
		throw new Exception("Plug-In Manager already contains an instance of Plug-In!");

	m_plugInArray.push_back(p_pPlugIn);
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
void PlugInManager::RemovePlugIn(IPlugIn *p_pPlugIn)
{
	std::vector<IPlugIn*>::iterator plugInIterator = FindPlugIn(p_pPlugIn);
	
	if (plugInIterator == m_plugInArray.end())
		throw new Exception("Plug-In does is not registered with Plug-In Manager!");
	
	m_plugInArray.erase(plugInIterator);
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
bool PlugInManager::ContainsPlugIn(IPlugIn *p_pPlugIn)
{
	return (FindPlugIn(p_pPlugIn) != m_plugInArray.end());
}
//----------------------------------------------------------------------------------------------
std::vector<IPlugIn*>::iterator PlugInManager::FindPlugIn(IPlugIn *p_pPlugIn)
{
	return std::find(m_plugInArray.begin(), m_plugInArray.end(), p_pPlugIn);
}
//----------------------------------------------------------------------------------------------
/// Loads a plug-in contained in a dynamic system library.
///	\param p_strLibrary Name of dynamic library file
/// \return Pointer to plug-in instance
//----------------------------------------------------------------------------------------------
IPlugIn* PlugInManager::Load(const std::string& p_strLibrary)
{
	IPlugIn* pPlugIn = NULL;

	if (NULL == m_libraryManager.Load(p_strLibrary) ||
		NULL == (pPlugIn = m_libraryManager.GetPlugInSingleton(p_strLibrary)))
		return NULL;

	Register(pPlugIn);
	return pPlugIn;
}

//----------------------------------------------------------------------------------------------
/// Unloads a loaded plug-in.
///	\param p_strLibrary Name of dynamic library file
//----------------------------------------------------------------------------------------------
void PlugInManager::Unload(const std::string& p_strLibrary)
{
	IPlugIn* pPlugIn = m_libraryManager.GetPlugInSingleton(p_strLibrary);
	
	if (pPlugIn != NULL)
	{
		Unregister(pPlugIn);
		m_libraryManager.DisposePlugInSingleton(p_strLibrary);
		m_libraryManager.Unload(p_strLibrary);
	}
}

//----------------------------------------------------------------------------------------------
/// Registers a static plug-in with the plug-in manager.
///	\param p_pPlugIn Pointer to plug-in instance
//----------------------------------------------------------------------------------------------
void PlugInManager::Register(IPlugIn* p_pPlugIn)
{
	if (p_pPlugIn != NULL)
	{
		AddPlugIn(p_pPlugIn);
		p_pPlugIn->Register(m_pEngineKernel);
	}
}

//----------------------------------------------------------------------------------------------
///	Unregisters a plug-in from the plug-in manager.
/// \param p_strName Name of plug-in to unregister
//----------------------------------------------------------------------------------------------
void PlugInManager::Unregister(IPlugIn* p_pPlugIn)
{
	if (p_pPlugIn != NULL)
	{
		RemovePlugIn(p_pPlugIn);
		p_pPlugIn->Unregister(m_pEngineKernel);
	}
}

//----------------------------------------------------------------------------------------------
///	Performs initialisation of specified plug-in.
///	\param p_strName Name of plug-in to initialise
//----------------------------------------------------------------------------------------------
void PlugInManager::Initialise(IPlugIn* p_pPlugIn)
{
	if (p_pPlugIn != NULL)
		p_pPlugIn->Initialise();
}

//----------------------------------------------------------------------------------------------
///	Shutdown the specified plug-in.
///	\param p_strName Name of plug-in to shutdown
//----------------------------------------------------------------------------------------------
void PlugInManager::Shutdown(IPlugIn* p_pPlugIn)
{
	if (p_pPlugIn != NULL)
		p_pPlugIn->Shutdown();
}