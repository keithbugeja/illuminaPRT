//----------------------------------------------------------------------------------------------
//	Filename:	DiscontinuityBuffer.h
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
		class DiscontinuityBuffer
			: public IPostProcess
		{
		protected:
			int m_nKernelSize;
			
			float m_fAngleCosine,
				m_fDistance;

		public:
			DiscontinuityBuffer(int p_nKernelSize = 1, float p_fAngle = 0.75f, float p_fDistance = 10.0f)
				: IPostProcess()
				, m_nKernelSize(p_nKernelSize)
				, m_fAngleCosine(p_fAngle)
				, m_fDistance(p_fDistance)
			{ }

			DiscontinuityBuffer(const std::string &p_strName, int p_nKernelSize = 1, float p_fAngle = 0.75f, float p_fDistance = 10.0f)
				: IPostProcess(p_strName)
				, m_nKernelSize(p_nKernelSize)
				, m_fAngleCosine(p_fAngle)
				, m_fDistance(p_fDistance)
			{ }

			bool ApplyToRegion(RadianceBuffer *p_pBuffer, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight)
			{
				//bool result = ApplyToRegion(p_pBuffer, tempBuffer, p_nRegionX, p_nRegionY, p_nRegionWidth, p_nRegionHeight);
				//return result;
				return true;
			}

			bool ApplyToRegion(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight)
			{
				RadianceContext *pKernelContext,
					*pNeighbourContext,
					*pOutputContext;

				Spectrum Li;

				int ys, ye, xs, xe;
				int irradianceSamples;

				//----------------------------------------------------------------------------------------------
				for (int y = p_nRegionY + m_nKernelSize; y < p_nRegionHeight - m_nKernelSize; ++y)
				{
					ys = y - m_nKernelSize;
					ye = y + m_nKernelSize;

					for (int x = p_nRegionX + m_nKernelSize; x < p_nRegionWidth - m_nKernelSize; ++x)
					{
						xs = x - m_nKernelSize;
						xe = x + m_nKernelSize;

						irradianceSamples = 1;
						Li = 0.f;

						pKernelContext = p_pInput->GetP(x, y);
						pOutputContext = p_pOutput->GetP(x, y);

						for (int dy = ys; dy < ye; dy++)
						{
							pNeighbourContext = p_pInput->GetP(xs, dy);

							for (int dx = xs; dx < xe; dx++)
							{
								if (//(Vector3::DistanceSquared(pRadianceContext->Position, pInnerContext->Position) < m_fDBDist) &&
									(Vector3::Dot(pKernelContext->Normal, pNeighbourContext->Normal) > m_fAngleCosine))
								{
									Li += pNeighbourContext->Indirect;
									irradianceSamples++;
								}

								pNeighbourContext++;
							}
						}
			
						// Compute final colour
						pOutputContext->Final = pKernelContext->Direct + Li / irradianceSamples;
						pOutputContext->Flag = 1;
					}
				}

				std::cout << "Ready!" << std::endl;

				return true;
			}

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput)
			{
				return ApplyToRegion(p_pInput, p_pOutput, 0, 0, p_pInput->GetWidth(), p_pInput->GetHeight());
			}

			std::string ToString(void) const { return "[DiscontinuityBuffer]"; }
		};

		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------
	}
}