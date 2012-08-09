//----------------------------------------------------------------------------------------------
//	Filename:	RenderTaskWorker.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include "RenderTaskWorker.h"
#include "Communicator.h"
//----------------------------------------------------------------------------------------------
bool RenderTaskWorker::Compute(void) 
{
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

		//--------------------------------------------------
		// Send back result
		//--------------------------------------------------
		m_pRenderTile->SetID(tileID);
		m_pRenderTile->Compress();

		Communicator::Send(m_pRenderTile->GetCompressedBuffer(), m_pRenderTile->GetCompressedBufferSize(),
			GetCoordinatorID(), Communicator::Worker_Coordinator_Job);
	}

	return true;
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

	// Set up context
	m_renderTaskContext.FrameWidth = m_pRenderer->GetDevice()->GetWidth();
	m_renderTaskContext.FrameHeight = m_pRenderer->GetDevice()->GetHeight();
	m_renderTaskContext.TilesPerRow = m_renderTaskContext.FrameWidth / m_renderTaskContext.TileWidth;
	m_renderTaskContext.TilesPerColumn = m_renderTaskContext.FrameHeight / m_renderTaskContext.TileHeight;
	m_renderTaskContext.TotalTiles = m_renderTaskContext.TilesPerColumn * m_renderTaskContext.TilesPerRow;
	
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
	return true;
}
//----------------------------------------------------------------------------------------------
