//----------------------------------------------------------------------------------------------
//	Filename:	DiscontinuityBuffer.h
//	Author:		Keith Bugeja
//	Date:		27/03/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/PostProcess.h"

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

		public:
			bool ApplyToRegion(RadianceBuffer *p_pBuffer, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight)
			{
			}

			bool ApplyToRegion(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight)
			{
				/*
				RadianceContext *pKernelContext,
					*pNeighbourContext,
					*pOutputContext;

				Spectrum Li;

				int ys, ye, xs, xe;
				int irradianceSamples;

				//----------------------------------------------------------------------------------------------
				for (int y = p_nRegionY + m_nDBSize; y < p_nRegionHeight - m_nDBSize; ++y)
				{
					ys = y - m_nDBSize;
					ye = y + m_nDBSize;

					for (int x = p_nRegionX + m_nDBSize; x < p_nRegionWidth - m_nDBSize; ++x)
					{
						xs = x - m_nDBSize;
						xe = x + m_nDBSize;

						irradianceSamples = 1;
						Li = 0.f;

						pKernelContext = p_pInput->GetP(x, y);
						pOutputContext = p_pOutput->GetP(x, y);

						for (int dy = ys; dy < ye; dy++)
						{
							pNeighbourContext = p_pInput->GetP(xs, dy);

							for (int dx = xs; dx < xe; dx++)
							{
								if (
								   //(Vector3::DistanceSquared(pRadianceContext->Position, pInnerContext->Position) < m_fDBDist) &&
									(Vector3::Dot(pKernelContext->Normal, pNeighbour->Normal) > m_fDBCos))
								{
									Li += pNeighbourContext->Indirect;
									irradianceSamples++;
								}

								pNeighbourContext++;
							}
						}
			
						// Compute final colour
						pOutputContext->Indirect = Li / irradianceSamples;
						pOutputContext->Flag = 1;
					}
				}
				*/
			}

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput)
			{
				return true;
			}

			std::string ToString(void) const { return "[DiscontinuityBuffer]"; }
		};

		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------
	}
}