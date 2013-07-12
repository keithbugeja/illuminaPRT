//----------------------------------------------------------------------------------------------
//	Filename:	AsyncRenderTaskCoordinator.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include <boost/algorithm/string.hpp> 

#include "AsyncRenderTaskCoordinator.h"
#include "Communicator.h"
//----------------------------------------------------------------------------------------------
#define __POSTPROC_MOTIONBLUR
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
bool AsyncRenderTaskCoordinator::OnInitialise(void) 
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
		logger->Write("RenderTaskCoordinator :: Unable to determine task id [" __TaskID "] from argument list.", LL_Error);
		return false;
	}

	if (!pArgumentMap->GetArgument(__Job_User, m_strUserName) || !pArgumentMap->GetArgument(__Job_Name, m_strJobName))
	{
		logger->Write("RenderTaskCoordinator :: Unable to determine task details ([" __Job_User "] or [" __Job_Name "]) from argument list.", LL_Error);
		return false;
	}

	if (!pArgumentMap->GetArgument(__Script_Name, strScriptName))
	{
		logger->Write("RenderTaskCoordinator :: Unable to find [" __Script_Name "] entry in argument list.", LL_Error);
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
	m_pRadianceBuffer = new RadianceBuffer(m_pRenderer->GetDevice()->GetWidth(), m_pRenderer->GetDevice()->GetHeight());
	m_pRadianceOutput = new RadianceBuffer(m_pRenderer->GetDevice()->GetWidth(), m_pRenderer->GetDevice()->GetHeight());

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
		boost::thread(boost::bind(AsyncRenderTaskCoordinator::InputThreadHandler, this));

	boost::thread decompressionThreadHandler =
		boost::thread(boost::bind(AsyncRenderTaskCoordinator::DecompressionThreadHandler, this));

	boost::thread computeThreadHandler = 
		boost::thread(boost::bind(AsyncRenderTaskCoordinator::ComputeThreadHandler, this));

	// Log accumulated messages
	logger->Write(messageLog.str(), LL_Info);

	// Add custom channel to logger
	m_pSink = new AsynchronousFileSink(m_strUserName + "_" + m_strJobName + "_timings.txt");
	m_pChannel = new LoggerChannel("data_collection");
	m_pChannel->AddSink(m_pSink);
	logger->AddChannel("data_collection", m_pChannel, false);

	// Add header to log file
	#if defined __JOB_TIMING
		std::stringstream headerMessage;
		headerMessage << "Time";

		#if __JOB_TIMING_METHOD == __JOB_TIMING_FINE
			headerMessage << "\t Radiance\tAccumulation\tCommit";
		#endif

		headerMessage << "\tCycle Time\tCycle Hz\tFrame Time\tFrame Hz\tWorkers\n";
	
		(*logger)["data_collection"]->Write(headerMessage.str(), LL_All);
	#endif
	
	m_checkPointTime = 
		m_bootTime = Platform::GetTime();

	return true;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
void AsyncRenderTaskCoordinator::OnShutdown(void)
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
	delete m_pRadianceOutput;

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
bool AsyncRenderTaskCoordinator::OnHeartbeat(void) 
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

	// Get list of heartbeat workers
	std::vector<int> &workerList 
		= GetAvailableWorkerList();

	// Send first batch of state changes
	for (auto worker : workerList)
	{
		Communicator::Send(&synchronisePacketSize, sizeof(int), worker, Communicator::Coordinator_Worker_Job);
		Communicator::Send(&syncPacket, sizeof(SynchronisePacket), worker, Communicator::Coordinator_Worker_Job);
	}

	return true;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
bool AsyncRenderTaskCoordinator::OnHeartbeatAbort(void)
{
	/* 
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
	*/

	return true;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
bool AsyncRenderTaskCoordinator::OnMessageReceived(ResourceMessage *p_pMessage) 
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
		m_cameraPathEx.Reset();
		m_animationTime = 0.f;

		std::vector<Vector3> pointList;
		arg.GetArgument("vertices", pointList);
		arg.GetArgument("delta", m_animationTimeDelta);
	
		for (std::vector<Vector3>::iterator it = pointList.begin(); 
			 it != pointList.end(); it++)
		{
			m_cameraPath.AddVertex(*it);
		}

		m_cameraPath.PreparePath();
	}
	else if (command == "pathex")
	{
		m_cameraPath.Reset();
		m_cameraPathEx.Reset();
		m_animationTime = 0.f;

		std::vector<Vector3> vertexList;
		arg.GetArgument("vertices", vertexList);
		arg.GetArgument("delta", m_animationTimeDelta);
	
		PathVertexEx pathVertexEx;
		
		for (std::vector<Vector3>::iterator it = vertexList.begin();
			 it != vertexList.end(); it++)
		{
			pathVertexEx.position = *it++;

			float angle = (*it).X / 360 * Maths::PiTwo;

			pathVertexEx.orientation = pathVertexEx.position + 
				Vector3(Maths::Sin(angle), 0 , Maths::Cos(angle));
			
			m_cameraPathEx.AddVertex(pathVertexEx);
		}

		m_cameraPathEx.PreparePath();
	}

	return true;
}
//----------------------------------------------------------------------------------------------
// Synchronous computation
//----------------------------------------------------------------------------------------------
bool AsyncRenderTaskCoordinator::Compute(void) 
{
	// Synchronous computation goes here

	// memcpy(m_pRadianceOutput, m_pRadianceBuffer, m_pRadianceBuffer->GetArea() * sizeof(RadianceContext));
	// Apply temporal filtering
	m_pMotionBlurFilter->Apply(m_pRadianceBuffer, m_pRadianceOutput);
	if (m_bResetAccumulation)
	{
		m_pMotionBlurFilter->Reset();
		m_bResetAccumulation = false;
	}

	// Commit frame
	m_pRenderer->Commit(m_pRadianceOutput);

	// block for 1/30th of a second (we want approx 24fps)
	boost::this_thread::sleep(boost::posix_time::milliseconds(30));
	
	return true;
}
//----------------------------------------------------------------------------------------------
// Asynchronous computation
//----------------------------------------------------------------------------------------------
void AsyncRenderTaskCoordinator::ComputeThreadHandler(AsyncRenderTaskCoordinator *p_pCoordinator)
{
	// Receive buffer
	SerialisableRenderTile *pRenderTile = NULL;

	// Set up request and status for non-blocking receive
	Communicator::Status status;

	int request = 0,
		finalTaskID = -1,
		tileID = 0;

	p_pCoordinator->m_nProducerIndex = 0;
	p_pCoordinator->m_nTilesPacked = 0;

	while(p_pCoordinator->IsRunning())
	{
		// Any work requests?
		while (Communicator::ProbeAsynchronous(Communicator::Source_Any, Communicator::Worker_Coordinator_Job, &status))
		{
			// Remove message from pending set
			p_pCoordinator->m_pending.erase(status.MPI_SOURCE);

			// Check by size if payload-less message
			if (Communicator::GetSize(&status) == sizeof(int))
			{
				Communicator::Receive(&request, 
					Communicator::GetSize(&status), status.MPI_SOURCE,
					Communicator::Worker_Coordinator_Job);
			}
			else
			{
				// Receive compressed tile
				pRenderTile = p_pCoordinator->m_renderTileBuffer[p_pCoordinator->m_nProducerIndex++];

				Communicator::Receive(pRenderTile->GetTransferBuffer(), 
					Communicator::GetSize(&status), status.MPI_SOURCE,
					Communicator::Worker_Coordinator_Job);

				// Receive complete -> Inform consumer
				AtomicInt32::Increment((Int32*)&(p_pCoordinator->m_nTilesPacked));
				p_pCoordinator->m_decompressionQueueCV.notify_one();
			}

			// Worker is slated for release
			if (p_pCoordinator->m_release.find(status.MPI_SOURCE) != p_pCoordinator->m_release.end())
			{
				// Send sigterm
				Communicator::Send(&finalTaskID, sizeof(int), status.MPI_SOURCE, Communicator::Coordinator_Worker_Job);
			}
			else
			{
				// Add to pending set
				p_pCoordinator->m_pending.insert(status.MPI_SOURCE);

				Communicator::Send(&tileID, sizeof(int), status.MPI_SOURCE, Communicator::Coordinator_Worker_Job);
				tileID--;
			}
		}
	}

	// while running
	//
	// wait for work request
	//   does it have a payload?
	//      push to PC buffer
	//   is worker still registered?
	//      send job to worker
	//   else
	//      send eoj to worker
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
void AsyncRenderTaskCoordinator::InputThreadHandler(AsyncRenderTaskCoordinator *p_pCoordinator)
{
	static int counterdisplay = 0;

	while(p_pCoordinator->IsRunning())
	{
		if (p_pCoordinator->m_cameraPath.IsEmpty() && p_pCoordinator->m_cameraPathEx.IsEmpty())
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
		} 
		else 
		{
			float lastAnimationTime = p_pCoordinator->m_animationTime;
			p_pCoordinator->m_animationTime += p_pCoordinator->m_animationTimeDelta;

			if (p_pCoordinator->m_animationTime >= 1.f) 
			{
				std::cout << std::endl << "+++ Animation complete +++" << std::endl;

				p_pCoordinator->m_animationTime = 0;

				p_pCoordinator->m_cameraPath.Clear();
				p_pCoordinator->m_cameraPathEx.Clear();
				p_pCoordinator->m_bResetWorkerSeed = true;
				p_pCoordinator->m_bResetAccumulation = true;
			}
			else
			{
				if (counterdisplay++ > 25) 
				{ 
					counterdisplay = 0;
					std::cout << std::endl << "Animation complete:" << p_pCoordinator->m_animationTime * 100.0f << "%" << std::endl;
				}

				if (!p_pCoordinator->m_cameraPath.IsEmpty())
				{
					p_pCoordinator->m_observerPosition = p_pCoordinator->m_cameraPath.GetPosition(p_pCoordinator->m_animationTime - 0.001f);
					p_pCoordinator->m_observerTarget = p_pCoordinator->m_cameraPath.GetPosition(p_pCoordinator->m_animationTime + 0.001f);
				}
				else if (!p_pCoordinator->m_cameraPathEx.IsEmpty())
				{
					p_pCoordinator->m_cameraPathEx.Get(p_pCoordinator->m_animationTime, 
						p_pCoordinator->m_observerPosition, p_pCoordinator->m_observerTarget);
				}
				
				p_pCoordinator->m_pCamera->MoveTo(p_pCoordinator->m_observerPosition);
				p_pCoordinator->m_pCamera->LookAt(p_pCoordinator->m_observerTarget);

				p_pCoordinator->m_bResetAccumulation = true;
				p_pCoordinator->m_bResetWorkerSeed = true;
			}
		}

		boost::this_thread::sleep(boost::posix_time::millisec(20));
	}
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
void AsyncRenderTaskCoordinator::DecompressionThreadHandler(AsyncRenderTaskCoordinator *p_pCoordinator)
{
	ServiceManager::GetInstance()->GetLogger()->Write("RenderTaskCoordinator :: Decompression thread started.", LL_Info);

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

	ServiceManager::GetInstance()->GetLogger()->Write("RenderTaskCoordinator :: Decompression thread completed.", LL_Info);
}