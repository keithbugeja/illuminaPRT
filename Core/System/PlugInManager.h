//----------------------------------------------------------------------------------------------
//	Filename:	PlugInManager.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <vector>

#include "System/IlluminaPRT.h"
#include "System/LibraryManager.h"
#include "Exception/Exception.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class PlugInManager
		{
		private:
			std::vector<IPlugIn*> m_plugInArray;
			LibraryManager m_libraryManager;
			EngineKernel* m_pEngineKernel;

		protected:
			// Manager protected methods for adding, removing,
			// seeking and testing containment.
			void AddPlugIn(IPlugIn *p_pPlugIn);
			void RemovePlugIn(IPlugIn *p_pPlugIn);
			bool ContainsPlugIn(IPlugIn *p_pPlugIn);
			std::vector<IPlugIn*>::iterator FindPlugIn(IPlugIn *p_pPlugIn);

		public:
			PlugInManager(EngineKernel *p_pEngineKernel);

			// Dynamic library loading/unloading mechanism
			IPlugIn *Load(const std::string& p_strLibrary);
			void Unload(const std::string& p_strLibrary);

			// Static library loading/unloading mechanism
			void Register(IPlugIn *p_pPlugIn);
			void Unregister(IPlugIn *p_pPlugIn);

			// Initalisation
			void Initialise(IPlugIn *p_pPlugIn);
			void Shutdown(IPlugIn *p_pPlugIn);
		};
	}
}