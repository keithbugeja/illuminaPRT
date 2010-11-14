//----------------------------------------------------------------------------------------------
//	Filename:	LibrarySymbols.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

namespace Illumina
{
	namespace Core
	{
		class IPlugIn;

		typedef unsigned long PlugInHandle;

		typedef IPlugIn*(*SYMBOL_SINGLE_GET)(void);
		typedef void(*SYMBOL_SINGLE_DISPOSE)(void);

		typedef PlugInHandle(*SYMBOL_MULTI_CREATE)(void);
		typedef IPlugIn*(*SYMBOL_MULTI_GET)(PlugInHandle);
		typedef void(*SYMBOL_MULTI_DISPOSE)(PlugInHandle);
		typedef void(*SYMBOL_MULTI_DISPOSEALL)(void);
	}
}
