//----------------------------------------------------------------------------------------------
//	Filename:	MedianFilter.h
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
		class MedianFilter
			: public IPostProcess
		{
		protected:
			Spectrum *m_pKernel;
			float *m_pfKernel,
				m_fGain;
			int m_nKernelSize,
				m_nKernelWidth;

		public:
			MedianFilter(void)
				: IPostProcess()
			{ }

			MedianFilter(const std::string &p_strName)
				: IPostProcess(p_strName)
			{ }

			void Reset(void) 
			{ }
			
			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, IPostProcess::BlendMode p_eBlendMode = IPostProcess::Replace)
			{
				RadianceContext *pInputContext,
					*pOutputContext,
					*pKernelContext;

				int idx[] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
				
				idx[0] -= p_nRegionWidth;
				idx[1] -= p_nRegionWidth;
				idx[2] -= p_nRegionWidth;
				idx[6] += p_nRegionWidth;
				idx[7] += p_nRegionWidth;
				idx[8] += p_nRegionWidth;

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

						for (int s = 0; s < 9; s++)
						{
							
						}

						pInputContext = p_pInput->GetP(x, y);
						pOutputContext = p_pOutput->GetP(x, y);
						
						// Compute final colour
						pOutputContext->Final *= m_fGain;
					}
				}

				return true;
			}

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput, IPostProcess::BlendMode p_eBlendMode = IPostProcess::Replace)
			{
				return Apply(p_pInput, p_pOutput, 0, 0, p_pInput->GetWidth(), p_pInput->GetHeight(), p_eBlendMode);
			}

			std::string ToString(void) const { return "[MedianFilter]"; }
		};
	}
}