#pragma once

#include "Device\Device.h"

namespace Illumina
{
	namespace Core
	{
		class ImageDevice : public IDevice
		{
		protected:
			Image *m_pImage;
			IImageIO *m_pImageIO;
			std::string m_strFilename;

		public:
			ImageDevice(int p_nWidth, int p_nHeight, IImageIO *p_pImageIO, const std::string &p_strFilename)
				: m_pImage(new Image(p_nWidth, p_nHeight))
				, m_pImageIO(p_pImageIO)
				, m_strFilename(p_strFilename)
			{ }

			~ImageDevice()
			{
				delete m_pImage;
			}

			int GetWidth(void) const { return m_pImage->GetWidth(); }
			int GetHeight(void) const { return m_pImage->GetHeight(); }

			void BeginFrame(void)
			{ }

			void EndFrame(void)
			{
				m_pImageIO->Save(*m_pImage, m_strFilename);
			}

			void Set(int p_nX, int p_nY, const Spectrum &p_spectrum)
			{
				RGBPixel pixel(HDRToLDR(p_spectrum[0]), HDRToLDR(p_spectrum[1]), HDRToLDR(p_spectrum[2]));
				m_pImage->Set(p_nX, p_nY, pixel);
			}

			void Set(float p_fX, float p_fY, const Spectrum &p_spectrum)
			{
				Set((int)p_fX, (int)p_fY, p_spectrum); 
			}

		protected:
			float HDRToLDR(float p_fValue)
			{
				return (float)(p_fValue / (p_fValue + 1));
			}
		};
	}
}