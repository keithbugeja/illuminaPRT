//----------------------------------------------------------------------------------------------
//	Filename:	RenderTaskCoordinator.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include "RenderTaskCoordinator.h"
#include "Communicator.h"
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
bool RenderTaskCoordinator::Compute(void) 
{
	RadianceContext *pTileBuffer;
	
	int receivedTileID,
		waitingTasks = 0;

	int tileID = m_renderTaskContext.TotalTiles - 1;

	// Get list of available workers
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

	// Wait for results and send 
	Communicator::Status status;

	while (waitingTasks > 0)
	{
		// Probe for incoming message
		Communicator::Probe(Communicator::Source_Any, Communicator::Worker_Coordinator_Job, &status);
		
		// Set size of compressed buffer
		m_pRenderTile->SetCompressedBufferSize(Communicator::GetSize(&status));

		// Receive compressed tile
		Communicator::Receive(m_pRenderTile->GetCompressedBuffer(), 
			Communicator::GetSize(&status), status.MPI_SOURCE,
			Communicator::Worker_Coordinator_Job);

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

		// Decompress current tile
		m_pRenderTile->Decompress();

		// Get image data and copy to buffer
		receivedTileID = m_pRenderTile->GetID();
		pTileBuffer = m_pRenderTile->GetImageData()->GetBuffer(); 

		int sx = (receivedTileID % m_renderTaskContext.TilesPerRow) * m_renderTaskContext.TileWidth,
			sy = (receivedTileID / m_renderTaskContext.TilesPerRow) * m_renderTaskContext.TileHeight,
			sxe = sx + m_renderTaskContext.TileWidth,
			sye = sy + m_renderTaskContext.TileHeight;

		for (int y = sy; y < sye; y++) 
		{
			for (int x = sx; x < sxe; x++) 
			{
				m_pRadianceBuffer->Set(x, y, *pTileBuffer);
				
				pTileBuffer++;
			}
		}
	}

	// Apply discontinuity buffer
	m_pDiscontinuityBuffer->Apply(m_pRadianceBuffer, m_pRadianceBuffer);

	// Apply tone mapping
	m_pDragoTone->Apply(m_pRadianceBuffer, m_pRadianceBuffer);

	// Commit to device
	m_pRenderer->Commit(m_pRadianceBuffer);

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
	m_pDiscontinuityBuffer = m_pEngineKernel->GetPostProcessManager()->CreateInstance("Discontinuity", "DiscontinuityBuffer", "");
	m_pReconstructionBuffer = m_pEngineKernel->GetPostProcessManager()->CreateInstance("Reconstruction", "ReconstructionBuffer", "");
	m_pDragoTone = m_pEngineKernel->GetPostProcessManager()->CreateInstance("DragoTone", "DragoTone", "");

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
}
//----------------------------------------------------------------------------------------------
bool RenderTaskCoordinator::OnSynchronise(void) {
	return true;
}
//----------------------------------------------------------------------------------------------
bool RenderTaskCoordinator::OnMessageReceived(ResourceMessage *p_pMessage) {
	return true; 
}
//----------------------------------------------------------------------------------------------