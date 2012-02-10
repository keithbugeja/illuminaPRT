//----------------------------------------------------------------------------------------------
//	Filename:	Sampler.h
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
		// ISampler : Abstract base class for random sample generation methods. 
		//----------------------------------------------------------------------------------------------
		class ISampler
			: public Object
		{
		protected:
			ISampler(const std::string &p_strName) : Object(p_strName) { }
			ISampler(void) { }

		public:
			virtual void Reset(void) = 0;

			virtual void Get2DSamples(Vector2* p_pSamples, int p_nSampleCount) = 0;
			virtual void Get1DSamples(float* p_pSamples, int p_nSampleCount) = 0;
			
			virtual float Get1DSample(void) = 0;
			virtual Vector2 Get2DSample(void) = 0;

			virtual std::string ToString(void) const { return "ISampler"; }
		};

		//----------------------------------------------------------------------------------------------
		// SamplerManager : All Sampler factories must register with object.
		//----------------------------------------------------------------------------------------------
		typedef FactoryManager<ISampler> SamplerManager;
	} 
}