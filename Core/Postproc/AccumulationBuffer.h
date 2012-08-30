//----------------------------------------------------------------------------------------------
//	Filename:	AccumulationBuffer.h
//	Author:		Keith Bugeja
//	Date:		27/03/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Postproc/PostProcess.h"
#include "Geometry/Intersection.h"

//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------
		class AccumulationBuffer
			: public IPostProcess
		{
		protected:
			RadianceBuffer *m_pAccumulationBuffer;

			int m_nSampleCount;

		public:
			AccumulationBuffer(void)
				: IPostProcess()
				, m_pAccumulationBuffer(NULL)
				, m_nSampleCount(0)
			{ }

			AccumulationBuffer(const std::string &p_strName)
				: IPostProcess(p_strName)
				, m_pAccumulationBuffer(NULL)
				, m_nSampleCount(0)
			{ }

			void Reset(void) 
			{
				BOOST_ASSERT(m_pAccumulationBuffer != NULL);

				m_nSampleCount = 0;
				
				for (int y = 0; y < m_pAccumulationBuffer->GetHeight(); ++y)
					for (int x = 0; x < m_pAccumulationBuffer->GetWidth(); ++x)
						m_pAccumulationBuffer->GetP(x,y)->Final = 0.f;

			}

			void SetAccumulationBuffer(RadianceBuffer *p_pAccumulationBuffer)
			{
				m_pAccumulationBuffer = p_pAccumulationBuffer;
				m_nSampleCount = 0;
			}

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight)
			{
				RadianceContext *pInputContext,
					*pOutputContext,
					*pAccumulatorContext;

				// Increase sample count
				m_nSampleCount++;

				float countInv = 1.0f / m_nSampleCount;

				//----------------------------------------------------------------------------------------------
				for (int y = p_nRegionY; y < p_nRegionHeight; ++y)
				{
					for (int x = p_nRegionX; x < p_nRegionWidth; ++x)
					{
						pInputContext = p_pInput->GetP(x, y);
						pOutputContext = p_pOutput->GetP(x, y);
						pAccumulatorContext = m_pAccumulationBuffer->GetP(x, y);

						pAccumulatorContext->Final = pInputContext->Final + pAccumulatorContext->Final;
						
						pOutputContext->Final = pAccumulatorContext->Final * countInv;
						pOutputContext->Flag = 1;
					}
				}

				return true;
			}

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput)
			{
				return Apply(p_pInput, p_pOutput, 0, 0, p_pInput->GetWidth(), p_pInput->GetHeight());
			}

			std::string ToString(void) const { return "[AccumulationBuffer]"; }
		};

		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------
	}
}