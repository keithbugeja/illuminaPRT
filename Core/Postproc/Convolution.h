//----------------------------------------------------------------------------------------------
//	Filename:	Convolution.h
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
		class Convolution
			: public IPostProcess
		{
		protected:
			Spectrum *m_pKernel;
			int m_nKernelSize;

		public:
			Convolution(void)
				: IPostProcess()
				, m_pKernel(NULL)
				, m_nKernelSize(0)
			{ }

			Convolution(const std::string &p_strName)
				: IPostProcess(p_strName)
				, m_pKernel(NULL)
				, m_nKernelSize(0)
			{ }

			void Reset(void) 
			{ }

			void SetKernel(int p_nKernelSize, Spectrum *p_pKernel)
			{
				Safe_Delete(m_pKernel);

				m_nKernelSize = p_nKernelSize;
				m_pKernel = new Spectrum[p_pKernel];
				memcpy(m_pKernel, p_pKernel, p_nKernelSize * sizeof(Spectrum));
			}

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight)
			{
				RadianceContext *pInputContext,
					*pOutputContext;

				int *offset = (int*)malloc(m_nKernelSize * m_nKernelSize * sizeof(int));

				Spectrum *paddedBuffer = new Spectrum[(p_nRegionHeight * m_nKernelSize) * (p_nRegionWidth + m_nKernelSize)];

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

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput)
			{
				return Apply(p_pInput, p_pOutput, 0, 0, p_pInput->GetWidth(), p_pInput->GetHeight());
			}

			std::string ToString(void) const { return "[Convolution]"; }
		};
	}
}