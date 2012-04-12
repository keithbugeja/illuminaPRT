//----------------------------------------------------------------------------------------------
//	Filename:	DragoTone.h
//	Author:		Keith Bugeja
//	Date:		27/03/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Postproc/PostProcess.h"

//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------
		class DragoTone
			: public IPostProcess
		{
		protected:
			Spectrum m_scaleFactor;
			float m_fBias;

		public:
			DragoTone(void)
				: IPostProcess()
			{ }

			DragoTone(const std::string &p_strName)
				: IPostProcess(p_strName)
			{ }

			void Reset(void) 
			{ }

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight)
			{
				// -- Temp
				m_fBias = 0.8;
				m_scaleFactor.Set(95.f, 95.f, 95.f);
				// -- Temp

				Spectrum scaleFactor = 
					m_scaleFactor * 0.01f;

				RadianceContext *pInputContext,
					*pOutputContext;

				Spectrum Lwmax(0.f);

				for (int y = p_nRegionY; y < p_nRegionHeight; ++y)
				{
					for (int x = p_nRegionX; x < p_nRegionWidth; ++x)
					{
						pInputContext = p_pInput->GetP(x, y);
						Lwmax.Max(Lwmax, pInputContext->Final);
					}
				}

				float bias = Maths::Log(m_fBias) / Maths::Log(0.5f);

				for (int y = p_nRegionY; y < p_nRegionHeight; ++y)
				{
					for (int x = p_nRegionX; x < p_nRegionWidth; ++x)
					{
						pInputContext = p_pInput->GetP(x, y);
						pOutputContext = p_pOutput->GetP(x, y);

						float r = scaleFactor[0] / (Maths::LogBase(Lwmax[0] + 1, 10)) * Maths::Log(pInputContext->Final[0] + 1) / Maths::Log(2 + Maths::Pow((pInputContext->Final[0] / Lwmax[0]), bias) * 8);
						float g = scaleFactor[1] / (Maths::LogBase(Lwmax[1] + 1, 10)) * Maths::Log(pInputContext->Final[1] + 1) / Maths::Log(2 + Maths::Pow((pInputContext->Final[1] / Lwmax[1]), bias) * 8);
						float b = scaleFactor[2] / (Maths::LogBase(Lwmax[2] + 1, 10)) * Maths::Log(pInputContext->Final[2] + 1) / Maths::Log(2 + Maths::Pow((pInputContext->Final[2] / Lwmax[2]), bias) * 8);

						pOutputContext->Final.Set(r, g, b);
						pOutputContext->Final.Clamp();
						pOutputContext->Flag = 1;
					}
				}

				return true;
			}

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput)
			{
				return Apply(p_pInput, p_pOutput, 0, 0, p_pInput->GetWidth(), p_pInput->GetHeight());
			}

			std::string ToString(void) const { return "[AutoTone]"; }
		};

		//----------------------------------------------------------------------------------------------
		// Original ImageDevice Tone mapping code				
		//----------------------------------------------------------------------------------------------

		/*
		void ImageDevice::ToneMap(void)
		{
			RGBPixel Lw(0), Ld;

			for (int y = 0; y < m_pImage->GetHeight(); y++)
			{
				for (int x = 0; x < m_pImage->GetWidth(); x++)
				{
					RGBPixel pixel = m_pImage->Get(x,y);

					Lw.R += Maths::Log(pixel.R + Maths::Epsilon);
					Lw.G += Maths::Log(pixel.G + Maths::Epsilon);
					Lw.B += Maths::Log(pixel.B + Maths::Epsilon);
				}
			}

			Lw.R = Maths::Exp(1 / (m_pImage->GetWidth() * m_pImage->GetHeight()) * Lw.R);
			Lw.G = Maths::Exp(1 / (m_pImage->GetWidth() * m_pImage->GetHeight()) * Lw.G);
			Lw.B = Maths::Exp(1 / (m_pImage->GetWidth() * m_pImage->GetHeight()) * Lw.B);

			for (int y = 0; y < m_pImage->GetHeight(); y++)
			{
				for (int x = 0; x < m_pImage->GetWidth(); x++)
				{
					Ld = m_pImage->Get(x,y);

					Ld.R = HDRToLDR(Ld.R * (0.18f / Lw.R));
					Ld.G = HDRToLDR(Ld.G * (0.18f / Lw.G));
					Ld.B = HDRToLDR(Ld.B * (0.18f / Lw.B));
			
					//Ld.R = HDRToLDR(Ld.R);
					//Ld.G = HDRToLDR(Ld.G);
					//Ld.B = HDRToLDR(Ld.B);
			
					m_pImage->Set(x,y, Ld);
				}
			}

		}

		float ImageDevice::HDRToLDR(float p_fValue)
		{
			return (float)(p_fValue / (p_fValue + 1));
		}
		*/
		//----------------------------------------------------------------------------------------------
	}
}