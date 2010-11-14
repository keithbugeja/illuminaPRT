//----------------------------------------------------------------------------------------------
//	Filename:	PlugIn.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Version.h"

namespace Illumina 
{
	namespace Core
	{
		class EngineKernel;

		class IPlugIn
		{
		public:
			// Returns the plug-in name
			virtual const std::string& GetName(void) const = 0;

			// Returns the plug-in version
			virtual const Version& GetVersion(void) const = 0;

			// Registers the plug-in with its respective engine
			virtual void Register(EngineKernel* p_pEngineKernel) = 0;
	
			// Initialises the plug-in
			virtual void Initialise(void) = 0;

			// Shuts down the plug-in
			virtual void Shutdown(void) = 0;

			// Unregisters the plug-in from its respective engine
			virtual void Unregister(EngineKernel* p_pEngineKernel) = 0;
		};
	}
}