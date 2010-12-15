//----------------------------------------------------------------------------------------------
//	Filename:	LibraryManager.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <map>

#include "System/IlluminaPRT.h"
#include "System/LibrarySymbols.h"
#include "System/Library.h"
#include "System/PlugIn.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core 
	{
		class LibraryManager
		{
		private:
			std::map<std::string, ILibrary*> m_libraryMap;

			ILibrary* CreateLibrary();

		public:
			bool Load(const std::string& p_strName);
			void Unload(const std::string& p_strName);

			// Used for singleton plugins
			IPlugIn* GetPlugInSingleton(const std::string& p_strName);
			void DisposePlugInSingleton(const std::string& p_strName);

			// Used for multi-instance plugins
			PlugInHandle CreatePlugInInstance(const std::string& p_strName);
			IPlugIn* GetPlugInInstance(const std::string& p_strName, PlugInHandle p_hInstance);
			void DisposePlugInInstance(const std::string& p_strName, PlugInHandle p_hInstance);
			void DisposePlugInInstances(const std::string& p_strName);
		};
	}
}
