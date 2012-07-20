//----------------------------------------------------------------------------------------------
//	Filename:	Device.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "System/FactoryManager.h"
#include "Geometry/Intersection.h"

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
		public:
			enum AccessType
			{
				Read,
				Write,
				ReadWrite
			};

		protected:
			IDevice(const std::string &p_strName) : Object(p_strName) { }
			IDevice(void) { }

		public:
			virtual int GetWidth(void) const = 0;
			virtual int GetHeight(void) const = 0;
			
			virtual AccessType GetAccessType(void) const = 0;

			virtual bool Open(void) = 0;
			virtual void Close(void) = 0;

			virtual void BeginFrame(void) = 0;
			virtual void EndFrame(void) = 0;

			virtual void Set(int p_nX, int p_nY, const Spectrum &p_spectrum) = 0;
			virtual void Set(float p_fX, float p_fY, const Spectrum &p_spectrum) = 0;

			virtual void Get(int p_nX, int p_nY, Spectrum &p_spectrum) const = 0;
			virtual void Get(float p_fX, float p_fY, Spectrum &p_spectrum) const = 0;

			virtual Spectrum Get(int p_nX, int p_nY) const = 0;
			virtual Spectrum Get(float p_fX, float p_fY) const = 0;

			virtual void WriteRadianceBufferToDevice(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, 
				RadianceBuffer *p_pRadianceBuffer, int p_nDeviceX, int p_nDeviceY) = 0;

			std::string ToString(void) const { return "IDevice"; }
		};

		//----------------------------------------------------------------------------------------------
		// DeviceManager : All device classes implementing IDevice should register with DeviceManager.
		//----------------------------------------------------------------------------------------------
		typedef FactoryManager<IDevice> DeviceManager;
	}
}