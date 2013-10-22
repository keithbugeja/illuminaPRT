//----------------------------------------------------------------------------------------------
//	Filename:	MultithreadedFrameless.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------------------------
//	Include basic headers for OpenMP and io, string and file streams
//----------------------------------------------------------------------------------------------
#include <iostream>
#include <sstream>
#include <fstream>

//----------------------------------------------------------------------------------------------
//	Include boost header files for managing program options and file paths
//----------------------------------------------------------------------------------------------
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

//----------------------------------------------------------------------------------------------
#include "Logger.h"
#include "Export.h"
#include "Environment.h"
#include "MultithreadedCommon.h"

//#include <CL/cl.hpp>
//#pragma lib("opencl.lib")
//----------------------------------------------------------------------------------------------
class RenderThread_Frameless 
{
public:
	struct RenderThreadTile
	{
		short XStart, YStart; 
		short XSize, YSize;
	};

	struct RenderThreadStatistics
	{
		double	FirstBarrier,
				JobTime,
				SecondBarrier;

		int		JobCount;
	};

	class RenderThreadState
	{
	protected:
		// Shared tile ID 
		Int32 m_nTileID;
	
		// Run flag
		bool m_bIsRunning;

		// Tile details
		int m_nWidth, m_nHeight;

		// Frame buffer
		RadianceBuffer *m_pRadianceBuffer;

		// Current renderer
		IRenderer *m_pRenderer;

		// Barrier for frame synchronisation
		int m_nThreadCount;

		// Budget for each frame
		float m_fThreadBudget;

		boost::barrier *m_pRenderBarrier;
		boost::mutex *m_pStatLock;

		// Stat list
		std::vector< std::vector<RenderThread::RenderThreadStatistics>* > m_statistics;
		std::vector< RenderThread::RenderThreadTile> m_tilePacket;

	public:
		RenderThreadState(IRenderer *p_pRenderer, RadianceBuffer *p_pRadianceBuffer, int p_nTileWidth, int p_nTileHeight, int p_nThreadCount, float p_fThreadBudget = 1e+20)
			: m_pRenderer(p_pRenderer)
			, m_pRadianceBuffer(p_pRadianceBuffer)
			, m_nWidth(p_nTileWidth)
			, m_nHeight(p_nTileHeight)
			, m_pRenderBarrier(new boost::barrier(p_nThreadCount))
			, m_pStatLock(new boost::mutex())
			, m_nTileID(0)
			, m_nThreadCount(p_nThreadCount)
			, m_fThreadBudget(p_fThreadBudget)
		{ }

		~RenderThreadState(void)
		{
			for (std::vector< std::vector<RenderThread::RenderThreadStatistics>* >::iterator iter = m_statistics.begin();
				 iter != m_statistics.end(); iter++) 
			{
				delete *iter;
			}

			m_statistics.clear();
		}

		inline IRenderer* GetRenderer(void) { return m_pRenderer; }
		inline RadianceBuffer* GetRadianceBuffer(void) { return m_pRadianceBuffer; }
		inline float GetThreadBudget(void) const { return m_fThreadBudget; }
		inline int GetThreadCount(void) const { return m_nThreadCount; }
		inline int GetWidth(void) const { return m_nWidth; }
		inline int GetHeight(void) const { return m_nHeight; }
		inline void Reset(void) { m_nTileID = 0; }
		inline void Wait(void) { m_pRenderBarrier->wait(); }
		inline void Run(void) { m_bIsRunning = true; }
		inline bool IsRunning(void) { return m_bIsRunning; }
		inline void Stop(void) { m_bIsRunning = false; }
		inline int NextTile(void) 
		{ 
			return AtomicInt32::FetchAndAdd(&m_nTileID, 1); 
		}
		
		inline void PushThreadStatistics(std::vector<RenderThread::RenderThreadStatistics>* p_pThreadStats)
		{
			m_pStatLock->lock();
			m_statistics.push_back(p_pThreadStats);
			m_pStatLock->unlock();
		}

		std::string GetStatistics(void) 
		{
			std::stringstream message;

			for (int threadID = 0; threadID < m_statistics.size(); threadID++)
			{
				std::vector<RenderThread::RenderThreadStatistics> *pThreadStatList = 
					m_statistics[threadID];

				message << " -- Thread ID : [" << threadID << "]" << std::endl;

				for (int frameNumber = 0; frameNumber < pThreadStatList->size(); frameNumber++)
				{
					message << " --- Frame : [" << frameNumber << "]" << std::endl 
							<< " ---- Barrier 1 : " << (*pThreadStatList)[frameNumber].FirstBarrier << std::endl 
							<< " ---- Computation : " <<  (*pThreadStatList)[frameNumber].JobTime << std::endl
							<< " ---- Barrier 2 : " <<  (*pThreadStatList)[frameNumber].SecondBarrier << std::endl
							<< " ---- Jobs : " <<  (*pThreadStatList)[frameNumber].JobCount << std::endl << std::endl;
				}
			}

			return message.str();
		}

		inline RenderThread::RenderThreadTile &GetTilePacket(int p_nTileID)
		{
			return m_tilePacket[p_nTileID];
		}

		inline int GetTilePacketCount(void)
		{
			return m_tilePacket.size();
		}

		void GenerateTilePackets(int p_nStepSize)
		{
			int tileSize = Maths::Max(m_nWidth, m_nHeight),
				varTileSize = tileSize;

			int tilesPerRow = m_pRadianceBuffer->GetWidth() / tileSize,
				tilesPerCol = m_pRadianceBuffer->GetHeight() / tileSize,
				tilesPerPage = tilesPerRow * tilesPerCol;

			int tileX, tileY, 
				stepIncrement = 0, 
				currentStep = 1;

			RenderThread::RenderThreadTile tilePacket;

			for (int tileID = 0; tileID < tilesPerPage; tileID++)
			{
				tileX = (tileID % tilesPerRow) * tileSize,
				tileY = (tileID / tilesPerRow) * tileSize;

				tilePacket.XSize = varTileSize;
				tilePacket.YSize = varTileSize;

				for (int subTileY = 0; subTileY < currentStep; subTileY++)
				{
					for (int subTileX = 0; subTileX < currentStep; subTileX++)
					{
						tilePacket.XStart = tileX + subTileX * varTileSize;
						tilePacket.YStart = tileY + subTileY * varTileSize;
							
						m_tilePacket.push_back(tilePacket);
					}
				}
				
				if (varTileSize <= 16)
					continue;

				if (++stepIncrement == p_nStepSize)
				{
					stepIncrement = 0;
					currentStep <<= 1;
					varTileSize >>= 1;
				}
			}
		}
	};

public:
	// Thread body
	static void Render(RenderThreadState *p_pState)
	{
		IRenderer *pRenderer = p_pState->GetRenderer();
		RadianceBuffer *pRadianceBuffer = p_pState->GetRadianceBuffer();

		int tileID, tilesPerPage = p_pState->GetTilePacketCount();

		// Add statistics log and push onto state
		std::vector<RenderThread::RenderThreadStatistics>* pThreadStatList 
			= new std::vector<RenderThread::RenderThreadStatistics>();
		p_pState->PushThreadStatistics(pThreadStatList);

		// Use as container for each frame
		RenderThread::RenderThreadStatistics threadStats;

		while (p_pState->IsRunning())
		{
			// Reset job count for frame
			threadStats.JobCount = 0;

			// Time first barrier
			/* 
			threadStats.FirstBarrier = Platform::GetTime();
			p_pState->Wait();
			threadStats.FirstBarrier = Platform::ToSeconds(Platform::GetTime() - threadStats.FirstBarrier);
			*/

			// Time job
			/* threadStats.JobTime = Platform::GetTime();
			while((tileID = p_pState->NextTile()) < tilesPerPage && Platform::ToSeconds(Platform::GetTime() - threadStats.JobTime) < p_pState->GetThreadBudget())
			*/
			while((tileID = p_pState->NextTile()) < tilesPerPage)			
			{
				// std::cout << boost::this_thread::get_id() << " : " << tileID << std::endl;
				const RenderThread::RenderThreadTile &tilePacket = p_pState->GetTilePacket(tileID);
				
				// pRenderer->RenderRegion(pRadianceBuffer, tilePacket.XStart, tilePacket.YStart, tilePacket.XSize, tilePacket.YSize, tilePacket.XStart, tilePacket.YStart);
				pRenderer->RenderTile(pRadianceBuffer, tileID, tilePacket.XSize, tilePacket.YSize);

				// pRenderer->RenderRegion(pRadianceBuffer, tilePacket.XStart + 1, tilePacket.YStart + 1, 
				//	tilePacket.XSize - 1, tilePacket.YSize - 1, tilePacket.XStart + 1, tilePacket.YStart + 1);

				// Increment jobs done
				// threadStats.JobCount++;
			}
			
			p_pState->Reset();
			// boost::this_thread::sleep(boost::posix_time::millisec(10));

			/*
			threadStats.JobTime = Platform::ToSeconds(Platform::GetTime() - threadStats.JobTime);

			// Time second barrier
			threadStats.SecondBarrier = Platform::GetTime();
			p_pState->Wait();
			threadStats.SecondBarrier = Platform::ToSeconds(Platform::GetTime() - threadStats.SecondBarrier);
			*/

			// Save frame stats
			// pThreadStatList->push_back(threadStats);
		}
	}
};
//----------------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------------
class IlluminaMTFrameless
	: public IIlluminaMT
{
protected:
	int m_nBackBuffer;

	RadianceBuffer 
		*m_pRadianceTemp,
		*m_pRadianceBuffer,
		*m_pRadianceOut;
	
	IPostProcess 
		*m_pTonemapFilter,
		*m_pBilateralFilter,
		*m_pDiscontinuityFilter,
		*m_pReconstructionFilter;

	MotionBlur *m_pTemporalFilter;

public:
	RadianceBuffer *GetCommitBuffer(void) { return m_pRadianceBuffer; }

	void SetJobs(int p_nSubdivide, int p_nSize) { m_nJobs = p_nSubdivide; m_nSize = p_nSize; }

	bool OnInitialise(void) 
	{
		//----------------------------------------------------------------------------------------------
		// Post processing setup
		//----------------------------------------------------------------------------------------------
		m_pRadianceTemp = new RadianceBuffer(m_pRenderer->GetDevice()->GetWidth(), m_pRenderer->GetDevice()->GetHeight());
		m_pRadianceBuffer = new RadianceBuffer(m_pRenderer->GetDevice()->GetWidth(), m_pRenderer->GetDevice()->GetHeight());
		m_pRadianceOut = new RadianceBuffer(m_pRenderer->GetDevice()->GetWidth(), m_pRenderer->GetDevice()->GetHeight());

		// Discontinuity, reconstruction and tone mapping
		m_pTonemapFilter = m_pEngineKernel->GetPostProcessManager()->CreateInstance("GlobalTone", "GlobalTone", "");
		m_pBilateralFilter = m_pEngineKernel->GetPostProcessManager()->CreateInstance("BilateralFilter", "BilateralFilter", "");
		m_pDiscontinuityFilter = m_pEngineKernel->GetPostProcessManager()->CreateInstance("Discontinuity", "DiscontinuityBuffer", "");
		m_pReconstructionFilter = m_pEngineKernel->GetPostProcessManager()->CreateInstance("Reconstruction", "ReconstructionBuffer", "");

		// Motion blur filter
		m_pTemporalFilter = (MotionBlur*)m_pEngineKernel->GetPostProcessManager()->CreateInstance("MotionBlur", "MotionBlur", "");
		m_pTemporalFilter->SetExternalBuffer(m_pRadianceBuffer, 6);
		m_pTemporalFilter->Reset();
	
		//----------------------------------------------------------------------------------------------
		// Rendering budget setup
		//----------------------------------------------------------------------------------------------
		if (m_fFrameBudget > Maths::Epsilon)
			m_pRenderer->SetRenderBudget(m_fFrameBudget);
		else
			m_pRenderer->SetRenderBudget(m_fFrameBudget = 10000);

		return true;
	}

	bool OnShutdown(void)
	{
		return true;
	}

	void Render(void)
	{
		// Prepare integrator
		m_pIntegrator->Prepare(m_pEnvironment->GetScene());

		//----------------------------------------------------------------------------------------------
		// Rendering threads setup
		//----------------------------------------------------------------------------------------------
		RenderThread_Frameless::RenderThreadState renderThreadState(m_pRenderer, m_pRadianceOut, m_nSize, m_nSize, m_nThreadCount + 1, m_fFrameBudget);
		renderThreadState.Reset();
		renderThreadState.Run();

		renderThreadState.GenerateTilePackets(m_nJobs);

		for (int threadIdx = 0; threadIdx < m_nThreadCount; threadIdx++)
			boost::thread runThread(boost::bind(&RenderThread_Frameless::Render, &renderThreadState));

		//----------------------------------------------------------------------------------------------
		// Initialise timing
		//----------------------------------------------------------------------------------------------
		double start, elapsed = 0, eventStart, eventComplete;
	
		double fTotalFramesPerSecond = 0,
			fFrameTime = 0, fTotalTime = 0,
			fFrameIntegratorTime = 0, fTotalIntegratorTime = 0,
			fFrameSpaceTime = 0, fTotalSpaceTime = 0,
			fFrameRadianceTime = 0, fTotalRadianceTime = 0,
			fFramePostProcessingTime = 0, fTotalPostProcessingTime = 0,
			fFrameCommitTime = 0, fTotalCommitTime = 0;

		int nFramesProcessed = 0;
		
		//// Prepare integrator
		//m_pIntegrator->Prepare(m_pEnvironment->GetScene());

		//----------------------------------------------------------------------------------------------
		// Render loop
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
		if (m_pListener != NULL) m_pListener->OnBeginRender(this);
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
		for (int nFrame = 0; nFrame < m_nIterations; ++nFrame)
		{
			//----------------------------------------------------------------------------------------------
			if (m_pListener != NULL) m_pListener->OnBeginFrame(this);
			//----------------------------------------------------------------------------------------------

			//----------------------------------------------------------------------------------------------
			// Integrator preparation
			//----------------------------------------------------------------------------------------------
			eventStart = start = Platform::GetTime();

			//// Prepare integrator
			//m_pIntegrator->Prepare(m_pEnvironment->GetScene());

			// Time integrator event
			eventComplete = Platform::GetTime();
			fFrameIntegratorTime = Platform::ToSeconds(eventComplete - eventStart);
			fTotalIntegratorTime += fFrameIntegratorTime;

			//----------------------------------------------------------------------------------------------
			// Space update 
			//----------------------------------------------------------------------------------------------
			eventStart = Platform::GetTime();

			// Update space
			m_pSpace->Update();

			// Time space update event
			eventComplete = Platform::GetTime();
			fFrameSpaceTime = Platform::ToSeconds(eventComplete - eventStart);
			fTotalSpaceTime += fFrameSpaceTime;

			//----------------------------------------------------------------------------------------------
			// Radiance computation 
			//----------------------------------------------------------------------------------------------
			eventStart = Platform::GetTime();

			memcpy(m_pRadianceBuffer->GetP(0, 0), 
				m_pRadianceOut->GetP(0,0), m_pRadianceOut->GetArea() * sizeof(RadianceContext));

			// Time radiance computation event
			eventComplete = Platform::GetTime();
			fFrameRadianceTime = Platform::ToSeconds(eventComplete - eventStart);
			fTotalRadianceTime += fFrameRadianceTime;

			//----------------------------------------------------------------------------------------------
			// Post processing 
			//----------------------------------------------------------------------------------------------
			 
			eventStart = Platform::GetTime();

			// Bilateral filter
			if (m_flags.IsBilateralFilterEnabled())
				m_pBilateralFilter->Apply(m_pRadianceBuffer, m_pRadianceTemp);

			// Discontinuity
			if (m_flags.IsDiscontinuityBufferEnabled())
				m_pDiscontinuityFilter->Apply(m_pRadianceBuffer, m_pRadianceBuffer);

			// Accumulation
			if (m_flags.IsAccumulationBufferEnabled())
			{
				m_pTemporalFilter->Apply(m_pRadianceBuffer, m_pRadianceBuffer);
			}
			else
				m_pEnvironment->GetScene()->GetSampler()->Reset();

			// Tonemapping
			
			if (m_flags.IsToneMappingEnabled())
				m_pTonemapFilter->Apply(m_pRadianceBuffer, m_pRadianceBuffer);			
				//m_pTonemapFilter->Apply(m_pRadianceTemp, m_pRadianceBuffer);
				//m_pTonemapFilter->Apply(m_pRadianceBuffer, m_pRadianceTemp);
			

			// Time radiance computation event
			eventComplete = Platform::GetTime();
			fFramePostProcessingTime = Platform::ToSeconds(eventComplete - eventStart);
			fTotalPostProcessingTime += fFramePostProcessingTime;

			//----------------------------------------------------------------------------------------------
			// Output 
			//----------------------------------------------------------------------------------------------
			eventStart = Platform::GetTime();
				
			// Commit frame
			if (!m_flags.IsOutputToNullDevice())
				m_pRenderer->Commit(m_pRadianceBuffer);

			// Time radiance computation event
			eventComplete = Platform::GetTime();
			fFrameCommitTime = Platform::ToSeconds(eventComplete - eventStart);
			fTotalCommitTime += fFrameCommitTime;
			
			//----------------------------------------------------------------------------------------------
			if (m_pListener != NULL) m_pListener->OnEndFrame(this);
			//----------------------------------------------------------------------------------------------

			// Max is 60fps
			// boost::this_thread::yield();
			boost::this_thread::sleep(boost::posix_time::millisec(100));

			//----------------------------------------------------------------------------------------------
			// Frame time computations 
			//----------------------------------------------------------------------------------------------
			// Compute frame time
			fFrameTime = elapsed = Platform::ToSeconds(Platform::GetTime() - start);
			fTotalTime += fFrameTime;

			// Compute total fps
			fTotalFramesPerSecond += (float)(1.f/fFrameTime);

			// Increment number of computed frames
			nFramesProcessed++;

			//----------------------------------------------------------------------------------------------
			// Verbose output 
			//----------------------------------------------------------------------------------------------
			if (nFramesProcessed % m_nLoggerUpdate == 0)
			{
				std::stringstream message;
				message << std::endl << "-- Frame " << nFrame << std::endl
					<< "--- Integrator Preparation Time : [" << fFrameIntegratorTime << "s]" << std::endl
					<< "--- Space Update Time : [" << fFrameSpaceTime << "s]" << std::endl
					<< "--- Radiance Computation Time : [" << fFrameRadianceTime << "s]" << std::endl
					<< "--- Post-processing Time : [" << fFramePostProcessingTime << "s]" << std::endl
					<< "--- Output Time : [" << fFrameCommitTime << "s]" << std::endl
					<< "--- Frame Render Time : [" << fFrameTime << "s]" << std::endl
					<< "--- Frames per second : [" << fTotalFramesPerSecond / nFramesProcessed << "]" << std::endl;

				message << std::endl << "--- Integrator " << std::endl
					<< "--- " << m_pIntegrator->ToString() << std::endl;

				m_pLogger->Write(message.str());
			}
		}

		//----------------------------------------------------------------------------------------------
		// Stop rendering threads
		//----------------------------------------------------------------------------------------------
		renderThreadState.Stop();

		//----------------------------------------------------------------------------------------------
		// Output averaged timings
		//----------------------------------------------------------------------------------------------
		std::stringstream message;
		message << std::endl << "-- Average timings over [" << nFramesProcessed << "] frames" << std::endl
			<< "--- Integrator Preparation Time : [" << fTotalIntegratorTime / nFramesProcessed << "s]" << std::endl
			<< "--- Space Update Time : [" << fTotalSpaceTime / nFramesProcessed << "s]" << std::endl
			<< "--- Radiance Computation Time : [" << fTotalRadianceTime / nFramesProcessed << "s]" << std::endl
			<< "--- Post-processing Time : [" << fTotalPostProcessingTime / nFramesProcessed << "s]" << std::endl
			<< "--- Output Time : [" << fTotalCommitTime / nFramesProcessed << "s]" << std::endl
			<< "--- Frame Render Time : [" << fTotalTime / nFramesProcessed << "s]" << std::endl
			<< "--- Frames per second : [" << fTotalFramesPerSecond / nFramesProcessed << "]" << std::endl << std::endl;
		m_pLogger->Write(message.str());

		// Output per thread statistics
		std::ofstream threadTimings;
		threadTimings.open("threadTimings.txt", std::ios::binary);
		threadTimings << renderThreadState.GetStatistics() << std::endl;
		threadTimings.close();

		//----------------------------------------------------------------------------------------------
		if (m_pListener != NULL) m_pListener->OnEndRender(this);
		//----------------------------------------------------------------------------------------------
	}
};
