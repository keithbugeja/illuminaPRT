//----------------------------------------------------------------------------------------------
//	Filename:	ImageDevice.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Device/Device.h"

//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class ImageDevice 
			: public IDevice
		{
		protected:
			Image *m_pImage;
			IImageIO *m_pImageIO;
			std::string m_strFilename;
			bool m_bKillFilterOnExit;

		public:
			ImageDevice(const std::string &p_strName, int p_nWidth, int p_nHeight, IImageIO *p_pImageIO, const std::string &p_strFilename, bool p_bKillFilterOnExit = false);
			ImageDevice(int p_nWidth, int p_nHeight, IImageIO *p_pImageIO, const std::string &p_strFilename, bool p_bKillFilterOnExit = false);
			~ImageDevice(void);

			//----------------------------------------------------------------------------------------------
			// Interface implementation methods
			//----------------------------------------------------------------------------------------------
			int GetWidth(void) const;
			int GetHeight(void) const;
			
			IDevice::AccessType GetAccessType(void) const;

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
			std::string GetFilename(void) const;
			void SetFilename(const std::string &p_strFilename);

			IImageIO *GetImageWriter(void) const;
			void SetImageWriter(IImageIO *p_pImageIO);

			Image *GetImage(void) const;
		};
	}
}