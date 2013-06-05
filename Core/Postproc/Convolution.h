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
			float *m_pfKernel,
				m_fGain;
			int m_nKernelSize,
				m_nKernelWidth;

		public:
			Convolution(void)
				: IPostProcess()
				, m_pKernel(NULL)
				, m_pfKernel(NULL)
				, m_nKernelSize(0)
				, m_nKernelWidth(0)
			{ }

			Convolution(const std::string &p_strName)
				: IPostProcess(p_strName)
				, m_pKernel(NULL)
				, m_pfKernel(NULL)
				, m_nKernelSize(0)
				, m_nKernelWidth(0)
			{ }

			void Reset(void) 
			{ }

			void SetKernel(int p_nKernelSize, int p_nKernelWidth, float *p_pKernel, float p_fGain)
			{
				Safe_Delete(m_pfKernel);

				m_nKernelSize = p_nKernelSize;
				m_nKernelWidth = p_nKernelWidth;
				m_pfKernel = new float[p_nKernelSize];
				memcpy(m_pfKernel, p_pKernel, p_nKernelSize * sizeof(float));

				m_fGain = p_fGain;
			}

			void SetKernel(int p_nKernelSize, int p_nKernelWidth, Spectrum *p_pKernel, float p_fGain)
			{
				Safe_Delete(m_pKernel);

				m_nKernelSize = p_nKernelSize;
				m_nKernelWidth = p_nKernelWidth;
				m_pKernel = new Spectrum[p_nKernelSize];
				memcpy(m_pKernel, p_pKernel, p_nKernelSize * sizeof(Spectrum));

				m_fGain = p_fGain;
			}

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight)
			{
				RadianceContext *pInputContext,
					*pOutputContext,
					*pKernelContext;

				//----------------------------------------------------------------------------------------------
				int border = m_nKernelWidth >> 1;
				int ys, ye, xs, xe;
				float *pKernel;

				//----------------------------------------------------------------------------------------------
				for (int y = p_nRegionY + border; y < p_nRegionHeight - border; ++y)
				{
					ys = y - border;
					ye = y + border;

					for (int x = p_nRegionX + border; x < p_nRegionWidth - border; ++x)
					{
						xs = x - border;
						xe = x + border;

						pInputContext = p_pInput->GetP(x, y);
						pOutputContext = p_pOutput->GetP(x, y);
						
						pOutputContext->Final = 0; 
						pKernel = m_pfKernel;

						for (int dy = ys; dy <= ye; dy++)
						{
							pKernelContext = p_pInput->GetP(xs, dy);

							for (int dx = xs; dx <= xe; dx++)
							{
								pOutputContext->Final += pKernelContext->Final * *pKernel++;
								pKernelContext++;
							}
						}

						// Compute final colour
						pOutputContext->Final *= m_fGain;
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