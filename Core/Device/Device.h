//----------------------------------------------------------------------------------------------
//	Filename:	Device.h
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
		// IDevice : Abstract base class for output render devices.
		//----------------------------------------------------------------------------------------------
		class IDevice
			: public Object
		{
		protected:
			IDevice(const std::string &p_strId) : Object(p_strId) { }
			IDevice(void) { }

		public:
			virtual int GetWidth(void) const = 0;
			virtual int GetHeight(void) const = 0;

			virtual void BeginFrame(void) = 0;
			virtual void EndFrame(void) = 0;

			virtual void Set(int p_nX, int p_nY, const Spectrum &p_spectrum) = 0;
			virtual void Set(float p_fX, float p_fY, const Spectrum &p_spectrum) = 0;
		};

		//----------------------------------------------------------------------------------------------
		// DeviceManager : All device classes implementing IDevice should register with DeviceManager.
		//----------------------------------------------------------------------------------------------
		typedef FactoryManager<IDevice> DeviceManager;
	}
}