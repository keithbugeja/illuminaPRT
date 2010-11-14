//----------------------------------------------------------------------------------------------
//	Filename:	LibraryManager.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "LibraryImpl.h"
#include "LibraryManager.h"
#include "Exception\Exception.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
/// Factory for concrete implementations of ILibrary.
/// \returns Pointer to the newly created instance.
//----------------------------------------------------------------------------------------------
ILibrary* LibraryManager::CreateLibrary()
{
	#ifdef __PLATFORM_WINDOWS__
		return new LibraryImplementation();
	#else			
		throw new Exception("No Library System defined for platform!", __FILE__, __LINE__);
	#endif
}

//----------------------------------------------------------------------------------------------
///	Loads the specified dynamic library.
/// \param p_strName Dynamic library file
/// \return <b>true</b> if module has been successfully loaded, <b>false</b> otherwise
//----------------------------------------------------------------------------------------------
bool LibraryManager::Load(const std::string& p_strName)
{
	ILibrary* pLibrary = CreateLibrary();
	
	if (!pLibrary->Load(p_strName))
		return false;

	m_libraryMap[p_strName] = pLibrary; // .insert(p_strName, pLibrary);
	return true;
}

//----------------------------------------------------------------------------------------------
/// Unloads the specified dynamic library
/// \param p_strName Dynamic library file
//----------------------------------------------------------------------------------------------
void LibraryManager::Unload(const std::string& p_strName)
{
	if (m_libraryMap.find(p_strName) != m_libraryMap.end())
	{
		ILibrary* pLibrary = m_libraryMap[p_strName];
		m_libraryMap.erase(p_strName);
		delete pLibrary;
	}
}

//----------------------------------------------------------------------------------------------
/// Returns the <b>singleton</b> instance to the PlugIn loaded by the specified 
///	dynamic library. 
///	\param p_strName Dynamic library file
/// \return Pointer to PlugIn instance
//----------------------------------------------------------------------------------------------
Illumina::Core::IPlugIn* LibraryManager::GetPlugInSingleton(const std::string& p_strName)
{
	if (m_libraryMap.find(p_strName) == m_libraryMap.end())
		return NULL;

	ILibrary* pLibrary = m_libraryMap[p_strName];

	SYMBOL_SINGLE_GET GetPlugInSingleton = (SYMBOL_SINGLE_GET)pLibrary->GetSymbol("GetPlugInSingleton");
	BOOST_ASSERT(GetPlugInSingleton != NULL);
	
	Illumina::Core::IPlugIn* pPlugIn = GetPlugInSingleton();
	return pPlugIn;
}

//----------------------------------------------------------------------------------------------
///	Disposes of the PlugIn singleton instance within the specified dynamic library.
/// \param p_strName Dynamic library file
//----------------------------------------------------------------------------------------------
void LibraryManager::DisposePlugInSingleton(const std::string& p_strName)
{
	if (m_libraryMap.find(p_strName) != m_libraryMap.end())
	{
		ILibrary* pLibrary = m_libraryMap[p_strName];

		SYMBOL_SINGLE_DISPOSE DisposePlugInSingleton = (SYMBOL_SINGLE_DISPOSE)pLibrary->GetSymbol("DisposePlugInSingleton");
		BOOST_ASSERT(DisposePlugInSingleton != NULL);
		
		DisposePlugInSingleton();
	}
}

//----------------------------------------------------------------------------------------------
/// Creates a new PlugIn instance.
/// \param p_strName Dynamic library file
/// \return Handle to newly created instance
//----------------------------------------------------------------------------------------------
PlugInHandle LibraryManager::CreatePlugInInstance(const std::string& p_strName)
{
	if (m_libraryMap.find(p_strName) == m_libraryMap.end())
		return NULL;

	ILibrary* pLibrary = m_libraryMap[p_strName];
	
	SYMBOL_MULTI_CREATE CreatePlugInInstance = (SYMBOL_MULTI_CREATE)pLibrary->GetSymbol("CreatePlugInInstance");
	BOOST_ASSERT(CreatePlugInInstance != NULL);
	
	return CreatePlugInInstance();
}

//----------------------------------------------------------------------------------------------
/// Retrieves a PlugIn instance by the respective handle.
/// \param p_hInstance Handle to instance
/// \param p_strName Dynamic library file
/// \return Pointer to PlugIn object
//----------------------------------------------------------------------------------------------
IPlugIn* LibraryManager::GetPlugInInstance(const std::string& p_strName, PlugInHandle p_hInstance)
{
	if (m_libraryMap.find(p_strName) == m_libraryMap.end())
		return NULL;

	ILibrary* pLibrary = m_libraryMap[p_strName];

	SYMBOL_MULTI_GET GetPlugInInstance = (SYMBOL_MULTI_GET)pLibrary->GetSymbol("GetPlugInInstance");
	BOOST_ASSERT(GetPlugInInstance != NULL);
	
	return GetPlugInInstance( p_hInstance );
}

//----------------------------------------------------------------------------------------------
/// Disposes of a given PlugIn instance through the respective handle.
/// \param p_strName Dynamic library file
/// \param p_hInstance Handle to instance
//----------------------------------------------------------------------------------------------
void LibraryManager::DisposePlugInInstance(const std::string& p_strName, PlugInHandle p_hInstance)
{
	if (m_libraryMap.find(p_strName) != m_libraryMap.end())
	{
		ILibrary* pLibrary = m_libraryMap[p_strName];
	
		SYMBOL_MULTI_DISPOSE DisposePlugInInstance = (SYMBOL_MULTI_DISPOSE)pLibrary->GetSymbol("DisposePlugInInstance");
		BOOST_ASSERT(DisposePlugInInstance != NULL);
		
		DisposePlugInInstance( p_hInstance );
	}
}

//----------------------------------------------------------------------------------------------
/// Disposes of all PlugIn instances.
/// \param p_strName Dynamic library file
//----------------------------------------------------------------------------------------------
void LibraryManager::DisposePlugInInstances(const std::string& p_strName)
{
	if (m_libraryMap.find(p_strName) != m_libraryMap.end())
	{
		ILibrary* pLibrary = m_libraryMap[p_strName];
	
		SYMBOL_MULTI_DISPOSEALL DisposePlugInInstances = (SYMBOL_MULTI_DISPOSEALL)pLibrary->GetSymbol("DisposePlugInInstances");
		BOOST_ASSERT(DisposePlugInInstances != NULL);
		
		DisposePlugInInstances();
	}
}