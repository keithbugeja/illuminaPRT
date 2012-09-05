//----------------------------------------------------------------------------------------------
//	Filename:	RenderTaskWorker.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include "ServiceManager.h"
#include "RenderTaskWorker.h"
#include "Communicator.h"

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
bool RenderTaskWorker::ComputeUniform(void)
{
	double eventStart, eventComplete,
		computationTime;

	eventStart = Platform::GetTime();

	// Prepare integrator
	m_pIntegrator->Prepare(m_pEnvironment->GetScene());

	// Update space
	m_pSpace->Update();

	// Render
	int tileID;

	while (true)
	{
		//--------------------------------------------------
		// Receive tile
		//--------------------------------------------------
		Communicator::Receive(&tileID, sizeof(int), GetCoordinatorID(), Communicator::Coordinator_Worker_Job);

		//--------------------------------------------------
		// If termination signal, stop
		//--------------------------------------------------
		if (tileID == -1) break;

		//--------------------------------------------------
		// We have task id - render
		//--------------------------------------------------
		int sx = (tileID % m_renderTaskContext.TilesPerRow) * m_renderTaskContext.TileWidth,
			sy = (tileID / m_renderTaskContext.TilesPerRow) * m_renderTaskContext.TileHeight;
		
		m_pRenderer->RenderRegion(m_pRenderTile->GetImageData(), 
			sx, sy, m_renderTaskContext.TileWidth, m_renderTaskContext.TileHeight); 

		// Tone mapping moved to workers
		m_pToneMapper->Apply(m_pRenderTile->GetImageData(), m_pRenderTile->GetImageData());

		//--------------------------------------------------
		// Send back result
		//--------------------------------------------------
		m_pRenderTile->SetID(tileID);
		m_pRenderTile->Compress();

		Communicator::Send(m_pRenderTile->GetCompressedBuffer(), m_pRenderTile->GetCompressedBufferSize(),
			GetCoordinatorID(), Communicator::Worker_Coordinator_Job);
	}

	eventComplete = Platform::GetTime();
	computationTime = Platform::ToSeconds(eventComplete - eventStart);

	int meid = ServiceManager::GetInstance()->GetResourceManager()->Me()->GetID();
	std::cout << "---[" << meid << "] Computation time = " << computationTime << "s" << std::endl;

	return true;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
bool RenderTaskWorker::ComputeVariable(void)
{
	double eventStart, eventComplete,
		computationTime;

	eventStart = Platform::GetTime();

	// Prepare integrator
	m_pIntegrator->Prepare(m_pEnvironment->GetScene());

	// Update space
	m_pSpace->Update();

	// Render
	int tileID;

	while (true)
	{
		//--------------------------------------------------
		// Receive tile
		//--------------------------------------------------
		Communicator::Receive(&tileID, sizeof(int), GetCoordinatorID(), Communicator::Coordinator_Worker_Job);

		//--------------------------------------------------
		// If termination signal, stop
		//--------------------------------------------------
		if (tileID == -1) break;

		//--------------------------------------------------
		// We have task id - render
		//--------------------------------------------------
		// consider const Packet &packet!
		RenderTilePackets::Packet packet = m_renderTaskContext.TilePackets.GetPacket(tileID);

		m_pRenderer->RenderRegion(m_pRenderTile->GetImageData(), 
			packet.XStart, packet.YStart, packet.XSize, packet.YSize);

		// Tone mapping moved to workers
		m_pToneMapper->Apply(m_pRenderTile->GetImageData(), m_pRenderTile->GetImageData());

		//--------------------------------------------------
		// Send back result
		//--------------------------------------------------
		m_pRenderTile->SetID(tileID);
		m_pRenderTile->Compress();

		Communicator::Send(m_pRenderTile->GetCompressedBuffer(), m_pRenderTile->GetCompressedBufferSize(),
			GetCoordinatorID(), Communicator::Worker_Coordinator_Job);
	}

	eventComplete = Platform::GetTime();
	computationTime = Platform::ToSeconds(eventComplete - eventStart);

	int meid = ServiceManager::GetInstance()->GetResourceManager()->Me()->GetID();
	std::cout << "---[" << meid << "] Computation time = " << computationTime << "s" << std::endl;

	return true;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
bool RenderTaskWorker::Compute(void) 
{
	/* 
	 * Uniform tile sizes
	 */

	return ComputeUniform();
}
//----------------------------------------------------------------------------------------------
// User handlers for init, shutdown and sync events
bool RenderTaskWorker::OnCoordinatorMessages(void *p_pMessage) 
{
	return true;
}
//----------------------------------------------------------------------------------------------
bool RenderTaskWorker::OnInitialise(void) 
{
	std::cout << "RenderTaskWorker::OnInitialise()" << std::endl;

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
		std::cout << "Tile size[" << m_renderTaskContext.TileWidth << " x " 
			<< m_renderTaskContext.TileHeight << "]" << std::endl;
	}

	// Engine, environment
	m_pEnvironment = m_pSandbox->GetEnvironment();
	m_pEngineKernel = m_pSandbox->GetEngineKernel();

	// Alias 
	m_pIntegrator = m_pEnvironment->GetIntegrator();
	m_pRenderer = m_pEnvironment->GetRenderer();
	m_pCamera = m_pEnvironment->GetCamera();
	m_pSpace = m_pEnvironment->GetSpace();

	// Discontinuity, reconstruction and tone mapping
	m_pDiscontinuityBuffer = m_pEngineKernel->GetPostProcessManager()->CreateInstance("Discontinuity", "DiscontinuityBuffer", "");
	m_pReconstructionBuffer = m_pEngineKernel->GetPostProcessManager()->CreateInstance("Reconstruction", "ReconstructionBuffer", "");
	m_pToneMapper = m_pEngineKernel->GetPostProcessManager()->CreateInstance("GlobalTone", "GlobalTone", "");

	// Set up context
	m_renderTaskContext.FrameWidth = m_pRenderer->GetDevice()->GetWidth();
	m_renderTaskContext.FrameHeight = m_pRenderer->GetDevice()->GetHeight();
	m_renderTaskContext.TilesPerRow = m_renderTaskContext.FrameWidth / m_renderTaskContext.TileWidth;
	m_renderTaskContext.TilesPerColumn = m_renderTaskContext.FrameHeight / m_renderTaskContext.TileHeight;
	m_renderTaskContext.TotalTiles = m_renderTaskContext.TilesPerColumn * m_renderTaskContext.TilesPerRow;
	
	// NOTE: Possibly need to generate this every time worker count changes
	m_renderTaskContext.TilePackets.GeneratePackets(m_renderTaskContext.FrameWidth, m_renderTaskContext.FrameHeight,
		Maths::Max(m_renderTaskContext.TileWidth, m_renderTaskContext.TileHeight), 8);

	return true;
}
//----------------------------------------------------------------------------------------------
void RenderTaskWorker::OnShutdown(void) 
{
	// Shutdown renderer, integrator
	m_pRenderer->Shutdown();
	m_pIntegrator->Shutdown();

	// Shutdown and delete sandbox
	m_pSandbox->Shutdown(false);
	delete m_pSandbox;

	// Delete serialisable tile
	delete m_pRenderTile;
}
//----------------------------------------------------------------------------------------------
bool RenderTaskWorker::OnSynchronise(void) 
{
	int synchronisePacketSize;

	Communicator::Receive(&synchronisePacketSize, sizeof(int), GetCoordinatorID(), Communicator::Coordinator_Worker_Job);
	
	if (synchronisePacketSize == -1)
		return false;

	char buffer[2048];

	Communicator::Receive(buffer, synchronisePacketSize, GetCoordinatorID(), Communicator::Coordinator_Worker_Job);
	Vector3 *observer = (Vector3*)buffer;
	m_pCamera->MoveTo(*observer);

	return true;
}
//----------------------------------------------------------------------------------------------
