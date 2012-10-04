//----------------------------------------------------------------------------------------------
//	Filename:	main.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
// TODO:
// Double check ILight-derived classes ... some methods have not been tested properly.
// ?? DistributedRenderer should not instantiate MPI - change it to have it passed to the object
// Polish object factories
// Move factories to CorePlugins.dll
// Finish scene loaders
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
//	Set Illumina PRT compilation mode (SHM or DSM)
//----------------------------------------------------------------------------------------------
#define ILLUMINA_SHM

#if (!defined ILLUMINA_SHM)
	#define ILLUMINA_DSM
#endif

//----------------------------------------------------------------------------------------------
//	Set Illumina PRT version
//----------------------------------------------------------------------------------------------
namespace Illumina { namespace Core { const int Major = 0; const int Minor = 5; const int Build = 0; } }

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
//	Include basic headers for OpenMP and io, string and file streams
//----------------------------------------------------------------------------------------------
// #include <omp.h>
#include <iostream>
#include <sstream>
#include <fstream>

//----------------------------------------------------------------------------------------------
//	Include boost header files for managing program options and file paths
//----------------------------------------------------------------------------------------------
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

//----------------------------------------------------------------------------------------------
#if (defined(ILLUMINA_SHM))
//----------------------------------------------------------------------------------------------
#include "Logger.h"
#include "Environment.h"

//----------------------------------------------------------------------------------------------
class RenderThread 
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
		int m_nTileID;
	
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
			// return AtomicInt32::Increment(&m_nTileID);
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
			threadStats.FirstBarrier = Platform::GetTime();
			p_pState->Wait();
			threadStats.FirstBarrier = Platform::ToSeconds(Platform::GetTime() - threadStats.FirstBarrier);

			// Time job
			threadStats.JobTime = Platform::GetTime();
			while((tileID = p_pState->NextTile()) < tilesPerPage && Platform::ToSeconds(Platform::GetTime() - threadStats.JobTime) < p_pState->GetThreadBudget())			
			{
				// std::cout << boost::this_thread::get_id() << " : " << tileID << std::endl;
				const RenderThread::RenderThreadTile &tilePacket = p_pState->GetTilePacket(tileID);
				
				/**/
				pRenderer->RenderRegion(pRadianceBuffer, tilePacket.XStart, tilePacket.YStart, tilePacket.XSize, tilePacket.YSize, tilePacket.XStart, tilePacket.YStart);
				/**/

				/*
				pRenderer->RenderRegion(pRadianceBuffer, tilePacket.XStart + 1, tilePacket.YStart + 1, 
					tilePacket.XSize - 1, tilePacket.YSize - 1, tilePacket.XStart + 1, tilePacket.YStart + 1);
				/**/

				// Increment jobs done
				threadStats.JobCount++;
			}
			threadStats.JobTime = Platform::ToSeconds(Platform::GetTime() - threadStats.JobTime);

			// Time second barrier
			threadStats.SecondBarrier = Platform::GetTime();
			p_pState->Wait();
			threadStats.SecondBarrier = Platform::ToSeconds(Platform::GetTime() - threadStats.SecondBarrier);

			// Save frame stats
			pThreadStatList->push_back(threadStats);
		}
	}
};

void IlluminaPRT(bool p_bVerbose, int p_nVerboseFrequency, 
	int p_nIterations, int p_nThreads, int p_nFPS, 
	int p_nJobs, int p_nSize, int p_nFlags, 
	std::string p_strScript)
{
	//----------------------------------------------------------------------------------------------
	// Parse flags
	//----------------------------------------------------------------------------------------------
	bool bToneMappingEnabled			= (p_nFlags & 0x01), 
		bBilateralFilterEnabled			= (p_nFlags & 0x02),
		bDiscontinuityBufferEnabled		= (p_nFlags & 0x04),
		bFrameReconstructionEnabled		= (p_nFlags & 0x08),
		bAccumulationBufferEnabled		= (p_nFlags & 0x10),
		bOutputToNullDeviceEnabled		= (p_nFlags & 0x20);

	if (p_bVerbose)
	{
		std::cout << "-- Tone Mapping [" << bToneMappingEnabled << "]" << std::endl;
		std::cout << "-- Bilateral Filter [" << bBilateralFilterEnabled << "]" << std::endl;
		std::cout << "-- Discontinuity Buffer [" << bDiscontinuityBufferEnabled << "]" << std::endl;
		std::cout << "-- Frame Reconstruction [" << bFrameReconstructionEnabled << "]" << std::endl;
		std::cout << "-- Accumulation Buffer [" << bAccumulationBufferEnabled << "]" << std::endl;
		std::cout << "-- Null Device Output [" << bOutputToNullDeviceEnabled << "]" << std::endl;
	}

	//----------------------------------------------------------------------------------------------
	// Illumina sandbox environment 
	//----------------------------------------------------------------------------------------------
	SandboxEnvironment sandbox;

	sandbox.Initialise(p_bVerbose);
	if (!sandbox.LoadScene(p_strScript, p_bVerbose))
	{
		sandbox.Shutdown(p_bVerbose);
		return;
	}

	//----------------------------------------------------------------------------------------------
	// Alias required components
	//----------------------------------------------------------------------------------------------
	IIntegrator *pIntegrator = sandbox.GetEnvironment()->GetIntegrator();
	IRenderer *pRenderer = sandbox.GetEnvironment()->GetRenderer();
	ICamera *pCamera = sandbox.GetEnvironment()->GetCamera();
	ISpace *pSpace = sandbox.GetEnvironment()->GetSpace();

	Environment *pEnvironment = sandbox.GetEnvironment();
	EngineKernel *pEngineKernel = sandbox.GetEngineKernel();

	// Open output device
	pRenderer->GetDevice()->Open();

	// Initialisation complete
	Logger::Message("Initialisation complete. Rendering in progress...", p_bVerbose);

	//----------------------------------------------------------------------------------------------
	// Post processing setup
	//----------------------------------------------------------------------------------------------
	RadianceBuffer *pRadianceTemporalBuffer = new RadianceBuffer(
		pRenderer->GetDevice()->GetWidth(), pRenderer->GetDevice()->GetHeight()),
		*pRadianceAccumulationBuffer = new RadianceBuffer(
		pRenderer->GetDevice()->GetWidth(), pRenderer->GetDevice()->GetHeight()),
		*pRadianceBuffer, *pRadianceBufferChain[2];
	
	pRadianceBufferChain[0] = new RadianceBuffer(
		pRenderer->GetDevice()->GetWidth(), pRenderer->GetDevice()->GetHeight());
	
	pRadianceBufferChain[1] = new RadianceBuffer(
		pRenderer->GetDevice()->GetWidth(), pRenderer->GetDevice()->GetHeight());

	pRadianceBuffer = pRadianceBufferChain[0];

	int backBuffer = 0;

	IPostProcess *pDiscontinuityBuffer = pEngineKernel->GetPostProcessManager()->CreateInstance("Discontinuity", "DiscontinuityBuffer", "");
	IPostProcess *pAutoTone = pEngineKernel->GetPostProcessManager()->CreateInstance("AutoTone", "AutoTone", "");
	IPostProcess *pDragoTone = pEngineKernel->GetPostProcessManager()->CreateInstance("DragoTone", "DragoTone", ""); 
	IPostProcess *pGlobalTone = pEngineKernel->GetPostProcessManager()->CreateInstance("GlobalTone", "GlobalTone", "");
	IPostProcess *pReconstructionBuffer = pEngineKernel->GetPostProcessManager()->CreateInstance("Reconstruction", "ReconstructionBuffer", "");
	IPostProcess *pBilateralFilter = pEngineKernel->GetPostProcessManager()->CreateInstance("BilateralFilter", "BilateralFilter", "");

	AccumulationBuffer *pAccumulationBuffer = (AccumulationBuffer*)pEngineKernel->GetPostProcessManager()->CreateInstance("Accumulation", "AccumulationBuffer", "");
	pAccumulationBuffer->SetAccumulationBuffer(pRadianceAccumulationBuffer);
	pAccumulationBuffer->Reset();

	//----------------------------------------------------------------------------------------------
	// Rendering budget setup
	//----------------------------------------------------------------------------------------------
	float fRenderBudget;

	if (p_nFPS != 0)
	{
		fRenderBudget = 1.f / p_nFPS;
		pRenderer->SetRenderBudget((fRenderBudget * p_nThreads) / 256.0f );
		/*
		int tileCount = (pRenderer->GetDevice()->GetWidth() / p_nSize) * 
			(pRenderer->GetDevice()->GetHeight() / p_nSize);

		float perTileBudget = 1.f / (p_nFPS * (tileCount / p_nThreads));
		pRenderer->SetRenderBudget(perTileBudget);
		*/
	}
	else
	{
		fRenderBudget = 1e+20;
		pRenderer->SetRenderBudget(10000);
	}

	//----------------------------------------------------------------------------------------------
	// Rendering threads setup
	//----------------------------------------------------------------------------------------------
	RenderThread::RenderThreadState renderThreadState(pRenderer, pRadianceBuffer, p_nSize, p_nSize, p_nThreads + 1, fRenderBudget);
	renderThreadState.Reset();
	renderThreadState.Run();

	renderThreadState.GenerateTilePackets(p_nJobs);

	for (int threadIdx = 0; threadIdx < p_nThreads; threadIdx++)
		boost::thread runThread(boost::bind(&RenderThread::Render, &renderThreadState));

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

	//----------------------------------------------------------------------------------------------
	// Render loop
	//----------------------------------------------------------------------------------------------
	float alpha = Maths::Pi;
	Matrix3x3 rotation;
	Vector3 observer = pCamera->GetObserver();

	for (int nFrame = 0; nFrame < p_nIterations; ++nFrame)
	{
		// Animate scene 
		//alpha += Maths::PiTwo / 180.f;
		
		rotation.MakeRotation(Vector3::UnitYPos, alpha);

		////((GeometricPrimitive*)pSpace->PrimitiveList[0])->WorldTransform.SetScaling(Vector3::Ones * 20.0f);
		//// ((GeometricPrimitive*)pSpace->PrimitiveList[0])->WorldTransform.SetRotation(rotation);

		////pCamera->MoveTo(lookFrom);
		////pCamera->MoveTo(Vector3(Maths::Cos(alpha) * lookFrom.X, lookFrom.Y, Maths::Sin(alpha) * lookFrom.Z));
		////pCamera->LookAt(lookAt);

		Vector3 observer_ = observer;
		observer_.Z += Maths::Cos(alpha) * 4.f;
		observer_.X += Maths::Sin(alpha) * 2.f;
		// pCamera->MoveTo(observer_);


		//----------------------------------------------------------------------------------------------
		// Integrator preparation
		//----------------------------------------------------------------------------------------------
		eventStart = start = Platform::GetTime();

		// Prepare integrator
		pIntegrator->Prepare(pEnvironment->GetScene());

		// Time integrator event
		eventComplete = Platform::GetTime();
		fFrameIntegratorTime = Platform::ToSeconds(eventComplete - eventStart);
		fTotalIntegratorTime += fFrameIntegratorTime;

		//----------------------------------------------------------------------------------------------
		// Space update 
		//----------------------------------------------------------------------------------------------
		eventStart = Platform::GetTime();

		// Update space
		pSpace->Update();

		// Time space update event
		eventComplete = Platform::GetTime();
		fFrameSpaceTime = Platform::ToSeconds(eventComplete - eventStart);
		fTotalSpaceTime += fFrameSpaceTime;

		//----------------------------------------------------------------------------------------------
		// Radiance computation 
		//----------------------------------------------------------------------------------------------
		eventStart = Platform::GetTime();

		RadianceContext *c = pRadianceBuffer->GetP(0, 0);
		for (int j = pRadianceBuffer->GetArea(); j > 0; j--, c++)
			c->Flags = 0;

		// Render phase
		renderThreadState.Reset();
		renderThreadState.Wait();
		renderThreadState.Wait();

		// Time radiance computation event
		eventComplete = Platform::GetTime();
		fFrameRadianceTime = Platform::ToSeconds(eventComplete - eventStart);
		fTotalRadianceTime += fFrameRadianceTime;

		//----------------------------------------------------------------------------------------------
		// Post processing 
		//----------------------------------------------------------------------------------------------
		eventStart = Platform::GetTime();

		// Reconstruction
		if (bFrameReconstructionEnabled) 
		{
			pReconstructionBuffer->Apply(pRadianceBuffer, pRadianceTemporalBuffer);
			memcpy((void*)pRadianceBuffer->GetBuffer(), (void*)pRadianceTemporalBuffer->GetBuffer(), sizeof(RadianceContext) * pRadianceTemporalBuffer->GetArea());

			// pReconstructionBuffer->Apply(pRadianceBuffer, pRadianceBuffer);
		}
		
		if (bBilateralFilterEnabled)
			pBilateralFilter->Apply(pRadianceBuffer, pRadianceBuffer);

		// Discontinuity
		if (bDiscontinuityBufferEnabled)
			pDiscontinuityBuffer->Apply(pRadianceBuffer, pRadianceBuffer);

		// Accumulation
		if (bAccumulationBufferEnabled)
		{
			//pAccumulationBuffer->Reset();
			pAccumulationBuffer->Apply(pRadianceBuffer, pRadianceBuffer);
		}
		else
			pEnvironment->GetScene()->GetSampler()->Reset();		

		// Tonemapping
		if (bToneMappingEnabled)
			pGlobalTone->Apply(pRadianceBuffer, pRadianceBuffer);

		// Time radiance computation event
		eventComplete = Platform::GetTime();
		fFramePostProcessingTime = Platform::ToSeconds(eventComplete - eventStart);
		fTotalPostProcessingTime += fFramePostProcessingTime;

		//----------------------------------------------------------------------------------------------
		// Output 
		//----------------------------------------------------------------------------------------------
		eventStart = Platform::GetTime();
				
		// Commit frame
		if (!bOutputToNullDeviceEnabled)
			pRenderer->Commit(pRadianceBuffer);

		// Time radiance computation event
		eventComplete = Platform::GetTime();
		fFrameCommitTime = Platform::ToSeconds(eventComplete - eventStart);
		fTotalCommitTime += fFrameCommitTime;

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
		if (p_bVerbose && nFramesProcessed % p_nVerboseFrequency == 0)
		{
			std::cout << std::endl << "-- Frame " << nFrame << std::endl;
			std::cout << "--- Integrator Preparation Time : [" << fFrameIntegratorTime << "s]" << std::endl;
			std::cout << "--- Space Update Time : [" << fFrameSpaceTime << "s]" << std::endl;
			std::cout << "--- Radiance Computation Time : [" << fFrameRadianceTime << "s]" << std::endl;
			std::cout << "--- Post-processing Time : [" << fFramePostProcessingTime << "s]" << std::endl;
			std::cout << "--- Output Time : [" << fFrameCommitTime << "s]" << std::endl;
			std::cout << "--- Frame Render Time : [" << fFrameTime << "s]" << std::endl;
			std::cout << "--- Frames per second : [" << fTotalFramesPerSecond / nFramesProcessed << "]" << std::endl;
		}
	}

	//----------------------------------------------------------------------------------------------
	// Stop rendering threads
	//----------------------------------------------------------------------------------------------
	renderThreadState.Stop();

	//----------------------------------------------------------------------------------------------
	// Output averaged timings
	//----------------------------------------------------------------------------------------------
	if (p_bVerbose)
	{
		std::cout << std::endl << "-- Average timings over [" << nFramesProcessed << "] frames" << std::endl;
		std::cout << "--- Integrator Preparation Time : [" << fTotalIntegratorTime / nFramesProcessed << "s]" << std::endl;
		std::cout << "--- Space Update Time : [" << fTotalSpaceTime / nFramesProcessed << "s]" << std::endl;
		std::cout << "--- Radiance Computation Time : [" << fTotalRadianceTime / nFramesProcessed << "s]" << std::endl;
		std::cout << "--- Post-processing Time : [" << fTotalPostProcessingTime / nFramesProcessed << "s]" << std::endl;
		std::cout << "--- Output Time : [" << fTotalCommitTime / nFramesProcessed << "s]" << std::endl;
		std::cout << "--- Frame Render Time : [" << fTotalTime / nFramesProcessed << "s]" << std::endl;
		std::cout << "--- Frames per second : [" << fTotalFramesPerSecond / nFramesProcessed << "]" << std::endl << std::endl;
	}

	// Output per thread statistics
	std::ofstream threadTimings;
	threadTimings.open("threadTimings.txt", std::ios::binary);
	threadTimings << renderThreadState.GetStatistics() << std::endl;
	threadTimings.close();

	//----------------------------------------------------------------------------------------------
	// Shutdown system
	//----------------------------------------------------------------------------------------------
	
	// Close output device
	pRenderer->GetDevice()->Close();

	// Shutdown renderer and integrator
	pRenderer->Shutdown();
	pIntegrator->Shutdown();

	// Shutdown sandbox
	sandbox.Shutdown(p_bVerbose);

	//----------------------------------------------------------------------------------------------
	if (p_bVerbose)
	{
		Logger::Message("Complete :: Press enter to continue", true); std::getchar();
	}
}
//----------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	std::cout << "Illumina Renderer : Version " << Illumina::Core::Major << "." << Illumina::Core::Minor << "." << Illumina::Core::Build << " http://www.illuminaprt.codeplex.com " << std::endl;
	std::cout << "Copyright (C) 2010-2012 Keith Bugeja" << std::endl << std::endl;

	// default options
	int nVerboseFrequency = 1,
		nIterations = 1,
		nThreads = 1,
		nSize = 32,
		nJobs = 0x10000,
		nFPS = 5,
		nFlags = 0xFF;

	bool bVerbose = false;
	std::string strScript("default.ilm");

	// Declare the supported options.
	boost::program_options::options_description description("Allowed Settings");

	description.add_options()
		("help", "show this message")
		("verbose", boost::program_options::value<bool>(), "show extended information")
		("statfreq", boost::program_options::value<int>(), "show frame statistics every nth frame (requires verbose)")
		("script", boost::program_options::value<std::string>(), "script file to render")
		("workdir", boost::program_options::value<std::string>(), "working directory")
		("iterations", boost::program_options::value<int>(), "iterations to execute")
		("threads", boost::program_options::value<int>(), "number of rendering threads")
		("tilesize", boost::program_options::value<int>(), "initial length of tile edge")
		("tilejobs", boost::program_options::value<int>(), "number of jobs before tile subdivision")
		("flags", boost::program_options::value<int>(), "rendering flags")
		("fps", boost::program_options::value<int>(), "frame rendering frequency (hint)")
		;

	// Declare variable map
	boost::program_options::variables_map variableMap;

	// Parse command line options
	try 
	{
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, description), variableMap);
		boost::program_options::notify(variableMap);
	} 
	catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::unknown_option> > &exception) 
	{
		std::cout << "Unknown option [" << exception.get_option_name() << "] : Please use --help to display help message." << std::endl;
		return 1;
	}
	catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::invalid_option_value> > &exception) 
	{
		std::cout << "Error parsing input for [" << exception.get_option_name() << "] : Invalid argument value." << std::endl;
		return 1;
	}

	// --help
	if (variableMap.count("help"))
	{
		std::cout << description << std::endl;
		return 1;
	}

	// --verbose
	if (variableMap.count("verbose"))
	{
		bVerbose = variableMap["verbose"].as<bool>();
		std::cout << "Verbose mode [" << (bVerbose ? "ON]" : "OFF]") << std::endl;
	}

	// --statfreq
	if (variableMap.count("statfreq"))
	{
		try {
			nVerboseFrequency = variableMap["statfreq"].as<int>();
		} catch (...) { nVerboseFrequency = 1; } 
		std::cout << "Render statistics output frequency [" << nIterations << "]" << std::endl;
	}

	// --iterations
	if (variableMap.count("iterations"))
	{
		try {
			nIterations = variableMap["iterations"].as<int>();
		} catch (...) { nIterations = 1; } 
		std::cout << "Iterations [" << nIterations << "]" << std::endl;
	}

	// --script
	if (variableMap.count("script"))
	{
		strScript = variableMap["script"].as<std::string>();
		std::cout << "Script [" << strScript << "]" << std::endl;
	}

	// --workdir
	boost::filesystem::path cwdPath;

	if (variableMap.count("workdir"))
	{
		cwdPath = boost::filesystem::path(variableMap["workdir"].as<std::string>());
	}
	else
	{
		// Setting working directory
		boost::filesystem::path scriptPath(strScript);
		cwdPath = boost::filesystem::path(scriptPath.parent_path());
	}

	try {
		boost::filesystem::current_path(cwdPath);
		std::cout << "Working directory [" << cwdPath.string() << "]" << std::endl;;
	} catch (...) { std::cerr << "Error : Unable to set working directory to " << cwdPath.string() << std::endl; }

	// --threads
	if (variableMap.count("threads"))
	{
		try {
			nThreads = variableMap["threads"].as<int>();
		} catch (...) { nThreads = 1; } 
		std::cout << "Threads [" << nThreads << "]" << std::endl;
	}

	// --width
	if (variableMap.count("tilesize"))
	{
		try {
			nSize = variableMap["tilesize"].as<int>();
		} catch (...) { nSize = 16; } 
		std::cout << "Tile size [" << nSize << " x " << nSize << "]" << std::endl;
	}

	// --threads
	if (variableMap.count("tilejobs"))
	{
		try {
			nJobs = variableMap["tilejobs"].as<int>();
		} catch (...) { nJobs = 0x10000; } 
		std::cout << "Jobs before tile subdivision [" << nJobs << "]" << std::endl;
	}

	// --budget
	if (variableMap.count("fps"))
	{
		try {
			nFPS = variableMap["fps"].as<int>();
		} catch (...) { nFPS = 0; } 
		std::cout << "FPS [" << nFPS << "]" << std::endl;
	}

	// --flags
	if (variableMap.count("flags"))
	{
		try {
			nFlags = variableMap["flags"].as<int>();
		} catch (...) { nFlags = 0x01 | 0x02 | 0x04 | 0x08; } 
		std::cout << "Flags [" << nFlags << "]" << std::endl;
	}

	// -- start rendering
	IlluminaPRT(bVerbose, nVerboseFrequency, nIterations, nThreads, nFPS, nJobs, nSize, nFlags, strScript);

	// Exit
	return 1;
}

//----------------------------------------------------------------------------------------------
#elif (defined ILLUMINA_DSM)
//----------------------------------------------------------------------------------------------
#include "ServiceManager.h"

int main(int argc, char** argv)
{
	std::cout << "Illumina PRT : Version " << Major << "." << Minor << "." << Build << " http://www.illuminaprt.codeplex.com " << std::endl;
	std::cout << "Copyright (C) 2010-2012 Keith Bugeja" << std::endl << std::endl;

	// default options
	int nPort = 6660,
		nAdminPort = 6661;

	std::string strPath;
	bool bVerbose = false;

	// Declare the supported options.
	boost::program_options::options_description description("Allowed Settings");

	description.add_options()
		("help", "show this message")
		("verbose", boost::program_options::value<bool>(), "show extended information")
		("workdir", boost::program_options::value<std::string>(), "working directory")
		("port", boost::program_options::value<int>(), "service port")
		("adminport", boost::program_options::value<int>(), "admin port")
		;

	// Declare variable map
	boost::program_options::variables_map variableMap;

	// Parse command line options
	try 
	{
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, description), variableMap);
		boost::program_options::notify(variableMap);
	} 
	catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::unknown_option> > &exception) 
	{
		std::cout << "Unknown option [" << exception.get_option_name() << "] : Please use --help to display help message." << std::endl;
		return 1;
	}
	catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::invalid_option_value> > &exception) 
	{
		std::cout << "Error parsing input for [" << exception.get_option_name() << "] : Invalid argument value." << std::endl;
		return 1;
	}

	// --help
	if (variableMap.count("help"))
	{
		std::cout << description << std::endl;
		return 1;
	}

	// --verbose
	if (variableMap.count("verbose"))
	{
		bVerbose = variableMap["verbose"].as<bool>();
		std::cout << "Verbose mode [" << (bVerbose ? "ON]" : "OFF]") << std::endl;
	}

	// --port
	if (variableMap.count("port"))
	{
		try {
			nPort = variableMap["port"].as<int>();
		} catch (...) { nPort = 6660; } 
		std::cout << "Port [" << nPort << "]" << std::endl;
	}

	// --adminport
	if (variableMap.count("adminport"))
	{
		try {
			nAdminPort = variableMap["adminport"].as<int>();
		} catch (...) { nAdminPort = 6661; } 
		std::cout << "Admin Port [" << nAdminPort << "]" << std::endl;
	}

	// --workdir
	if (variableMap.count("workdir"))
	{
		strPath = variableMap["workdir"].as<std::string>();
	}

	// -- start service
	ServiceManager *pServiceManager = ServiceManager::GetInstance();
	
	pServiceManager->Initialise(nPort, nAdminPort, strPath, bVerbose);
	pServiceManager->Start();
	pServiceManager->Shutdown();
		
	return 0;
}
#endif