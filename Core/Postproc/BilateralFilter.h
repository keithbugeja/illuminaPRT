//----------------------------------------------------------------------------------------------
//	Filename:	BilateralFilter.h
//	Author:		Keith Bugeja
//	Date:		27/03/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Postproc/PostProcess.h"
#include "Maths/Statistics.h"

//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------
		class BilateralFilter
			: public IPostProcess
		{
		protected:
			int m_nKernelSize;
			
		public:
			BilateralFilter(int p_nKernelSize = 3, float p_fAngle = 0.75f, float p_fDistance = 10.0f)
				: IPostProcess()
				, m_nKernelSize(p_nKernelSize)
			{ }

			BilateralFilter(const std::string &p_strName, int p_nKernelSize = 3, float p_fAngle = 0.75f, float p_fDistance = 10.0f)
				: IPostProcess(p_strName)
				, m_nKernelSize(p_nKernelSize)
			{ }

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight)
			{
				RadianceContext *pKernelContext,
					*pNeighbourContext,
					*pOutputContext;

				Spectrum intensity;

				int ys, ye, xs, xe;
				
				float weight, 
					intensityWeight, 
					distanceWeight;

				float sigmaS = 4.0f,
					sigmaR = 0.5f;

				m_nKernelSize = 3;

				float mean_s_PDF = Statistics::GaussianPDF(0, 0, sigmaS),
					edge_s_PDF = Statistics::GaussianPDF(sigmaS, 0, sigmaS),
					interval_s_PDF = mean_s_PDF - edge_s_PDF,
					invSigmaS = 1.f / sigmaS;

				/*
				float mean_r_PDF = Statistics::GaussianPDF(0, 0, sigmaR),
					edge_r_PDF = Statistics::GaussianPDF(sigmaR, 0, sigmaR),
					interval_r_PDF = mean_r_PDF - edge_r_PDF,
					invSigmaR = 1.f / sigmaR;
				*/

				//----------------------------------------------------------------------------------------------
				for (int y = p_nRegionY + m_nKernelSize; y < p_nRegionHeight - m_nKernelSize; ++y)
				{
					ys = y - m_nKernelSize;
					ye = y + m_nKernelSize;

					for (int x = p_nRegionX + m_nKernelSize; x < p_nRegionWidth - m_nKernelSize; ++x)
					{
						xs = x - m_nKernelSize;
						xe = x + m_nKernelSize;

						weight = 0; intensity = 0;

						pKernelContext = p_pInput->GetP(x, y);
						pOutputContext = p_pOutput->GetP(x, y);

						for (int dy = ys; dy < ye; dy++)
						{
							pNeighbourContext = p_pInput->GetP(xs, dy);

							for (int dx = xs; dx < xe; dx++)
							{
								distanceWeight = pKernelContext->Distance - pNeighbourContext->Distance;
								
								/* Gaussian */
								//distanceWeight = Statistics::GaussianPDF(distanceWeight * distanceWeight);

								/* Gaussian approximation */
								distanceWeight *= distanceWeight;
								distanceWeight = distanceWeight > sigmaS ? 0.f : interval_s_PDF * (distanceWeight * invSigmaS) + mean_s_PDF;

								/*
								intensityWeight = (pKernelContext->Indirect[0] + pKernelContext->Indirect[1] + pKernelContext->Indirect[2]) * 0.33f - 
									(pNeighbourContext->Indirect[0] + pNeighbourContext->Indirect[1] + pNeighbourContext->Indirect[2]) * 0.33f; 
								intensityWeight *= intensityWeight;
								intensityWeight = intensityWeight > sigmaR ? 0.f : interval_r_PDF * (intensityWeight * invSigmaR) + mean_r_PDF;
								*/

								/*
								weight += distanceWeight * intensityWeight;
								intensity += pNeighbourContext->Indirect * distanceWeight * intensityWeight;
								*/

								weight += distanceWeight;
								intensity += pNeighbourContext->Indirect * distanceWeight;

								pNeighbourContext++;
							}
						}

						if (weight > Maths::Epsilon)
						{
							pOutputContext->Indirect = intensity / weight;
							pOutputContext->Final = pKernelContext->Direct + pOutputContext->Indirect * pKernelContext->Albedo;
						}
					}
				}

				return true;
			}

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput)
			{
				return Apply(p_pInput, p_pOutput, 0, 0, p_pInput->GetWidth(), p_pInput->GetHeight());
			}

			std::string ToString(void) const { return "[BilateralFilter]"; }
		};

		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------
	}
}