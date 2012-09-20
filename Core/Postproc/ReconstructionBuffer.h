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

				RadianceContext *pKernelContext,
					*pNeighbourContext,
					*pOutputContext;

				Spectrum Li, Ld;

				int ys, ye, xs, xe;
				int irradianceSamples;
				m_nKernelSize = 7;

				//----------------------------------------------------------------------------------------------
				for (int y = p_nRegionY + m_nKernelSize; y < p_nRegionHeight - m_nKernelSize; ++y)
				{
					ys = y - m_nKernelSize;
					ye = y + m_nKernelSize;

					for (int x = p_nRegionX + m_nKernelSize; x < p_nRegionWidth - m_nKernelSize; ++x)
					{
						pKernelContext = p_pInput->GetP(x, y);
						pOutputContext = p_pOutput->GetP(x, y);
						
						if (pKernelContext->Flags & RadianceContext::DF_Computed)
						{
							//pOutputContext->Final = pOutputContext->Indirect = pOutputContext->Direct = pOutputContext->Albedo = 10.f;
							//pOutputContext->Flags |= RadianceContext::DF_Final | RadianceContext::DF_Direct | RadianceContext::DF_Indirect | RadianceContext::DF_Albedo; 

							// pKernelContext->Flag = 0;
							// pKernelContext->Final = pKernelContext->Direct;
							continue;
						}

						// continue;

						xs = x - m_nKernelSize;
						xe = x + m_nKernelSize;

						irradianceSamples = 0;
						Spectrum Ld = 0.f, 
							Li = 0.f, 
							albedo = 0.f;

						for (int dy = ys; dy < ye; dy++)
						{
							pNeighbourContext = p_pInput->GetP(xs, dy);

							for (int dx = xs; dx < xe; dx++)
							{
								if (pNeighbourContext->Flags & RadianceContext::DF_Computed) 
								{
									// if (Vector3::Dot(pKernelContext->Normal, pNeighbourContext->Normal) > m_fAngle)
									{
										Ld += pNeighbourContext->Direct;
										Li += pNeighbourContext->Indirect;
										albedo += pNeighbourContext->Albedo;

										irradianceSamples++;
									}
								}

								pNeighbourContext++;
							}
						}
			
						// Compute final colour
						if (irradianceSamples) 
						{
							pOutputContext->Direct = Ld / irradianceSamples;
							pOutputContext->Indirect = Li / irradianceSamples;
							pOutputContext->Albedo = albedo / irradianceSamples;

							pOutputContext->Final = pOutputContext->Direct + pOutputContext->Indirect * pOutputContext->Albedo;
							pOutputContext->Flags |= RadianceContext::DF_Final | RadianceContext::DF_Direct | RadianceContext::DF_Indirect | RadianceContext::DF_Albedo; 
							// pOutputContext->Flag = 1;
						}
						/*else
						{
							pOutputContext->Final = pOutputContext->Indirect = pOutputContext->Direct = pOutputContext->Albedo = 10.f;
							pOutputContext->Flags |= RadianceContext::DF_Final | RadianceContext::DF_Direct | RadianceContext::DF_Indirect | RadianceContext::DF_Albedo; 
							pOutputContext->Flags |= RadianceContext::DF_Processed;
						}*/
					}
				}
				
				/*
				for (int y = p_nRegionY; y < p_nRegionHeight; ++y)
				{
					for (int x = p_nRegionX; x < p_nRegionWidth; ++x)
					{
						p_pOutput->GetP(x, y)->Flag = false;
					}
				}
				*/

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