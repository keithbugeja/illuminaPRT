//----------------------------------------------------------------------------------------------
//	Filename:	RenderTaskCoordinator.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include <boost/algorithm/string.hpp> 

#include "RenderTaskCoordinator.h"
#include "Communicator.h"
//----------------------------------------------------------------------------------------------
#define __JOB_TIMING_FINE		0
#define __JOB_TIMING_COARSE		1

#define __JOB_TIMING_METHOD __JOB_TIMING_COARSE
#define __JOB_TIMING
//----------------------------------------------------------------------------------------------
#if defined __JOB_TIMING
	#define StartTimer(x) x = Platform::GetTime()
	#define GetElapsed(x,y) Platform::ToSeconds((y = Platform::GetTime()) - x)
	#define RestartTimer(x,y) x = y
#else
	#define StartTimer(x)
	#define GetElapsed(x,y) 0
	#define RestartTimer(x,y)
#endif

#if __JOB_TIMING_METHOD == _JOB_TIMING_FINE
	#define StartTimerF(x) StartTimer(x)
	#define GetElapsedF(x,y) GetElapsed(x,y)
	#define RestartTimerF(x,y) RestartTimer(x,y)
#else
	#define StartTimerF(x)
	#define GetElapsedF(x,y) 0
	#define RestartTimerF(x,y)
#endif

//----------------------------------------------------------------------------------------------
static const int ____seed = 9843;
static const int ____seed_inc = 137;
//----------------------------------------------------------------------------------------------
bool RenderTaskCoordinator::OnInitialise(void) 
{
	// Get logger instance
	std::stringstream messageLog;
	Logger *logger = ServiceManager::GetInstance()->GetLogger();
	logger->Write("RenderTaskCoordinator :: Handling event [OnInitialise].", LL_Info);

	//----------------------------------------------------------------------------------------------
	// Initialise and load sandbox environment
	//----------------------------------------------------------------------------------------------
	std::string strScriptName;
	ArgumentMap *pArgumentMap = GetArgumentMap();

	m_pSandbox = new SandboxEnvironment();
	m_pSandbox->Initialise();

	if (!pArgumentMap->GetArgument(__TaskID, m_nTaskId))
	{
		logger->Write("RenderTaskCoordinator :: Unable to determine task id ["__TaskID"] from argument list.", LL_Error);
		return false;
	}

	if (!pArgumentMap->GetArgument(__Job_User, m_strUserName) || !pArgumentMap->GetArgument(__Job_Name, m_strJobName))
	{
		logger->Write("RenderTaskCoordinator :: Unable to determine task details (["__Job_User"] or ["__Job_Name"]) from argument list.", LL_Error);
		return false;
	}

	if (!pArgumentMap->GetArgument(__Script_Name, strScriptName))
	{
		logger->Write("RenderTaskCoordinator :: Unable to find ["__Script_Name"] entry in argument list.", LL_Error);
		return false;
	}

	if (!m_pSandbox->LoadScene(strScriptName))
	{
		messageLog << "RenderTaskCoordinator :: Unable to load scene script [" << strScriptName << "].";
		logger->Write(messageLog.str(), LL_Error);
		return false;
	}
		
	messageLog << "RenderTaskCoordinator :: Scene script [" << strScriptName << "] loaded.";
	
	//----------------------------------------------------------------------------------------------
	// Initialise render task
	//----------------------------------------------------------------------------------------------
	if (pArgumentMap->GetArgument(__Tile_Width, m_renderTaskContext.TileWidth) &&
		pArgumentMap->GetArgument(__Tile_Height, m_renderTaskContext.TileHeight))
	{
		m_pRenderTile = new SerialisableRenderTile(-1, m_renderTaskContext.TileWidth, m_renderTaskContext.TileHeight);
		
		messageLog << std::endl << "RenderTaskCoordinator :: Maximum tile size set to [" 
			<< m_renderTaskContext.TileWidth << " x " 
			<< m_renderTaskContext.TileHeight << "]";
	}

	//----------------------------------------------------------------------------------------------
	// Initialise render parameters
	//----------------------------------------------------------------------------------------------
	// Enable adaptive tile sizes
	std::string adaptiveTile; pArgumentMap->GetArgument(__Tile_Distribution_Adaptive, adaptiveTile); boost::to_lower(adaptiveTile);
	m_renderTaskContext.AdaptiveTiles = adaptiveTile == "false" ? false : true;

	// Enable batch size
	pArgumentMap->GetArgument(__Tile_Distribution_Batchsize, m_renderTaskContext.TileBatchSize);

	// Read the minimum number of required workers
	if (pArgumentMap->GetArgument(__Resource_Cap_Min, m_renderTaskContext.WorkersRequired))
		messageLog << std::endl << "RenderTaskCoordinator :: Workers required [" << m_renderTaskContext.WorkersRequired << "]";

	//----------------------------------------------------------------------------------------------
	// Initialise engine and environment
	//----------------------------------------------------------------------------------------------
	m_pEnvironment = m_pSandbox->GetEnvironment();
	m_pEngineKernel = m_pSandbox->GetEngineKernel();

	// Alias 
	m_pIntegrator = m_pEnvironment->GetIntegrator();
	m_pRenderer = m_pEnvironment->GetRenderer();
	m_pCamera = m_pEnvironment->GetCamera();
	m_pSpace = m_pEnvironment->GetSpace();

	//----------------------------------------------------------------------------------------------
	// Are we overriding the output device in the script file?
	//----------------------------------------------------------------------------------------------
	std::string deviceOverride; pArgumentMap->GetArgument(__Device_Override, deviceOverride); boost::to_lower(deviceOverride);
	
	if (deviceOverride == "true") {
		std::string deviceType; pArgumentMap->GetArgument(__Device_Type, deviceType); boost::to_lower(deviceType);
		
		std::string deviceString, deviceTag,
			deviceFactory, deviceId;

		IDevice *pDevice;

		// Is this a stream device we wish to override with?
		if (deviceType == "stream") 
		{
			std::stringstream deviceArguments; std::string argument;
			pArgumentMap->GetArgument(__Device_Width, argument);
			deviceArguments << "Width=" << argument;
			pArgumentMap->GetArgument(__Device_Height, argument);
			deviceArguments << ";Height=" << argument;
			pArgumentMap->GetArgument(__Device_Stream_IP, argument);
			deviceArguments << ";Address=" << argument;
			pArgumentMap->GetArgument(__Device_Stream_Port, argument);
			deviceArguments << ";Port=" << argument;
			pArgumentMap->GetArgument(__Device_Stream_Codec, argument);
			deviceArguments << ";Format=" << argument;
			pArgumentMap->GetArgument(__Device_Stream_Bitrate, argument);
			deviceArguments << ";BitRate=" << argument;
			pArgumentMap->GetArgument(__Device_Stream_Framerate, argument);
			deviceArguments << ";FrameRate=" << argument;
			deviceArguments << ";";

			deviceId = "__Override_Coordinator_Stream_Device";
			deviceFactory = "RTP";
			deviceString = deviceArguments.str();
		} 
		else if (deviceType == "sequence") 
		{
			std::string prefix;

			std::stringstream deviceArguments; std::string argument;
			pArgumentMap->GetArgument(__Device_Width, argument);
			deviceArguments << "Width=" << argument;
			pArgumentMap->GetArgument(__Device_Height, argument);
			deviceArguments << ";Height=" << argument;
			pArgumentMap->GetArgument(__Device_Sequence_BufferedFrames, argument);
			deviceArguments << ";BufferSize=" << argument;
			pArgumentMap->GetArgument(__Device_Sequence_Prefix, prefix);
			deviceArguments << ";Filename=" << prefix;
			pArgumentMap->GetArgument(__Device_Sequence_Format, argument);
			deviceArguments << "." << argument;
			deviceArguments << ";Format=" << argument;
			deviceArguments << ";";

			deviceId = "__Override_Coordinator_Sequence_Device";
			deviceFactory = "BufferedImage";
			deviceString = deviceArguments.str();

			// Set device tag
			pArgumentMap->GetArgument(__Device_Sequence_Details, deviceTag); boost::to_lower(deviceTag);
			
			if (deviceTag == "false")
				deviceTag = prefix;
			else
				deviceTag = prefix + m_strUserName + "_" + m_strJobName + "_";
		} 
		else if (deviceType == "image")
		{
			std::stringstream deviceArguments; std::string argument;
			pArgumentMap->GetArgument(__Device_Width, argument);
			deviceArguments << "Width=" << argument;
			pArgumentMap->GetArgument(__Device_Height, argument);
			deviceArguments << ";Height=" << argument;
			pArgumentMap->GetArgument(__Device_Image_Timestamp, argument);
			deviceArguments << ";Timestamp=" << argument;
			pArgumentMap->GetArgument(__Device_Image_Prefix, deviceTag);
			deviceArguments << ";Filename=" << deviceTag;
			pArgumentMap->GetArgument(__Device_Image_Format, argument);
			deviceArguments << "." << argument;
			deviceArguments << ";Format=" << argument;
			deviceArguments << ";";

			deviceId = "__Override_Coordinator_Image_Device";
			deviceFactory = "Image";
			deviceString = deviceArguments.str();
		}

		// Create device and set tag
		pDevice = m_pEngineKernel->GetDeviceManager()->CreateInstance(deviceFactory, deviceId, deviceString);
		pDevice->SetTag(deviceTag);

		// Associate device with renderer
		m_pRenderer->SetDevice(pDevice);
	}

	//----------------------------------------------------------------------------------------------
	// Initialise radiance buffers and post-processing filters
	//----------------------------------------------------------------------------------------------
	m_pRadianceBuffer = new RadianceBuffer(m_pRenderer->GetDevice()->GetWidth(), m_pRenderer->GetDevice()->GetHeight()),

	// Discontinuity, reconstruction and tone mapping
	m_pBilateralFilter = m_pEngineKernel->GetPostProcessManager()->CreateInstance("BilateralFilter", "BilateralFilter", "");
	m_pDiscontinuityBuffer = m_pEngineKernel->GetPostProcessManager()->CreateInstance("Discontinuity", "DiscontinuityBuffer", "");
	m_pReconstructionBuffer = m_pEngineKernel->GetPostProcessManager()->CreateInstance("Reconstruction", "ReconstructionBuffer", "");
	m_pTonemapFilter = m_pEngineKernel->GetPostProcessManager()->CreateInstance("GlobalTone", "GlobalTone", "");

	// Motion blur filter
	m_pMotionBlurFilter = (MotionBlur*)m_pEngineKernel->GetPostProcessManager()->CreateInstance("MotionBlur", "MotionBlur", "");
	m_pMotionBlurFilter->SetExternalBuffer(m_pRadianceBuffer, 6);
	m_pMotionBlurFilter->Reset();

	// Open rendering device for output
	//	m_pRenderer->GetDevice()->SetTag(m_strUserName + "_" + m_strJobName + "_");
	m_pRenderer->GetDevice()->Open();

	//----------------------------------------------------------------------------------------------
	// Set up render task context
	//----------------------------------------------------------------------------------------------
	m_renderTaskContext.FrameWidth = m_pRenderer->GetDevice()->GetWidth();
	m_renderTaskContext.FrameHeight = m_pRenderer->GetDevice()->GetHeight();
	m_renderTaskContext.TilesPerRow = m_renderTaskContext.FrameWidth / m_renderTaskContext.TileWidth;
	m_renderTaskContext.TilesPerColumn = m_renderTaskContext.FrameHeight / m_renderTaskContext.TileHeight;
	m_renderTaskContext.TotalTiles = m_renderTaskContext.TilesPerColumn * m_renderTaskContext.TilesPerRow;
		
	if (m_renderTaskContext.AdaptiveTiles)
	{
		messageLog << std::endl << "RenderTaskCoordinator :: Adaptive tiling enabled with batch size [" << m_renderTaskContext.TileBatchSize << "]";

		m_renderTaskContext.TilePackets.GeneratePackets(m_renderTaskContext.FrameWidth, m_renderTaskContext.FrameHeight,
			Maths::Max(m_renderTaskContext.TileWidth, m_renderTaskContext.TileHeight), m_renderTaskContext.TileBatchSize);
	} 
	else
	{
		messageLog << std::endl << "RenderTaskCoordinator :: Adaptive tiling disabled.";

		m_renderTaskContext.TilePackets.GeneratePackets(m_renderTaskContext.FrameWidth, m_renderTaskContext.FrameHeight,
			Maths::Max(m_renderTaskContext.TileWidth, m_renderTaskContext.TileHeight), 10000);
	}

	// Create receive buffers for each tile
	m_renderTileBuffer.clear();

	for (int packetIndex = 0; packetIndex < m_renderTaskContext.TilePackets.GetPacketCount(); packetIndex++)
		m_renderTileBuffer.push_back(
			new SerialisableRenderTile(-1, m_renderTaskContext.TileWidth, m_renderTaskContext.TileHeight));

	//----------------------------------------------------------------------------------------------
	// Camera and observer details 
	//----------------------------------------------------------------------------------------------
	// clear camera path
	m_cameraPath.Clear();

	// update observer position
	m_observerPosition	= m_pEnvironment->GetCamera()->GetObserver();
	m_observerTarget	= m_pEnvironment->GetCamera()->GetFrame().GetW();
	m_moveFlag[0] = m_moveFlag[1] = m_moveFlag[2] = m_moveFlag[3] = 0;
	
	m_bResetAccumulation = false;
	m_bResetWorkerSeed = true;

	//----------------------------------------------------------------------------------------------
	// kick off input thread
	//----------------------------------------------------------------------------------------------
	boost::thread inputThreadHandler = 
		boost::thread(boost::bind(RenderTaskCoordinator::InputThreadHandler, this));

	boost::thread decompressionThreadHandler =
		boost::thread(boost::bind(RenderTaskCoordinator::DecompressionThreadHandler, this));

	// Log accumulated messages
	logger->Write(messageLog.str(), LL_Info);

	// Add custom channel to logger
	m_pSink = new AsynchronousFileSink(m_strUserName + "_" + m_strJobName + "_timings.txt");
	m_pChannel = new LoggerChannel("data_collection");
	m_pChannel->AddSink(m_pSink);
	logger->AddChannel("data_collection", m_pChannel, false);

	// Add header to log file
	(*logger)["data_collection"]->Write("Time \t Radiance \t Accumulation \t Commit \t Cycle Time \t Cycle Hz \t Frame Time \t Frame Hz \t Workers \n", LL_All);
	
	m_checkPointTime = 
		m_bootTime = Platform::GetTime();

	return true;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
void RenderTaskCoordinator::OnShutdown(void)
{
	ServiceManager::GetInstance()->GetLogger()->Write("RenderTaskCoordinator :: Handling event [OnShutdown].", LL_Info);
	
	//----------------------------------------------------------------------------------------------
	// Delete channel and sink
	//----------------------------------------------------------------------------------------------
	delete m_pChannel;
	delete m_pSink;

	//----------------------------------------------------------------------------------------------
	// Wake up decompression thread, if sleeping and force quit (set packed tiles to zero)
	//----------------------------------------------------------------------------------------------
	m_nTilesPacked = 0; m_decompressionQueueCV.notify_all();

	//----------------------------------------------------------------------------------------------
	// Close rendering device
	//----------------------------------------------------------------------------------------------
	m_pRenderer->GetDevice()->Close();

	//----------------------------------------------------------------------------------------------
	// Shutdown renderer, integrator
	//----------------------------------------------------------------------------------------------
	m_pRenderer->Shutdown();
	m_pIntegrator->Shutdown();

	//----------------------------------------------------------------------------------------------
	// Delete radiance buffers
	//----------------------------------------------------------------------------------------------
	delete m_pRadianceBuffer;

	//----------------------------------------------------------------------------------------------
	// Shutdown and delete sandbox
	//----------------------------------------------------------------------------------------------
	m_pSandbox->Shutdown();
	delete m_pSandbox;

	//----------------------------------------------------------------------------------------------
	// Delete serialisable tile
	//----------------------------------------------------------------------------------------------
	delete m_pRenderTile;

	//----------------------------------------------------------------------------------------------
	// Clear tile buffer
	//----------------------------------------------------------------------------------------------
	for (std::vector<SerialisableRenderTile*>::iterator packetIterator = m_renderTileBuffer.begin();
		 packetIterator != m_renderTileBuffer.end(); ++packetIterator)
		 delete (*packetIterator);

	m_renderTileBuffer.clear();
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
bool RenderTaskCoordinator::OnSynchronise(void) 
{
	SynchronisePacket syncPacket;

	// Actual seed is now passed instead of reset flag
	if (m_bResetWorkerSeed) {
		m_seed = ____seed;
		m_bResetWorkerSeed = false;
	} else {
		m_seed += ____seed_inc;
	}

	syncPacket.seed = m_seed;
	syncPacket.observerPosition = m_observerPosition;
	syncPacket.observerTarget = m_observerTarget;

	int synchronisePacketSize = sizeof(SynchronisePacket);

	// Get list of available workers
	std::vector<int> &workerList 
		= GetAvailableWorkerList();

	// Send first batch of jobs
	for (std::vector<int>::iterator workerIterator = workerList.begin();
		 workerIterator != workerList.end(); workerIterator++)
	{
		Communicator::Send(&synchronisePacketSize, sizeof(int), *workerIterator, Communicator::Coordinator_Worker_Job);
		Communicator::Send(&syncPacket, sizeof(SynchronisePacket), *workerIterator, Communicator::Coordinator_Worker_Job);
	}

	return true;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
bool RenderTaskCoordinator::OnSynchroniseAbort(void)
{
	int abortSignal = -1;

	// Get list of available workers
	std::vector<int> &workerList 
		= GetAvailableWorkerList();

	// Send first batch of jobs
	for (std::vector<int>::iterator workerIterator = workerList.begin();
		 workerIterator != workerList.end(); workerIterator++)
	{
		Communicator::Send(&abortSignal, sizeof(int), *workerIterator, Communicator::Coordinator_Worker_Job);
	}

	return true;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
bool RenderTaskCoordinator::OnMessageReceived(ResourceMessage *p_pMessage) 
{
	// Generic message 
	Message_Controller_Resource_Generic *pMessage = (Message_Controller_Resource_Generic*)p_pMessage->Content;

	// Transform to argument map
	std::string command, action;
	ArgumentMap arg(pMessage->String);
	arg.GetArgument("command", command);

	// Quick and dirty parsing of move command 
	if (command == "move")
	{
		int direction;

		arg.GetArgument("action", action);
		arg.GetArgument("direction", direction);

		if (action == "start")
			m_moveFlag[direction] = 1;
		else
			m_moveFlag[direction] = 0;
	}
	else if (command == "path")
	{
		m_cameraPath.Reset();

		std::vector<Vector3> pointList;
		arg.GetArgument("vertices", pointList);
	
		for (std::vector<Vector3>::iterator it = pointList.begin(); 
			 it != pointList.end(); it++)
		{
			m_cameraPath.AddVertex(*it);
		}

		m_cameraPath.PreparePath();
	}

	return true; 
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
bool RenderTaskCoordinator::Compute(void) 
{
	SerialisableRenderTile *pRenderTile = NULL;
	RadianceContext *pTileBuffer;
	Communicator::Status status;

#if defined __JOB_TIMING
	double eventStart, eventComplete, 
		subEventStart, subEventComplete;

	double frameTime;

	double radianceTime,
		bilateralTime,
		discontinuityTime,
		toneTime,
		commitTime,
		accumulationTime;
#endif
	
	int receivedTileID, tileID,
		framePackageSize, waitingTasks;

	//----------------------------------------------------------------------------------------------
	// Initialise
	//----------------------------------------------------------------------------------------------
	tileID = m_renderTaskContext.TilePackets.GetPacketCount() - 1;

	framePackageSize = 0;
	waitingTasks = 0;

	// Initialise asynchronous decompression variables
	m_nProducerIndex = 0;
	m_nConsumerIndex = 0;
	m_nTilesPacked = 0;

	//----------------------------------------------------------------------------------------------
	// Get list of available workers for this frame and send the initial job batch
	//----------------------------------------------------------------------------------------------
	StartTimer(eventStart);
	
	std::vector<int> &workerList 
		= GetAvailableWorkerList();

	// Send first batch of jobs
	for (std::vector<int>::iterator workerIterator = workerList.begin();
		 workerIterator != workerList.end(); workerIterator++)
	{
		Communicator::Send(&tileID, sizeof(int), *workerIterator, Communicator::Coordinator_Worker_Job);

		tileID--; waitingTasks++;
		
		if (tileID < 0)
			break;
	}

	//----------------------------------------------------------------------------------------------
	// Wait for results and send new task, if available
	//----------------------------------------------------------------------------------------------
	while (waitingTasks > 0)
	{
		// Probe for incoming message
		Communicator::Probe(Communicator::Source_Any, Communicator::Worker_Coordinator_Job, &status);

		// Receive compressed tile
		pRenderTile = m_renderTileBuffer[m_nProducerIndex++]; 

		Communicator::Receive(pRenderTile->GetTransferBuffer(), 
			Communicator::GetSize(&status), status.MPI_SOURCE,
			Communicator::Worker_Coordinator_Job);

		// Receive complete -> Inform consumer
		AtomicInt32::Increment((Int32*)&m_nTilesPacked);
		m_decompressionQueueCV.notify_one();

		// Add frame size
		framePackageSize += Communicator::GetSize(&status);

		if (tileID >= 0)
		{
			// Acknowledge receipt and send new job
			Communicator::Send(&tileID, sizeof(int), status.MPI_SOURCE, Communicator::Coordinator_Worker_Job);
			tileID--;
		}
		else
		{
			// Communicate no more jobs
			int finalTaskID = -1;

			Communicator::Send(&finalTaskID, sizeof(int), status.MPI_SOURCE, Communicator::Coordinator_Worker_Job);
			waitingTasks--;
		}
	}

	// Compute time elapsed for radiance computation (communication + rendering)
	radianceTime = GetElapsedF(eventStart, subEventComplete);	

	//----------------------------------------------------------------------------------------------
	// Additional post-processing filters
	//----------------------------------------------------------------------------------------------
	// Bilateral filter
	RestartTimerF(subEventStart, subEventComplete);
	//m_pBilateralFilter->Apply(m_pRadianceBuffer, m_pRadianceBuffer);
	bilateralTime = GetElapsedF(subEventStart, subEventComplete);

	// Discontinuity buffer
	RestartTimerF(subEventStart, subEventComplete);
	//m_pDiscontinuityBuffer->Apply(m_pRadianceBuffer, m_pRadianceBuffer);
	discontinuityTime = GetElapsedF(subEventStart, subEventComplete);

	// Tone mapping (moved to worker)
	RestartTimerF(subEventStart, subEventComplete);
	//m_pTonemapFilter->Apply(m_pRadianceBuffer, m_pRadianceBuffer);
	toneTime = GetElapsedF(subEventStart, subEventComplete);

	// Accumulation and motion blur
	RestartTimerF(subEventStart, subEventComplete);
	m_pMotionBlurFilter->Apply(m_pRadianceBuffer, m_pRadianceBuffer);
	if (m_bResetAccumulation) 
	{
		m_pMotionBlurFilter->Reset();
		m_bResetAccumulation = false;
	}
	accumulationTime = GetElapsedF(subEventStart, subEventComplete);

	// Commit to device
	RestartTimerF(subEventStart, subEventComplete);
	m_pRenderer->Commit(m_pRadianceBuffer);
	commitTime = GetElapsedF(subEventStart, subEventComplete);

	// Get frame time
	frameTime = GetElapsed(eventStart, eventComplete);

	//----------------------------------------------------------------------------------------------
	// Show frame time breakdown  
	//----------------------------------------------------------------------------------------------
	#if defined __JOB_TIMING
		std::stringstream statistics;

		double elapsedFromLastCheckpoint = Platform::ToSeconds(eventComplete - m_checkPointTime);
		m_checkPointTime = eventComplete;

		statistics << "RenderTaskCoordinator :: Computation statistics for task [" << m_nTaskId << "] : " 
		#if __JOB_TIMING_METHOD == __JOB_TIMING_FINE
			<< std::endl << "\t---| Radiance Time : " << radianceTime << "s"
			<< std::endl << "\t---| Bilateral Time : " << bilateralTime << "s"
			<< std::endl << "\t---| Discontinuity Time : " << discontinuityTime << "s"
			<< std::endl << "\t---| Tonemapping Time : " << toneTime << "s"
			<< std::endl << "\t---| Accumulation Time : " << accumulationTime << "s"
			<< std::endl << "\t---| Commit Time : " << commitTime << "s"
		#endif
			<< std::endl << "\t---| Frame Time : " << frameTime << "s" 
			<< std::endl << "\t---| Frame Rate : " << 1.0 / frameTime << "Hz"
			<< std::endl;

		ServiceManager::GetInstance()->GetLogger()->Write(statistics.str(), LL_Info);

		// Log output to data collection channel (asynchronous file-output channel)
		statistics.str(std::string(""));

		statistics << Platform::ToSeconds(eventComplete - m_bootTime)
		#if __JOB_TIMING_METHOD == __JOB_TIMING_FINE
			<< '\t' << radianceTime << '\t' << accumulationTime << '\t' << commitTime
		#endif
			<< '\t' << elapsedFromLastCheckpoint << '\t' << 1.0 / elapsedFromLastCheckpoint
			<< '\t' << frameTime << '\t' << 1.0 / frameTime
			<< '\t' << workerList.size()
			<< std::endl;

		(*ServiceManager::GetInstance()->GetLogger())["data_collection"]->Write(statistics.str(), LL_All);
	#endif

	return true;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
void RenderTaskCoordinator::InputThreadHandler(RenderTaskCoordinator *p_pCoordinator)
{
	static float time = 0.f;

	while(p_pCoordinator->IsRunning())
	{
		if (p_pCoordinator->m_cameraPath.IsEmpty())
		{
			bool resetFlag = (p_pCoordinator->m_moveFlag[0] |
				 p_pCoordinator->m_moveFlag[1] |
				 p_pCoordinator->m_moveFlag[2] |
				 p_pCoordinator->m_moveFlag[3]) > 0;

			p_pCoordinator->m_bResetAccumulation |= resetFlag;
			p_pCoordinator->m_bResetWorkerSeed |= resetFlag;

			const OrthonormalBasis &basis = p_pCoordinator->m_pCamera->GetFrame();
			Vector3 observer = p_pCoordinator->m_pCamera->GetObserver();

			if (p_pCoordinator->m_moveFlag[0])
				observer += basis.GetU() * 0.1f;

			if (p_pCoordinator->m_moveFlag[1])
				observer -= basis.GetU() * 0.1f;
		
			if (p_pCoordinator->m_moveFlag[2])
				observer += basis.GetW() * 0.1f;
		
			if (p_pCoordinator->m_moveFlag[3])
				observer -= basis.GetW() * 0.1f;

			p_pCoordinator->m_observerTarget = observer + basis.GetW(); 
			p_pCoordinator->m_observerPosition = observer;

			p_pCoordinator->m_pCamera->MoveTo(observer);
			p_pCoordinator->m_pCamera->LookAt(p_pCoordinator->m_observerTarget);

			// std::cout << "ITH::ResetAccum(A) = " << p_pCoordinator->m_bResetAccumulationBuffer << std::endl;
		} 
		else 
		{
			time += 0.0005f;

			if (time >= 1.f) 
			{
				time = 0; 

				p_pCoordinator->m_cameraPath.Clear(); 
				p_pCoordinator->m_bResetWorkerSeed = true;
				p_pCoordinator->m_bResetAccumulation = true;
			}
			else
			{
				p_pCoordinator->m_observerPosition = p_pCoordinator->m_cameraPath.GetPosition(time - 0.001f);
				p_pCoordinator->m_observerTarget = p_pCoordinator->m_cameraPath.GetPosition(time + 0.001f);
				p_pCoordinator->m_pCamera->MoveTo(p_pCoordinator->m_observerPosition);
				p_pCoordinator->m_pCamera->LookAt(p_pCoordinator->m_observerTarget);

				p_pCoordinator->m_bResetAccumulation = true;
				p_pCoordinator->m_bResetWorkerSeed = true;

				// std::cout << p_pCoordinator->m_observerPosition.ToString() << std::endl;
			}

			// std::cout << "ITH::ResetAccum(B) = " << p_pCoordinator->m_bResetAccumulationBuffer << std::endl;
		}

		boost::this_thread::sleep(boost::posix_time::millisec(20));
	}
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
void RenderTaskCoordinator::DecompressionThreadHandler(RenderTaskCoordinator *p_pCoordinator)
{
	ServiceManager::GetInstance()->GetLogger()->Write("RenderTaskCoordinator :: Decompression thread started.", LL_Info);
	/* */

	int receivedTileID;
	RadianceContext *pTileBuffer;
	SerialisableRenderTile *pRenderTile;	

	p_pCoordinator->m_nTilesPacked = 0;
	p_pCoordinator->m_nConsumerIndex = 0;

	boost::mutex bufferMutex;
	boost::unique_lock<boost::mutex> bufferLock(bufferMutex);

	while(p_pCoordinator->IsRunning())
	{
		while(p_pCoordinator->m_nTilesPacked == 0 && p_pCoordinator->IsRunning()) {
			p_pCoordinator->m_decompressionQueueCV.wait(bufferLock);
		}

		// TODO: Check this shit out or it's going to blow in our faces. Goddamnit.
		if (p_pCoordinator->IsRunning() == false) break;

		pRenderTile = p_pCoordinator->m_renderTileBuffer[p_pCoordinator->m_nConsumerIndex++];
		AtomicInt32::Decrement((Int32*)&(p_pCoordinator->m_nTilesPacked));

		// Decompress current tile
		pRenderTile->Unpackage();
		
		// Get image data and copy to buffer
		receivedTileID = pRenderTile->GetID();
		pTileBuffer = pRenderTile->GetImageData()->GetBuffer(); 

		RenderTilePackets::Packet packet = p_pCoordinator->m_renderTaskContext.TilePackets.GetPacket(receivedTileID);
		int sx = packet.XStart,
			sy = packet.YStart,
			sxe = sx + packet.XSize,
			sye = sy + packet.YSize;
		
		for (int y = sy; y < sye; y++) 
		{
			for (int x = sx; x < sxe; x++) 
			{
				p_pCoordinator->m_pRadianceBuffer->Set(x, y, *pTileBuffer);
				
				pTileBuffer++;
			}
		}
	}

	/* */
	ServiceManager::GetInstance()->GetLogger()->Write("RenderTaskCoordinator :: Decompression thread completed.", LL_Info);
}