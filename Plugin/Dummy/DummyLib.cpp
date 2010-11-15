//----------------------------------------------------------------------------------------------
//	Filename:	DummyLib.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "System/Platform.h"
#include "System/LibrarySymbols.h"

#include "DummyPlugIn.h"

//----------------------------------------------------------------------------------------------
// Windows : Include platform stuff
//----------------------------------------------------------------------------------------------
#ifdef __PLATFORM_WINDOWS__
	#include <windows.h>

	#ifndef DLLExport
	#define DLLExport __declspec( dllexport )
	#endif
#endif

using namespace Illumina::PlugIn;

DummyPlugIn* g_pDummyPlugIn = NULL;

extern "C" 
{
	//----------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------
	DLLExport Illumina::Core::IPlugIn* GetPlugInSingleton(void)
	{
		std::cout<<"Returning PlugIn Singleton"<<std::endl;

		if (g_pDummyPlugIn == NULL)
			g_pDummyPlugIn = new DummyPlugIn();

		return g_pDummyPlugIn;
	}

	//----------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------
	DLLExport void DisposePlugInSingleton(void)
	{
		std::cout<<"Disposing PlugIn Singleton"<<std::endl;

		delete g_pDummyPlugIn;
	}

	//----------------------------------------------------------------------------------------------
	// Multi-instance
	//----------------------------------------------------------------------------------------------
	DLLExport Illumina::Core::PlugInHandle CreatePlugInInstance(void)
	{
		return (Illumina::Core::PlugInHandle)NULL;
	}

	//----------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------
	DLLExport Illumina::Core::IPlugIn* GetPlugInInstance(Illumina::Core::PlugInHandle p_hInstance)
	{
		return g_pDummyPlugIn;
	}

	//----------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------
	void DLLExport DisposePlugInInstance(Illumina::Core::PlugInHandle p_hInstance)
	{
	}

	//----------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------
	void DLLExport DisposePlugInInstances()
	{
		delete g_pDummyPlugIn;
	}
}