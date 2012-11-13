//----------------------------------------------------------------------------------------------
//	Filename:	RenderTaskCoordinator.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include <boost/algorithm/string.hpp> 

#include "RenderTaskCoordinator.h"
#include "Communicator.h"
//----------------------------------------------------------------------------------------------
bool RenderTaskCoordinator::Compute(void) 
{
	RadianceContext *pTileBuffer;
	
	int receivedTileID,
		waitingTasks = 0;

	int tileID = m_renderTaskContext.TilePackets.GetPacketCount() - 1;

	double eventStart, eventComplete, 
		subEventStart, subEventComplete,
		radianceTime,
		bilateralTime,
		discontinuityTime,
		toneTime,
		commitTime,
		decompressionTime,
		framePackageSize;

	decompressionTime = 
		framePackageSize = 0;

	// Get list of available workers
	std::vector<int> &workerList 
		= GetAvailableWorkerList();

	eventStart = Platform::GetTime();

	// Send first batch of jobs
	for (std::vector<int>::iterator workerIterator = workerList.begin();
		 workerIterator != workerList.end(); workerIterator++)
	{
		Communicator::Send(&tileID, sizeof(int), *workerIterator, Communicator::Coordinator_Worker_Job);

		tileID--; waitingTasks++;
		
		if (tileID < 0)
			break;
	}

	// Wait for results and send 
	Communicator::Status status;

	SerialisableRenderTile *pRenderTile = NULL;

	m_nProducerIndex = 0;
	m_nConsumerIndex = 0;
	m_nTilesPacked = 0;

	while (waitingTasks > 0)
	{
		// Probe for incoming message
		Communicator::Probe(Communicator::Source_Any, Communicator::Worker_Coordinator_Job, &status);

		// Receive compressed tile
		/*
		Communicator::Receive(m_pRenderTile->GetTransferBuffer(), 
			Communicator::GetSize(&status), status.MPI_SOURCE,
			Communicator::Worker_Coordinator_Job);
		*/

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

		subEventStart = Platform::GetTime();

		/*
		// Decompress current tile
		pRenderTile->Unpackage();

		// Get image data and copy to buffer
		receivedTileID = pRenderTile->GetID();
		pTileBuffer = pRenderTile->GetImageData()->GetBuffer(); 

		RenderTilePackets::Packet packet = m_renderTaskContext.TilePackets.GetPacket(receivedTileID);
		int sx = packet.XStart,
			sy = packet.YStart,
			sxe = sx + packet.XSize,
			sye = sy + packet.YSize;
		
		for (int y = sy; y < sye; y++) 
		{
			for (int x = sx; x < sxe; x++) 
			{
				m_pRadianceBuffer->Set(x, y, *pTileBuffer);
				
				pTileBuffer++;
			}
		}
		*/

		subEventComplete = Platform::GetTime();
		decompressionTime += Platform::ToSeconds(subEventComplete - subEventStart);
	}

	eventComplete = Platform::GetTime();
	radianceTime = Platform::ToSeconds(eventComplete - eventStart);

	// Discontinuity buffer
	/*
	eventStart = Platform::GetTime();
	m_pBilateralFilter->Apply(m_pRadianceBuffer, m_pRadianceBuffer);
	eventComplete = Platform::GetTime();
	bilateralTime = Platform::ToSeconds(eventComplete - eventStart);
	
	eventStart = Platform::GetTime();
	m_pDiscontinuityBuffer->Apply(m_pRadianceBuffer, m_pRadianceBuffer);
	eventComplete = Platform::GetTime();
	discontinuityTime = Platform::ToSeconds(eventComplete - eventStart);
	/**/

	// Tone mapping (moved to server)
	/*
	eventStart = Platform::GetTime();
	m_pDragoTone->Apply(m_pRadianceBuffer, m_pRadianceBuffer);
	eventComplete = Platform::GetTime();
	toneTime = Platform::ToSeconds(eventComplete - eventStart);
	/**/

	// Accumulation
	eventStart = Platform::GetTime();
	m_pAccumulationBuffer->Apply(m_pRadianceBuffer, m_pRadianceBuffer);
	eventComplete = Platform::GetTime();

	// Commit to device
	eventStart = Platform::GetTime();
	m_pRenderer->Commit(m_pRadianceBuffer);
	eventComplete = Platform::GetTime();
	commitTime = Platform::ToSeconds(eventComplete - eventStart);

	if (m_bResetAccumulationBuffer)
	{
		m_pAccumulationBuffer->Reset();
		m_bResetAccumulationBuffer = false;
	}

	std::cout << "---| Radiance Time : " << radianceTime << "s" << std::endl;
	std::cout << "---| Decompression Time : " << decompressionTime << "s for " << framePackageSize / (1024 * 1024) << " MB" << std::endl;
	std::cout << "---| Bilateral Time : " << bilateralTime << "s" << std::endl;
	std::cout << "---| Discontinuity Time : " << discontinuityTime << "s" << std::endl;
	std::cout << "---| Tonemapping Time : " << toneTime << "s" << std::endl;
	std::cout << "---| Commit Time : " << commitTime << "s" << std::endl;

	return true;
}
//----------------------------------------------------------------------------------------------
bool RenderTaskCoordinator::OnInitialise(void) 
{
	std::cout << "RenderTaskCoordinator::OnInitialise()" << std::endl;

	std::string strScriptName;
	ArgumentMap *pArgumentMap = GetArgumentMap();

	// Initialise sandbox environment
	m_pSandbox = new SandboxEnvironment();
	m_pSandbox->Initialise(false);

	// Read environment script
	if (!pArgumentMap->GetArgument("script", strScriptName))
		return false;

	if (m_pSandbox->LoadScene(strScriptName, true))
		std::cout << "Scene [" << strScriptName << "] loaded." << std::endl;

	// Read tile width and height arugments;
	if (pArgumentMap->GetArgument("width", m_renderTaskContext.TileWidth) &&
		pArgumentMap->GetArgument("height", m_renderTaskContext.TileHeight))
	{
		m_pRenderTile = new SerialisableRenderTile(-1, 
			m_renderTaskContext.TileWidth, m_renderTaskContext.TileHeight);
		std::cout << "Tile size [" << m_renderTaskContext.TileWidth << " x " 
			<< m_renderTaskContext.TileHeight << "]" << std::endl;
	}

	// Read adaptive tile settings
	std::string adaptiveTile;
	pArgumentMap->GetArgument("useadaptive", adaptiveTile); boost::to_lower(adaptiveTile);
	m_renderTaskContext.AdaptiveTiles = adaptiveTile == "false" ? false : true;
	pArgumentMap->GetArgument("batchsize", m_renderTaskContext.TileBatchSize);

	// Read the minimum number of required workers
	if (pArgumentMap->GetArgument("min", m_renderTaskContext.WorkersRequired))
		std::cout << "Workers required [" << m_renderTaskContext.WorkersRequired << "]" << std::endl;

	// Engine, environment
	m_pEnvironment = m_pSandbox->GetEnvironment();
	m_pEngineKernel = m_pSandbox->GetEngineKernel();

	// Alias 
	m_pIntegrator = m_pEnvironment->GetIntegrator();
	m_pRenderer = m_pEnvironment->GetRenderer();
	m_pCamera = m_pEnvironment->GetCamera();
	m_pSpace = m_pEnvironment->GetSpace();

	// Radiance buffer and accumulation radiance buffer
	m_pRadianceBuffer = new RadianceBuffer(m_pRenderer->GetDevice()->GetWidth(), m_pRenderer->GetDevice()->GetHeight()),
	m_pRadianceAccumulationBuffer = new RadianceBuffer(m_pRenderer->GetDevice()->GetWidth(), m_pRenderer->GetDevice()->GetHeight());

	// Discontinuity, reconstruction and tone mapping
	m_pBilateralFilter = m_pEngineKernel->GetPostProcessManager()->CreateInstance("BilateralFilter", "BilateralFilter", "");
	m_pDiscontinuityBuffer = m_pEngineKernel->GetPostProcessManager()->CreateInstance("Discontinuity", "DiscontinuityBuffer", "");
	m_pReconstructionBuffer = m_pEngineKernel->GetPostProcessManager()->CreateInstance("Reconstruction", "ReconstructionBuffer", "");
	m_pDragoTone = m_pEngineKernel->GetPostProcessManager()->CreateInstance("GlobalTone", "GlobalTone", "");
				//m_pEngineKernel->GetPostProcessManager()->CreateInstance("DragoTone", "DragoTone", "");

	// Accumulation buffer
	m_pAccumulationBuffer = (AccumulationBuffer*)m_pEngineKernel->GetPostProcessManager()->CreateInstance("Accumulation", "AccumulationBuffer", "");
	m_pAccumulationBuffer->SetAccumulationBuffer(m_pRadianceAccumulationBuffer);
	m_pAccumulationBuffer->Reset();

	// Open output device
	m_pRenderer->GetDevice()->Open();

	// Set up context
	m_renderTaskContext.FrameWidth = m_pRenderer->GetDevice()->GetWidth();
	m_renderTaskContext.FrameHeight = m_pRenderer->GetDevice()->GetHeight();
	m_renderTaskContext.TilesPerRow = m_renderTaskContext.FrameWidth / m_renderTaskContext.TileWidth;
	m_renderTaskContext.TilesPerColumn = m_renderTaskContext.FrameHeight / m_renderTaskContext.TileHeight;
	m_renderTaskContext.TotalTiles = m_renderTaskContext.TilesPerColumn * m_renderTaskContext.TilesPerRow;
		
	if (m_renderTaskContext.AdaptiveTiles)
	{
		std::cout << "Adaptive tiling enabled: Tile Batch [" << m_renderTaskContext.TileBatchSize << "]" << std::endl;

		m_renderTaskContext.TilePackets.GeneratePackets(m_renderTaskContext.FrameWidth, m_renderTaskContext.FrameHeight,
			Maths::Max(m_renderTaskContext.TileWidth, m_renderTaskContext.TileHeight), m_renderTaskContext.TileBatchSize);
	} 
	else
	{
		std::cout << "Adaptive tiling disabled." << std::endl;

		m_renderTaskContext.TilePackets.GeneratePackets(m_renderTaskContext.FrameWidth, m_renderTaskContext.FrameHeight,
			Maths::Max(m_renderTaskContext.TileWidth, m_renderTaskContext.TileHeight), 10000);
	}

	// Create receive buffers for each tile
	for (int packetIndex = 0; packetIndex < m_renderTaskContext.TilePackets.GetPacketCount(); packetIndex++)
	{
		m_renderTileBuffer.push_back(new SerialisableRenderTile(-1, m_renderTaskContext.TileWidth, m_renderTaskContext.TileHeight));
	}

	// Update observer position
	m_observerPosition = m_pEnvironment->GetCamera()->GetObserver();
	m_moveFlag[0] = m_moveFlag[1] = m_moveFlag[2] = m_moveFlag[3];
	m_bResetAccumulationBuffer = true;

	// kick off input thread
	boost::thread inputThreadHandler = 
		boost::thread(boost::bind(RenderTaskCoordinator::InputThreadHandler, this));

	boost::thread decompressionThreadHandler =
		boost::thread(boost::bind(RenderTaskCoordinator::DecompressionThreadHandler, this));

	return true;
}
//----------------------------------------------------------------------------------------------
void RenderTaskCoordinator::OnShutdown(void)
{
	// Close device
	m_pRenderer->GetDevice()->Close();

	// Shutdown renderer, integrator
	m_pRenderer->Shutdown();
	m_pIntegrator->Shutdown();

	// Delete radiance buffers
	delete m_pRadianceBuffer;
	delete m_pRadianceAccumulationBuffer;

	// Shutdown and delete sandbox
	m_pSandbox->Shutdown(false);
	delete m_pSandbox;

	// Delete serialisable tile
	delete m_pRenderTile;

	for (std::vector<SerialisableRenderTile*>::iterator packetIterator = m_renderTileBuffer.begin();
		 packetIterator != m_renderTileBuffer.end(); ++packetIterator)
		 delete (*packetIterator);
}
//----------------------------------------------------------------------------------------------
bool RenderTaskCoordinator::OnSynchronise(void) 
{
	int synchronisePacketSize = sizeof(Vector3);

	// Get list of available workers
	std::vector<int> &workerList 
		= GetAvailableWorkerList();

	// Send first batch of jobs
	for (std::vector<int>::iterator workerIterator = workerList.begin();
		 workerIterator != workerList.end(); workerIterator++)
	{
		Communicator::Send(&synchronisePacketSize, sizeof(int), *workerIterator, Communicator::Coordinator_Worker_Job);
		Communicator::Send(&m_observerPosition, sizeof(Vector3), *workerIterator, Communicator::Coordinator_Worker_Job);
	}

	return true;
}
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
bool RenderTaskCoordinator::OnMessageReceived(ResourceMessage *p_pMessage) 
{
	Message_Controller_Resource_Generic *pMessage = (Message_Controller_Resource_Generic*)p_pMessage->Content;
	// std::cout << "Received a generic message : [" << pMessage->String << "]" << std::endl;

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

	return true; 
}
//----------------------------------------------------------------------------------------------
void RenderTaskCoordinator::InputThreadHandler(RenderTaskCoordinator *p_pCoordinator)
{
	while(p_pCoordinator->IsRunning())
	{
		p_pCoordinator->m_bResetAccumulationBuffer |= 
			(p_pCoordinator->m_moveFlag[0] |
			 p_pCoordinator->m_moveFlag[1] |
			 p_pCoordinator->m_moveFlag[2] |
			 p_pCoordinator->m_moveFlag[3]);

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

		p_pCoordinator->m_pCamera->MoveTo(observer);
		p_pCoordinator->m_observerPosition = observer;

		boost::this_thread::sleep(boost::posix_time::millisec(20));
	}
}
//----------------------------------------------------------------------------------------------
void RenderTaskCoordinator::DecompressionThreadHandler(RenderTaskCoordinator *p_pCoordinator)
{
	std::cout << "Decompression Thread Handler" << std::endl;
	/**/

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
}