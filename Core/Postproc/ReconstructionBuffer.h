//----------------------------------------------------------------------------------------------
//	Filename:	ReconstructionBuffer.h
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
		class ReconstructionBuffer
			: public IPostProcess
		{
		protected:
			int m_nKernelSize;
			
			float m_fAngle,
				m_fDistance;

		public:
			ReconstructionBuffer(int p_nKernelSize = 5, float p_fAngle = 0.75f, float p_fDistance = 10.0f)
				: IPostProcess()
				, m_nKernelSize(p_nKernelSize)
				, m_fAngle(p_fAngle)
				, m_fDistance(p_fDistance)
			{ }

			ReconstructionBuffer(const std::string &p_strName, int p_nKernelSize = 5, float p_fAngle = 0.75f, float p_fDistance = 10.0f)
				: IPostProcess(p_strName)
				, m_nKernelSize(p_nKernelSize)
				, m_fAngle(p_fAngle)
				, m_fDistance(p_fDistance)
			{ }

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight)
			{
				std::cout << "Reconstructing..." << std::endl;

				m_nKernelSize = 7;

				RadianceContext *pKernelContext,
					*pNeighbourContext,
					*pOutputContext;

				int halfKernel = 
					m_nKernelSize >> 1;

				int xs, xe, ys, ye,
					sx, sy,
					remainingSamples,
					maxSamples = 
						m_nKernelSize * m_nKernelSize;

				float weight,
					totalWeight;

				for (int y = p_nRegionY + halfKernel; y < p_nRegionHeight - halfKernel; ++y)
				{
					ys = y - halfKernel;
					ye = y + halfKernel;

					for (int x = p_nRegionX + halfKernel; x < p_nRegionWidth - halfKernel; ++x)
					{
						xs = x - halfKernel;
						xe = x + halfKernel;

						pKernelContext = p_pInput->GetP(x, y);
						pOutputContext = p_pOutput->GetP(x, y);

						if (pKernelContext->Flags & RadianceContext::DF_Direct > 0)
							continue;

						Spectrum indirect = 0.f,
							direct = 0.f,
							albedo = 0.f;

						remainingSamples = (m_nKernelSize * m_nKernelSize); // >> 2;

						for(totalWeight = 0; remainingSamples-- > 0;)
						{
							sx = xs + m_nKernelSize * QuasiRandomSequence::VanDerCorput(maxSamples - remainingSamples);
							sy = ys + m_nKernelSize * QuasiRandomSequence::Sobol2(maxSamples - remainingSamples);

							pNeighbourContext = p_pInput->GetP(sx, sy);

							// int d0 = (pNeighbourContext->Flags & RadianceContext::DF_Direct) / RadianceContext::DF_Direct;
							if (pNeighbourContext->Flags & RadianceContext::DF_Direct > 0)
							{
								float d = 1.f;

								indirect += pNeighbourContext->Indirect * d;
								direct += pNeighbourContext->Direct * d;
								albedo += pNeighbourContext->Albedo * d;

								totalWeight += d;
							}
						}

						if (totalWeight > Maths::Epsilon)
						{
							// if ((pOutputContext->Flags & RadianceContext::DF_Direct) == 0)
							{
								pOutputContext->Direct = direct / totalWeight;
								pOutputContext->Albedo = albedo / totalWeight;
							}

							// if ((pOutputContext->Flags & RadianceContext::DF_Indirect) == 0)
							{
								pOutputContext->Indirect = indirect / totalWeight;
							}

							pOutputContext->Final = (pOutputContext->Direct + pOutputContext->Indirect) * pOutputContext->Albedo;
						}
						/* */
					}
				}

				//----------------------------------------------------------------------------------------------

				return true;
			}

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput)
			{
				return Apply(p_pInput, p_pOutput, 0, 0, p_pInput->GetWidth(), p_pInput->GetHeight());
			}

			std::string ToString(void) const { return "[DiscontinuityBuffer]"; }
		};

		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------
	}
}