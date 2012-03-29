//----------------------------------------------------------------------------------------------
//	Filename:	PostProcess.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "System/FactoryManager.h"

#include "Object/Object.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		// IPostProcess : Abstract base class for post-processes. 
		//----------------------------------------------------------------------------------------------
		class IPostProcess 
			: public Object
		{
		protected:

		public:
			virtual bool Initialise(void) { return true; }
			virtual bool Shutdown(void) { return false; }

			virtual bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput) = 0;

			std::string ToString(void) const { return "IPostProcess"; }
		};

		//----------------------------------------------------------------------------------------------
		// PostProcessManager : All PostProcess factories must register with object.
		//----------------------------------------------------------------------------------------------
		typedef FactoryManager<IPostProcess> PostProcessManager;
	}
}