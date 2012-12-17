//----------------------------------------------------------------------------------------------
//	Filename:	RenderTaskWorker.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include <boost/algorithm/string.hpp> 

#include "ServiceManager.h"
#include "RenderTaskWorker.h"
#include "Communicator.h"
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
bool RenderTaskWorker::ComputeUniform(void)
{
	/*
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
	*/
	return true;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
bool RenderTaskWorker::ComputeVariable(void)
{
	double eventStart, eventComplete,
		communicationStart, communicationComplete,
		jobTime, communicationTime;

	int pixelsRendered = 0;
	size_t bytesTransferred = 0;
	
	// Start clocking job time
	eventStart = Platform::GetTime();

	//if (m_bResetSampler) {
		m_pEnvironment->GetSampler()->Reset(m_unSamplerSeed);
	//}

	// Prepare integrator
	m_pIntegrator->Prepare(m_pEnvironment->GetScene());

	// Update space
	m_pSpace->Update();

	// Render
	int tileID;

	for (communicationTime = 0, jobTime = 0;;)
	{
		//--------------------------------------------------
		// Receive tile
		//--------------------------------------------------
		communicationStart = Platform::GetTime();

		Communicator::Receive(&tileID, sizeof(int), GetCoordinatorID(), Communicator::Coordinator_Worker_Job);
		
		communicationComplete = Platform::GetTime();
		communicationTime += Platform::ToSeconds(communicationComplete - communicationStart);

		//--------------------------------------------------
		// If termination signal, stop
		//--------------------------------------------------
		if (tileID == -1) break;

		//--------------------------------------------------
		// We have task id - render
		//--------------------------------------------------
		// consider const Packet &packet!
		RenderTilePackets::Packet packet = m_renderTaskContext.TilePackets.GetPacket(tileID);

		const int kernelSize = 8;
		const int halfKernelSize = kernelSize >> 1;

		m_pRenderTile->Resize(packet.XSize, packet.YSize);
		m_pRimmedRenderTile->Resize(packet.XSize + kernelSize, packet.YSize + kernelSize);

		m_pRenderer->RenderRegion(m_pRimmedRenderTile->GetImageData(),
			packet.XStart - halfKernelSize,
			packet.YStart - halfKernelSize,
			packet.XSize + kernelSize,
			packet.YSize + kernelSize,
			0, 0);

		pixelsRendered += (packet.XSize + kernelSize) * (packet.YSize + kernelSize);

		// Discontinuity Buffer
		/* 
		((DiscontinuityBuffer*)m_pDiscontinuityBuffer)->SetKernelSize(4);
		m_pDiscontinuityBuffer->Apply(m_pRimmedRenderTile->GetImageData(), m_pRimmedRenderTile->GetImageData());
		*/

		// Bilateral Filter
		/**/

		//((BilateralFilter*)m_pBilateralFilter)->SetKernelSize(4);
		//m_pBilateralFilter->Apply(m_pRimmedRenderTile->GetImageData(), m_pRimmedRenderTile->GetImageData());
		
		/*
		m_pBilateralFilter->Apply(m_pRimmedRenderTile->GetImageData(), m_pRimmedRenderTile->GetImageData());
		m_pBilateralFilter->Apply(m_pRimmedRenderTile->GetImageData(), m_pRimmedRenderTile->GetImageData());
		m_pBilateralFilter->Apply(m_pRimmedRenderTile->GetImageData(), m_pRimmedRenderTile->GetImageData());
		*/

		/**/
		// m_pBilateralFilter->Apply(m_pRimmedRenderTile->GetImageData(), m_pRimmedRenderTile->GetImageData());
		// m_pBilateralFilter->Apply(m_pRimmedRenderTile->GetImageData(), m_pRimmedRenderTile->GetImageData());
		//m_pBilateralFilter->Apply(m_pRimmedRenderTile->GetImageData(), m_pRimmedRenderTile->GetImageData());
		/**/

		RadianceContext *pSrc, *pDst;
		
		for (int y = 0, ys = halfKernelSize; y < packet.YSize; y++, ys++)
		{
			pSrc = m_pRimmedRenderTile->GetImageData()->GetP(halfKernelSize, ys);
			pDst = m_pRenderTile->GetImageData()->GetP(0, y);

			for (int x = 0; x < packet.XSize; x++)
			{
				pDst->Final = pSrc->Final;
				pDst->Flags = RadianceContext::DF_Final | RadianceContext::DF_Computed;

				pSrc++;
				pDst++;
			}
		}

		// Tone mapping moved to workers
		m_pToneMapper->Apply(m_pRenderTile->GetImageData(), m_pRenderTile->GetImageData());

		//--------------------------------------------------
		// Package result
		//--------------------------------------------------
		m_pRenderTile->SetID(tileID);
		m_pRenderTile->Package();

		bytesTransferred += m_pRenderTile->GetTransferBufferSize();

		//--------------------------------------------------
		// Send back result
		//--------------------------------------------------
		communicationStart = Platform::GetTime();

		Communicator::Send(m_pRenderTile->GetTransferBuffer(), m_pRenderTile->GetTransferBufferSize(),
			GetCoordinatorID(), Communicator::Worker_Coordinator_Job);

		communicationComplete = Platform::GetTime();
		communicationTime += Platform::ToSeconds(communicationComplete - communicationStart);
	}

	eventComplete = Platform::GetTime();
	jobTime = Platform::ToSeconds(eventComplete - eventStart);

	/* 
	int meid = ServiceManager::GetInstance()->GetResourceManager()->Me()->GetID();
	std::cout << "---[" << meid << "] Job time = " << jobTime << "s" << std::endl;
	std::cout << "---[" << meid << "] -- Computation time = " << jobTime - communicationTime << "s" << std::endl;
	std::cout << "---[" << meid << "] -- Communication time = " << communicationTime << "s" << std::endl;
	std::cout << "---[" << meid << "] -- Bytes transferred = " << bytesTransferred << std::endl;
	std::cout << "---[" << meid << "] -- Pixels rendered = " << pixelsRendered << std::endl;
	*/

	return true;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
bool RenderTaskWorker::Compute(void) 
{
	/* 
	 * Uniform tile sizes
	 */

	return ComputeVariable();
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

	//----------------------------------------------------------------------------------------------
	// Initialise sandbox environment
	//----------------------------------------------------------------------------------------------
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

		m_pRimmedRenderTile = new SerialisableRenderTile(-1,
			m_renderTaskContext.TileWidth + 32, m_renderTaskContext.TileHeight + 32);
	}

	// Read adaptive tile settings
	std::string adaptiveTile;
	pArgumentMap->GetArgument("useadaptive", adaptiveTile); boost::to_lower(adaptiveTile);
	m_renderTaskContext.AdaptiveTiles = adaptiveTile == "false" ? false : true;
	pArgumentMap->GetArgument("batchsize", m_renderTaskContext.TileBatchSize);

	//----------------------------------------------------------------------------------------------
	// Engine, environment
	//----------------------------------------------------------------------------------------------
	m_pEnvironment = m_pSandbox->GetEnvironment();
	m_pEngineKernel = m_pSandbox->GetEngineKernel();

	//----------------------------------------------------------------------------------------------
	// Alias 
	//----------------------------------------------------------------------------------------------
	m_pIntegrator = m_pEnvironment->GetIntegrator();
	m_pRenderer = m_pEnvironment->GetRenderer();
	m_pCamera = m_pEnvironment->GetCamera();
	m_pSpace = m_pEnvironment->GetSpace();

	//----------------------------------------------------------------------------------------------
	// Discontinuity, reconstruction and tone mapping
	//----------------------------------------------------------------------------------------------
	m_pBilateralFilter = m_pEngineKernel->GetPostProcessManager()->CreateInstance("BilateralFilter", "BilateralFilter", "KernelSize=3;");
	m_pDiscontinuityBuffer = m_pEngineKernel->GetPostProcessManager()->CreateInstance("Discontinuity", "DiscontinuityBuffer", "KernelSize=3;");
	m_pReconstructionBuffer = m_pEngineKernel->GetPostProcessManager()->CreateInstance("Reconstruction", "ReconstructionBuffer", "");
	m_pToneMapper = m_pEngineKernel->GetPostProcessManager()->CreateInstance("GlobalTone", "GlobalTone", "");

	//----------------------------------------------------------------------------------------------
	// Set up context
	//----------------------------------------------------------------------------------------------
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

	//----------------------------------------------------------------------------------------------
	// Reset sampler
	m_bResetSampler = true;
	m_unSamplerSeed = 0x03170317;

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
	SynchronisePacket *packet = (SynchronisePacket*)buffer;

	std::cout << "Worker got sync packet [0] : " << packet->observerPosition.ToString() << std::endl;
	std::cout << "Worker got sync packet [1] : " << packet->observerTarget.ToString() << std::endl;
	std::cout << "Worker get sync packet [2] : " << packet->resetSeed << std::endl;

	if (packet->resetSeed != 0)
		m_unSamplerSeed = 0x03170317;
	else
		m_unSamplerSeed += 0x0101;

	m_pCamera->MoveTo(packet->observerPosition);
	m_pCamera->LookAt(packet->observerTarget);
	
	return true;
}
//----------------------------------------------------------------------------------------------
