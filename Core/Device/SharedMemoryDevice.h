//----------------------------------------------------------------------------------------------
//	Filename:	SharedMemoryDevice.h
//	Author:		Keith Bugeja
//	Date:		12/07/2013
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#if (defined __PLATFORM_WINDOWS__)
#include <boost/interprocess/windows_shared_memory.hpp>
#endif

#include <boost/interprocess/mapped_region.hpp>

//----------------------------------------------------------------------------------------------
#include "Device/Device.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class SharedMemoryDevice 
			: public IDevice
		{
		protected:
			// Image pointers used for double bufferring
			Image *m_pImage[2],
				*m_pFrontBuffer,
				*m_pBackBuffer;

			// Denotes active back buffer
			int m_nActiveBuffer;

			// Device properties
			bool m_bIsStreaming,
				m_bIsOpen;

			// Shared memory tag
			std::string m_strTag;

			#if (defined __PLATFORM_WINDOWS__)
			boost::interprocess::windows_shared_memory
				*m_pSharedMemorySink;
			#else
			boost::interprocess::shared_memory_object
				*m_pSharedMemorySink;
			#endif

			boost::interprocess::mapped_region 
				*m_pSharedMemoryRegion;

		public:
			SharedMemoryDevice(const std::string &p_strName, int p_nWidth, int p_nHeight, const std::string &p_strTag);
			SharedMemoryDevice(int p_nWidth, int p_nHeight, const std::string &p_strTag);

			~SharedMemoryDevice(void);

			//----------------------------------------------------------------------------------------------
			// Interface implementation methods
			//----------------------------------------------------------------------------------------------
			int GetWidth(void) const;
			int GetHeight(void) const;
			
			IDevice::AccessType GetAccessType(void) const;

			bool Open(void);
			void Close(void);

			void BeginFrame(void);
			void EndFrame(void);

			void Set(int p_nX, int p_nY, const Spectrum &p_spectrum);
			void Set(float p_fX, float p_fY, const Spectrum &p_spectrum);

			void Get(int p_nX, int p_nY, Spectrum &p_spectrum) const;
			void Get(float p_fX, float p_fY, Spectrum &p_spectrum) const;

			Spectrum Get(int p_nX, int p_nY) const;
			Spectrum Get(float p_fX, float p_fY) const;

			void WriteRadianceBufferToDevice(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, 
				RadianceBuffer *p_pRadianceBuffer, int p_nDeviceX, int p_nDeviceY);

			//----------------------------------------------------------------------------------------------
			// Type-specific methods
			//----------------------------------------------------------------------------------------------
		public:
			bool IsStreaming(void) const;
			void Stream(void);

			Image *GetImage(void) const;

			const std::string GetTag(void) const;
		};
	}
}