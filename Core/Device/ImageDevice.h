//----------------------------------------------------------------------------------------------
//	Filename:	ImageDevice.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Device/Device.h"

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

			int GetWidth(void) const;
			int GetHeight(void) const;

			void BeginFrame(void);
			void EndFrame(void);

			void Set(int p_nX, int p_nY, const Spectrum &p_spectrum);
			void Set(float p_fX, float p_fY, const Spectrum &p_spectrum);

			// --> TEMPORARY --- until EGPGV paper is sorted out!
			std::string GetFilename(void) const { return m_strFilename; }
			void SetFilename(const std::string &p_fname) { m_strFilename = p_fname; }
			void WriteToBuffer(RGBBytePixel *p_buffer);
			// --> TEMPORARY --- until EGPGV paper is sorted out!

		protected:
			void ToneMap(void);
			float HDRToLDR(float p_fValue);
		};
	}
}