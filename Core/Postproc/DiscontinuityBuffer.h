//----------------------------------------------------------------------------------------------
//	Filename:	DiscontinuityBuffer.h
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
		class DiscontinuityBuffer
			: public IPostProcess
		{
		protected:
			int m_nKernelSize;
			
			float m_fAngle,
				m_fDistance;

		public:
			DiscontinuityBuffer(int p_nKernelSize = 5, float p_fAngle = 0.75f, float p_fDistance = 10.0f)
				: IPostProcess()
				, m_nKernelSize(p_nKernelSize)
				, m_fAngle(p_fAngle)
				, m_fDistance(p_fDistance)
			{ }

			DiscontinuityBuffer(const std::string &p_strName, int p_nKernelSize = 5, float p_fAngle = 0.75f, float p_fDistance = 10.0f)
				: IPostProcess(p_strName)
				, m_nKernelSize(p_nKernelSize)
				, m_fAngle(p_fAngle)
				, m_fDistance(p_fDistance)
			{ }

			void SetKernelSize(int p_nKernelSize) { m_nKernelSize = p_nKernelSize; }

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, IPostProcess::BlendMode p_eBlendMode = IPostProcess::Replace)
			{
				m_fDistance = 0.5f;
				m_fAngle = 0.5f;//7f;
/*
				RadianceContext *pKernelContext,
					*pNeighbourContext,
					*pOutputContext;

				Spectrum irradiance;
				int irradianceSamples;

				int horizontalModulo = p_pInput->GetWidth() - (m_nKernelSize + 1),
					verticalScanLine;

				int borderWidth = m_nKernelSize >> 1,
					borderRegionY = p_nRegionY + borderWidth,
					borderRegionX = p_nRegionX + borderWidth,
					borderRegionHeight = p_nRegionHeight - borderWidth,
					borderRegionWidth = p_nRegionWidth - borderWidth;

				for (int y = borderRegionY; y < borderRegionHeight; ++y)
				{
					// Kernel Y Start
					verticalScanLine = y - borderWidth;

					// Pixel to convolve
					pKernelContext = p_pInput->GetP(borderRegionX, y);
					
					// Output buffer
					pOutputContext = p_pOutput->GetP(borderRegionX, y);

					for (int x = borderRegionX; x < borderRegionWidth; ++x, ++pKernelContext, ++pOutputContext)
					{
						// Irradiance samples and value
						irradiance = 0.f;
						irradianceSamples = 0;

						pNeighbourContext = p_pInput->GetP(x - borderWidth, verticalScanLine); 

						for (int kernelY = 0; kernelY < m_nKernelSize; kernelY++, pNeighbourContext += horizontalModulo)
						{
							for (int kernelX = 0; kernelX < m_nKernelSize; kernelX++, pNeighbourContext++)
							{
								//float d = Maths::Max(0, m_fDistance - Maths::FAbs(pKernelContext->Distance - pNeighbourContext->Distance)); 
								//float a = Maths::Max(0, Vector3::Dot(pKernelContext->Normal, pNeighbourContext->Normal) - m_fAngle);
								//int b = a + d > 0 ? 1 : 0;
								//irradiance = irradiance + pNeighbourContext->Indirect * b;
								//irradianceSamples += b;
								
								if (Maths::FAbs(pKernelContext->Distance - pNeighbourContext->Distance) < m_fDistance)
								{
									if (Vector3::Dot(pKernelContext->Normal, pNeighbourContext->Normal) > m_fAngle)
									{
										irradiance = irradiance + pNeighbourContext->Indirect;
										irradianceSamples++;
									}
								}
							}
						}

						pOutputContext->Final = pKernelContext->Direct + (irradiance * pKernelContext->Albedo) / irradianceSamples;
						pOutputContext->Flags |= RadianceContext::DF_Processed;
					}
				}
				*/

				RadianceContext *pKernelContext,
					*pNeighbourContext,
					*pOutputContext;

				Spectrum Li, Ld;

				int ys, ye, xs, xe;
				int irradianceSamples;

				// m_nKernelSize = 3;
				int border = 1;

				//----------------------------------------------------------------------------------------------
				for (int y = p_nRegionY + border; y < p_nRegionHeight - border; ++y)
				{
					ys = y - border;
					ye = y + border;

					for (int x = p_nRegionX + border; x < p_nRegionWidth - border; ++x)
					{
						xs = x - border;
						xe = x + border;

						irradianceSamples = 0;
						Li = 0.f;

						pKernelContext = p_pInput->GetP(x, y);
						pOutputContext = p_pOutput->GetP(x, y);

						for (int dy = ys; dy <= ye; dy++)
						{
							pNeighbourContext = p_pInput->GetP(xs, dy);

							for (int dx = xs; dx <= xe; dx++)
							{
								if (Vector3::Dot(pKernelContext->Normal, pNeighbourContext->Normal) > m_fAngle)
								{
									Li += pNeighbourContext->Indirect;
									irradianceSamples++;
								}

								pNeighbourContext++;
							}
						}
			
						// Compute final colour
						pOutputContext->Final = pKernelContext->Direct + (Li * pKernelContext->Albedo) / irradianceSamples;
						pOutputContext->Flags |= RadianceContext::DF_Processed;
					}
				}
				

				/*
				RadianceContext *pKernelContext,
					*pNeighbourContext,
					*pOutputContext;

				Spectrum Li, Ld;

				int ys, ye, xs, xe;
				int irradianceSamples;
				//float irradianceSamples;

				// m_nKernelSize = 3;

				// std::cout << "Discontinuity Buffer :: Kernel Size = " << m_nKernelSize << std::endl;

				//----------------------------------------------------------------------------------------------
				for (int y = p_nRegionY + m_nKernelSize; y < p_nRegionHeight - m_nKernelSize; ++y)
				{
					ys = y - m_nKernelSize;
					ye = y + m_nKernelSize;

					for (int x = p_nRegionX + m_nKernelSize; x < p_nRegionWidth - m_nKernelSize; ++x)
					{
						xs = x - m_nKernelSize;
						xe = x + m_nKernelSize;

						irradianceSamples = 0;
						Li = 0.f;

						pKernelContext = p_pInput->GetP(x, y);
						pOutputContext = p_pOutput->GetP(x, y);

						for (int dy = ys; dy <= ye; dy++)
						{
							pNeighbourContext = p_pInput->GetP(xs, dy);

							for (int dx = xs; dx <= xe; dx++)
							{
								if (Vector3::Dot(pKernelContext->Normal, pNeighbourContext->Normal) > m_fAngle)
								{
									Li += pNeighbourContext->Indirect;
									irradianceSamples++;

									//float wd = Maths::Sqr(pKernelContext->Distance - pNeighbourContext->Distance);
									//wd = Statistics::GaussianPDFApprox(wd);//(wd, 0, 4);

									//Li += pNeighbourContext->Indirect * wd;
									//irradianceSamples += wd;
								}

								pNeighbourContext++;
							}
						}
			
						// Compute final colour
						if (irradianceSamples) {
							pOutputContext->Final = pKernelContext->Direct + (Li * pKernelContext->Albedo) / irradianceSamples;
							//pOutputContext->Indirect = (Li * pKernelContext->Albedo) / irradianceSamples;
							//pOutputContext->Final = pOutputContext->Indirect;// pKernelContext->Direct + pOutputContext->Indirect; // * pKernelContext->Albedo;
							pOutputContext->Flags |= RadianceContext::DF_Processed;
						}
					}
				}
				*/
				
				return true;
			}

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput, IPostProcess::BlendMode p_eBlendMode = IPostProcess::Replace)
			{
				return Apply(p_pInput, p_pOutput, 0, 0, p_pInput->GetWidth(), p_pInput->GetHeight(), p_eBlendMode);
			}

			std::string ToString(void) const { return "[DiscontinuityBuffer]"; }
		};

		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------
	}
}