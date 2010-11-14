//----------------------------------------------------------------------------------------------
//	Filename:	LibraryImpl.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
#pragma once

#include "Platform.h"
#include "Library.h"

#ifdef __PLATFORM_WINDOWS__
#include <Windows.h>

namespace Illumina 
{
	namespace Core
	{
		class LibraryImplementation : public ILibrary
		{
		protected:
			HMODULE m_hModule;

		public:
			bool Load(const std::string& p_strName)
			{
				return ((m_hModule = LoadLibraryA(p_strName.c_str())) != NULL);
			}

			void* GetSymbol(const std::string& p_strSymbol)
			{
				return (void*)GetProcAddress(m_hModule, p_strSymbol.c_str());
			}

			void Unload(void)
			{
				FreeLibrary(m_hModule);
			}
		};
	}
}
#endif