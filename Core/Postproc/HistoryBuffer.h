//----------------------------------------------------------------------------------------------
//	Filename:	HistoryBuffer.h
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
		class HistoryBuffer
			: public IPostProcess
		{
		protected:
			std::vector<RadianceBuffer*> m_historyBufferList;
			RadianceBuffer *m_pAccumulationBuffer,
				*m_pHistoryBuffer;

			int m_nSampleCount,
				m_nFrameNumber,
				m_nAccumulationDelay;

		public:
			HistoryBuffer(void)
				: IPostProcess()
				, m_pAccumulationBuffer(NULL)
				, m_pHistoryBuffer(NULL)
				, m_nSampleCount(0)
				, m_nFrameNumber(0)
				, m_nAccumulationDelay(0)
			{ }

			HistoryBuffer(const std::string &p_strName)
				: IPostProcess(p_strName)
				, m_pAccumulationBuffer(NULL)
				, m_pHistoryBuffer(NULL)
				, m_nSampleCount(0)
				, m_nFrameNumber(0)
				, m_nAccumulationDelay(0)
			{ }

			void Reset(void) 
			{
				BOOST_ASSERT(m_pAccumulationBuffer != NULL);

				m_nSampleCount = 0;
				m_nAccumulationDelay = m_historyBufferList.size();			
				m_pAccumulationBuffer->Clear();

				/*
				for (int y = 0; y < m_pAccumulationBuffer->GetHeight(); ++y)
					for (int x = 0; x < m_pAccumulationBuffer->GetWidth(); ++x)
						m_pAccumulationBuffer->GetP(x,y)->Final = 0.f;
				*/
			}

			void SetHistoryBuffer(RadianceBuffer *p_pHistoryBuffer, RadianceBuffer *p_pAccumulationBuffer, int p_nHistorySize)
			{
				if (m_historyBufferList.size() > 0)
				{
					for(std::vector<RadianceBuffer*>::iterator it = m_historyBufferList.begin();
						it != m_historyBufferList.end(); it++)
					{
						delete *it;
					}

					m_historyBufferList.clear();
				}

				m_pHistoryBuffer = p_pHistoryBuffer;
				m_pAccumulationBuffer = p_pAccumulationBuffer;
				
				m_pHistoryBuffer->Clear();
				m_pAccumulationBuffer->Clear();
				
				m_nSampleCount = 0;
				m_nFrameNumber = 0;
				m_nAccumulationDelay = 0;

				for (int frame = 0; frame < p_nHistorySize; frame++)
				{
					m_historyBufferList.push_back(new RadianceBuffer(m_pAccumulationBuffer->GetWidth(), m_pAccumulationBuffer->GetHeight()));
				}
			}

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight)
			{
				RadianceContext 
					*pInputContext,
					*pOutputContext,
					*pDiscardContext,
					*pHistoryContext,
					*pAccumulatorContext;

				RadianceBuffer *pFrameOut = m_historyBufferList[m_nFrameNumber];
				m_nFrameNumber = (m_nFrameNumber + 1) % m_historyBufferList.size();

				if (m_nAccumulationDelay > 0)
				{
					m_nAccumulationDelay--;

					float countInv = 1.f / m_historyBufferList.size();

					//----------------------------------------------------------------------------------------------			
					for (int y = p_nRegionY; y < p_nRegionHeight; ++y)
					{
						pInputContext = p_pInput->GetP(p_nRegionX, y);
						pOutputContext = p_pOutput->GetP(p_nRegionX, y);
						pDiscardContext = pFrameOut->GetP(p_nRegionX, y);
						pHistoryContext = m_pHistoryBuffer->GetP(p_nRegionX, y);

						for (int x = p_nRegionX; x < p_nRegionWidth; ++x)
						{
							pHistoryContext->Final = pHistoryContext->Final - pDiscardContext->Final + pInputContext->Final;
							pDiscardContext->Final = pInputContext->Final;

							pOutputContext->Final = pHistoryContext->Final * countInv;
							pOutputContext->Flags |= RadianceContext::DF_Accumulated;

							pInputContext++; pOutputContext++; pDiscardContext++; pHistoryContext++;
						}
					}
					//----------------------------------------------------------------------------------------------			
				}
				else
				{
					m_nSampleCount++;
					
					float countInv = 1.f / (m_historyBufferList.size() + m_nSampleCount);

					//----------------------------------------------------------------------------------------------			
					for (int y = p_nRegionY; y < p_nRegionHeight; ++y)
					{
						pInputContext = p_pInput->GetP(p_nRegionX, y);
						pOutputContext = p_pOutput->GetP(p_nRegionX, y);

						pHistoryContext = m_pHistoryBuffer->GetP(p_nRegionX, y);
						pDiscardContext = pFrameOut->GetP(p_nRegionX, y);

						pAccumulatorContext = m_pAccumulationBuffer->GetP(p_nRegionX, y);

						for (int x = p_nRegionX; x < p_nRegionWidth; ++x)
						{
							pAccumulatorContext->Final += pDiscardContext->Final;
							pHistoryContext->Final = pHistoryContext->Final - pDiscardContext->Final + pInputContext->Final;
							pDiscardContext->Final = pInputContext->Final;
						
							pOutputContext->Final = (pHistoryContext->Final + pAccumulatorContext->Final) * countInv;
							pOutputContext->Flags |= RadianceContext::DF_Accumulated;

							pHistoryContext++; pAccumulatorContext++; pDiscardContext++; pOutputContext++; pInputContext++;
						}
					}
					//----------------------------------------------------------------------------------------------			
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