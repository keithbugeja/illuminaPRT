//----------------------------------------------------------------------------------------------
//	Filename:	Library.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <string>

namespace Illumina 
{
	namespace Core
	{
		class ILibrary
		{
		public:
			// Load a dynamic library
			virtual bool Load(const std::string& p_strPlugIn) = 0;
	
			// Return a symbol from dynamic library
			virtual void* GetSymbol(const std::string& p_strSymbolName) = 0;
	
			// Unload dynamic library
			virtual void Unload(void) = 0;
		};
	}
}
