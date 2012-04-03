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
			IPostProcess(const std::string &p_strName)
				: Object(p_strName) 
			{ }

			IPostProcess(void) { }

		public:
			virtual bool Initialise(void) { return true; }
			virtual bool Shutdown(void) { return false; }

			virtual bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput) = 0;
			virtual bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight) = 0;

			std::string ToString(void) const { return "IPostProcess"; }
		};

		//----------------------------------------------------------------------------------------------
		// PostProcessManager : All PostProcess factories must register with object.
		//----------------------------------------------------------------------------------------------
		typedef FactoryManager<IPostProcess> PostProcessManager;
	}
}