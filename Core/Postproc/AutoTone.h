//----------------------------------------------------------------------------------------------
//	Filename:	AutoTone.h
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
		class AutoTone
			: public IPostProcess
		{
		protected:
			Spectrum m_scaleFactor;
			float m_fBias;

		public:
			AutoTone(void)
				: IPostProcess()
			{ }

			AutoTone(const std::string &p_strName)
				: IPostProcess(p_strName)
			{ }

			void Reset(void) 
			{ }

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, IPostProcess::BlendMode p_eBlendMode = IPostProcess::Replace)
			{
				RadianceContext *pInputContext,
					*pOutputContext;

				Spectrum Lwmax(0.f);

				//----------------------------------------------------------------------------------------------
				Spectrum low(Maths::Maximum), 
					high(0.f), range, Lw(0.f);

				for (int y = p_nRegionY; y < p_nRegionHeight; ++y)
				{
					for (int x = p_nRegionX; x < p_nRegionWidth; ++x)
					{
						pInputContext = p_pInput->GetP(x, y);

						low.Min(low, pInputContext->Final);
						high.Max(high, pInputContext->Final);
					}
				}

				range = high - low;

				for (int y = p_nRegionY; y < p_nRegionHeight; ++y)
				{
					for (int x = p_nRegionX; x < p_nRegionWidth; ++x)
					{
						pInputContext = p_pInput->GetP(x, y);
						pOutputContext = p_pOutput->GetP(x, y);

						pOutputContext->Final = (pInputContext->Final - low) / range;
						pOutputContext->Final.Clamp();
						pOutputContext->Flags |= RadianceContext::DF_ToneMapped;
					}
				}

				return true;
			}

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput, IPostProcess::BlendMode p_eBlendMode = IPostProcess::Replace)
			{
				return Apply(p_pInput, p_pOutput, 0, 0, p_pInput->GetWidth(), p_pInput->GetHeight(), p_eBlendMode);
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