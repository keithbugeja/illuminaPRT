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
		class MotionBlur
			: public IPostProcess
		{
		protected:
			std::vector<RadianceBuffer*> m_framebufferHistoryList;
	
			RadianceBuffer *m_pRestFrameBuffer,
				*m_pMotionFrameBuffer;

			int m_nRestBufferSamples,
				m_nRestFrameDelay,
				m_nMotionBlurFrames;

		public:
			MotionBlur(void)
				: IPostProcess()
				, m_pRestFrameBuffer(NULL)
				, m_pMotionFrameBuffer(NULL)
				, m_nRestBufferSamples(0)
				, m_nRestFrameDelay(0)
				, m_nMotionBlurFrames(0)
			{ }

			MotionBlur(const std::string &p_strName)
				: IPostProcess(p_strName)
				, m_pRestFrameBuffer(NULL)
				, m_pMotionFrameBuffer(NULL)
				, m_nRestBufferSamples(0)
				, m_nRestFrameDelay(0)
				, m_nMotionBlurFrames(0)
			{ }

			~MotionBlur(void)
			{
				Safe_Delete(m_pRestFrameBuffer);
				Safe_Delete(m_pMotionFrameBuffer);
			}

			void Reset(void)
			{
				m_nRestFrameDelay = m_nMotionBlurFrames;
			}

			void SetExternalBuffer(RadianceBuffer *p_pFrameBuffer, int p_nMotionBlurSize)
			{
				BOOST_ASSERT(p_nMotionBlurSize > 0);

				// Motion, rest and final buffers
				Safe_Delete(m_pRestFrameBuffer);
				Safe_Delete(m_pMotionFrameBuffer);

				// Allocate buffers
				int width = p_pFrameBuffer->GetWidth(),
					height = p_pFrameBuffer->GetHeight();

				m_pRestFrameBuffer = new RadianceBuffer(width, height); m_pRestFrameBuffer->Clear();
				m_pMotionFrameBuffer = new RadianceBuffer(width, height); m_pMotionFrameBuffer->Clear();

				// Reset counters
				m_nMotionBlurFrames = p_nMotionBlurSize;
				m_nRestFrameDelay = 0;
				m_nRestBufferSamples = 0;
			}

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight)
			{
				static const float actual = 0.65f;
				static const float history = 0.35f;

				RadianceContext
					*pRestContext,
					*pMotionContext,
					*pInputContext,
					*pOutputContext;				

				// If accumulation is enabled
				if (m_nRestFrameDelay == m_nMotionBlurFrames)
				{
					m_nRestFrameDelay--;
					m_nRestBufferSamples = 0;

					//----------------------------------------------------------------------------------------------			
					for (int y = p_nRegionY; y < p_nRegionHeight; ++y)
					{
						pInputContext = p_pInput->GetP(p_nRegionX, y);
						pOutputContext = p_pOutput->GetP(p_nRegionX, y);

						pRestContext = m_pRestFrameBuffer->GetP(p_nRegionX, y);
						pMotionContext = m_pMotionFrameBuffer->GetP(p_nRegionX, y);

						for (int x = p_nRegionX; x < p_nRegionWidth; ++x)
						{
							pMotionContext->Final = pInputContext->Final * actual + pMotionContext->Final * history;
							pRestContext->Final = 0.f;

							pOutputContext->Final = pMotionContext->Final;
							pOutputContext->Flags |= RadianceContext::DF_Accumulated;

							pInputContext++; pOutputContext++;
							pRestContext++; pMotionContext++;
						}
					}
					//----------------------------------------------------------------------------------------------
					//std::cout << "MotionBlur :: Moving camera..." << std::endl;
				} 
				else if (m_nRestFrameDelay > 0)
				{
					m_nRestFrameDelay--;
					m_nRestBufferSamples++;
				
					float fSamplesRcp = 1.f / (m_nRestBufferSamples + 1);

					//----------------------------------------------------------------------------------------------			
					for (int y = p_nRegionY; y < p_nRegionHeight; ++y)
					{
						pInputContext = p_pInput->GetP(p_nRegionX, y);
						pOutputContext = p_pOutput->GetP(p_nRegionX, y);

						pRestContext = m_pRestFrameBuffer->GetP(p_nRegionX, y);
						pMotionContext = m_pMotionFrameBuffer->GetP(p_nRegionX, y);
						
						for (int x = p_nRegionX; x < p_nRegionWidth; ++x)
						{
							pRestContext->Final += pInputContext->Final;
							pMotionContext->Final = pInputContext->Final * actual + pMotionContext->Final * history;

							pOutputContext->Final = (pMotionContext->Final + pRestContext->Final) * fSamplesRcp;
							pOutputContext->Flags |= RadianceContext::DF_Accumulated;

							pInputContext++; pOutputContext++;
							pMotionContext++; pRestContext++;
						}
					}
					//----------------------------------------------------------------------------------------------			
					//std::cout << "MotionBlur :: Stabilising camera..." << std::endl;
				}
				else if (m_nRestFrameDelay == 0)
				{
					// Add a sample to the rest framebuffer
					m_nRestBufferSamples++;

					//----------------------------------------------------------------------------------------------			
					for (int y = p_nRegionY; y < p_nRegionHeight; ++y)
					{
						pInputContext = p_pInput->GetP(p_nRegionX, y);
						pOutputContext = p_pOutput->GetP(p_nRegionX, y);

						pRestContext = m_pRestFrameBuffer->GetP(p_nRegionX, y);
						pMotionContext = m_pMotionFrameBuffer->GetP(p_nRegionX, y);
						
						for (int x = p_nRegionX; x < p_nRegionWidth; ++x)
						{
							pRestContext->Final += pInputContext->Final;

							pOutputContext->Final = pMotionContext->Final = pRestContext->Final / m_nRestBufferSamples;
							pOutputContext->Flags |= RadianceContext::DF_Accumulated;

							pInputContext++; pOutputContext++;
							pMotionContext++; pRestContext++;
						}
					}
					//----------------------------------------------------------------------------------------------			
					//std::cout << "MotionBlur :: Still camera..." << std::endl;
				}

				//std::cout << "MotionBlur :: Samples = [" << m_nRestBufferSamples << "], rest delay = [" << m_nRestFrameDelay << "]" << std::endl;
				return true;
			}

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput)
			{
				return Apply(p_pInput, p_pOutput, 0, 0, p_pInput->GetWidth(), p_pInput->GetHeight());
			}

			std::string ToString(void) const { return "[MotionBlur]"; }
		};


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


				std::cout << "HistoryBuffer :: Reset()" << std::endl;
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

					// float countInv = 1.f / m_historyBufferList.size();

					//----------------------------------------------------------------------------------------------			
					for (int y = p_nRegionY; y < p_nRegionHeight; ++y)
					{
						pInputContext = p_pInput->GetP(p_nRegionX, y);
						pOutputContext = p_pOutput->GetP(p_nRegionX, y);
						pDiscardContext = pFrameOut->GetP(p_nRegionX, y);
						pHistoryContext = m_pHistoryBuffer->GetP(p_nRegionX, y);
						
						for (int x = p_nRegionX; x < p_nRegionWidth; ++x)
						{
							pDiscardContext->Final = pInputContext->Final;

							// Uniform temporal weighting
							// pHistoryContext->Final = pHistoryContext->Final - pDiscardContext->Final + pInputContext->Final;
							// Uniform temporal weighting
							
							// Exponentially averaged weighting
							// pHistoryContext->Final = (pHistoryContext->Final + pInputContext->Final) * 0.5;
							pHistoryContext->Final = pHistoryContext->Final * 0.4f + pInputContext->Final * 0.6f;
							// Exponentially averaged weighting

							// pOutputContext->Final = pHistoryContext->Final * countInv;
							pOutputContext->Final = pHistoryContext->Final;
							pOutputContext->Flags |= RadianceContext::DF_Accumulated;

							pInputContext++; pOutputContext++; pDiscardContext++; pHistoryContext++;
						}
					}
					//----------------------------------------------------------------------------------------------			
				}
				else
				{					
					//if (m_nSampleCount == 0)
					//{
					//	for (int y = p_nRegionY; y < p_nRegionHeight; ++y)
					//	{
					//		pHistoryContext = m_pHistoryBuffer->GetP(p_nRegionX, y);

					//		for (int x = p_nRegionX; x < p_nRegionWidth; ++x)
					//		{
					//			pHistoryContext->Final = 0.f;

					//			for (int t = 0; t < m_historyBufferList.size(); ++t)
					//			{
					//				pHistoryContext->Final += m_historyBufferList[t]->GetP(x,y)->Final; 
					//			}

					//			pHistoryContext++;
					//		}
					//	}
					//}
					
					m_nSampleCount++;
					
					// Uniform temporal weighting
					float countInv = 1.f / (m_historyBufferList.size() + m_nSampleCount);
					// Uniform temporal weighting

					// Exponentially averaged weighting
					// float countInv = 1.f / (1 + m_nSampleCount);
					// Exponentially averaged weighting

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
							// Uniform temporal weighting
							pHistoryContext->Final = pHistoryContext->Final - pDiscardContext->Final + pInputContext->Final;
							// Uniform temporal weighting

							// Exponentially averaged weighting
							// pHistoryContext->Final = (pHistoryContext->Final + pInputContext->Final) * 0.5;
							// Exponentially averaged weighting

							pAccumulatorContext->Final += pDiscardContext->Final;
							pDiscardContext->Final = pInputContext->Final;
						
							pOutputContext->Final = (pHistoryContext->Final + pAccumulatorContext->Final) * countInv;
							pOutputContext->Flags |= RadianceContext::DF_Accumulated;

							pHistoryContext++; pAccumulatorContext++; pDiscardContext++; pOutputContext++; pInputContext++;
						}
					}
					//----------------------------------------------------------------------------------------------			
				
					std::cout << "HistoryBuffer :: Using " << m_nSampleCount << " samples in accumulation buffer" << std::endl;
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