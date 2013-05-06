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
	// Need following:
	// 1) Overlap size
	// 2) Post-process filters

	/* 
	double eventStart, eventComplete,
		communicationStart, communicationComplete,
		jobTime, communicationTime;
	*/

	m_nBorderSize = 1;
	m_uiFilterFlags = __PPF_BilateralFilter | 
		__PPF_Discontinuity |
		__PPF_Tonemapping;

	size_t bytesTransferred = 0;
	int pixelsRendered = 0, 
		tileID;
	
	int overlap = 
		m_nBorderSize << 1;

	//----------------------------------------------------------------------------------------------
	// Start clocking job time
	//----------------------------------------------------------------------------------------------
	// eventStart = Platform::GetTime();

	// Set seed 
	m_pEnvironment->GetSampler()->Reset(m_unSamplerSeed);

	// Prepare integrator
	m_pIntegrator->Prepare(m_pEnvironment->GetScene());

	// Update space
	m_pSpace->Update();

	// Render
	// for (communicationTime = 0, jobTime = 0;;)
	for(;;)
	{
		//--------------------------------------------------
		// Receive tile
		//--------------------------------------------------
		// communicationStart = Platform::GetTime();

		Communicator::Receive(&tileID, sizeof(int), GetCoordinatorID(), Communicator::Coordinator_Worker_Job);

		// communicationComplete = Platform::GetTime();
		// communicationTime += Platform::ToSeconds(communicationComplete - communicationStart);

		//--------------------------------------------------
		// If termination signal, stop
		//--------------------------------------------------
		if (tileID == -1) break;

		//--------------------------------------------------
		// We have task id - render
		//--------------------------------------------------
		// consider const Packet &packet!
		RenderTilePackets::Packet packet = m_renderTaskContext.TilePackets.GetPacket(tileID);

		m_pRenderTile->Resize(packet.XSize, packet.YSize);
		m_pRimmedRenderTile->Resize(packet.XSize + overlap, packet.YSize + overlap);
		
		m_pRenderer->RenderRegion(m_pRimmedRenderTile->GetImageData(),
			packet.XStart - m_nBorderSize,
			packet.YStart - m_nBorderSize,
			packet.XSize + overlap,
			packet.YSize + overlap,
			0, 0);

		// pixelsRendered += (packet.XSize + overlap) * (packet.YSize + overlap);
		/**/
		// Discontinuity Buffer
		if (m_uiFilterFlags & __PPF_Discontinuity)
		{		
			((DiscontinuityBuffer*)m_pDiscontinuityBuffer)->SetKernelSize(1 + overlap);
			m_pDiscontinuityBuffer->Apply(m_pRimmedRenderTile->GetImageData(), m_pRimmedRenderTile->GetImageData());
		} 
		/**/
		/*
		// Bilateral Filter
		if (m_uiFilterFlags & __PPF_BilateralFilter)
		{
			((BilateralFilter*)m_pBilateralFilter)->SetKernelSize(1 + overlap);
			m_pBilateralFilter->Apply(m_pRimmedRenderTile->GetImageData(), m_pRimmedRenderTile->GetImageData());
		}
		*/

		// Trim tile
		RadianceContext *pSrc, *pDst;
		
		for (int y = 0, ys = m_nBorderSize; y < packet.YSize; y++, ys++)
		{
			pSrc = m_pRimmedRenderTile->GetImageData()->GetP(m_nBorderSize, ys);
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
		if (m_uiFilterFlags & __PPF_Tonemapping)
		{
			m_pToneMapper->Apply(m_pRenderTile->GetImageData(), m_pRenderTile->GetImageData());
		}

		//--------------------------------------------------
		// Package result
		//--------------------------------------------------
		m_pRenderTile->SetID(tileID);
		m_pRenderTile->Package();

		// bytesTransferred += m_pRenderTile->GetTransferBufferSize();

		//--------------------------------------------------
		// Send back result
		//--------------------------------------------------
		// communicationStart = Platform::GetTime();

		Communicator::Send(m_pRenderTile->GetTransferBuffer(), m_pRenderTile->GetTransferBufferSize(),
			GetCoordinatorID(), Communicator::Worker_Coordinator_Job);

		// communicationComplete = Platform::GetTime();
		// communicationTime += Platform::ToSeconds(communicationComplete - communicationStart);
	}

	/* 
	eventComplete = Platform::GetTime();
	jobTime = Platform::ToSeconds(eventComplete - eventStart);

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
bool RenderTaskWorker::ComputeVariable(void)
{
	double eventStart, eventComplete,
		communicationStart, communicationComplete,
		jobTime, communicationTime;

	int pixelsRendered = 0;
	size_t bytesTransferred = 0;
	
	//----------------------------------------------------------------------------------------------
	// Start clocking job time
	//----------------------------------------------------------------------------------------------
	eventStart = Platform::GetTime();

	// Set seed 
	m_pEnvironment->GetSampler()->Reset(m_unSamplerSeed);

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

	// return ComputeVariable();
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
		// Get logger instance
	std::stringstream messageLog;
	Logger *logger = ServiceManager::GetInstance()->GetLogger();
	logger->Write("RenderTaskWorker :: Handling event [OnInitialise].", LL_Info);

	//----------------------------------------------------------------------------------------------
	// Initialise and load sandbox environment
	//----------------------------------------------------------------------------------------------
	std::string strScriptName;
	ArgumentMap *pArgumentMap = GetArgumentMap();

	m_pSandbox = new SandboxEnvironment();
	m_pSandbox->Initialise();

	if (!pArgumentMap->GetArgument(__TaskID, m_nTaskId))
	{
		logger->Write("RenderTaskWorker :: Unable to determine task id ["__TaskID"] from argument list.", LL_Error);
		return false;
	}

	if (!pArgumentMap->GetArgument(__Script_Name, strScriptName))
	{
		logger->Write("RenderTaskWorker :: Unable to find ["__Script_Name"] entry in argument list.", LL_Error);
		return false;
	}

	if (!m_pSandbox->LoadScene(strScriptName))
	{
		messageLog << "RenderTaskWorker :: Unable to load scene script [" << strScriptName << "].";
		logger->Write(messageLog.str(), LL_Error);
		return false;
	}
		
	messageLog << "RenderTaskWorker :: Scene script [" << strScriptName << "] loaded.";
	
	//----------------------------------------------------------------------------------------------
	// Initialise render task
	//----------------------------------------------------------------------------------------------
	if (pArgumentMap->GetArgument(__Tile_Width, m_renderTaskContext.TileWidth) &&
		pArgumentMap->GetArgument(__Tile_Height, m_renderTaskContext.TileHeight))
	{
		m_pRenderTile = new SerialisableRenderTile(-1, m_renderTaskContext.TileWidth, m_renderTaskContext.TileHeight);
		m_pRimmedRenderTile = new SerialisableRenderTile(-1, m_renderTaskContext.TileWidth + 32, m_renderTaskContext.TileHeight + 32);

		messageLog << std::endl << "RenderTaskWorker :: Maximum tile size set to [" 
			<< m_renderTaskContext.TileWidth << " x " 
			<< m_renderTaskContext.TileHeight << "], border [+32 x +32]";
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
		messageLog << std::endl << "RenderTaskWorker :: Workers required [" << m_renderTaskContext.WorkersRequired << "]";

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
	std::string deviceOverride; pArgumentMap->GetArgument(__Device_Override, deviceOverride); boost::to_lower(deviceOverride);
	if (deviceOverride == "true")
	{
		int width, height;
		pArgumentMap->GetArgument(__Device_Width, width);
		pArgumentMap->GetArgument(__Device_Height, height);

		m_renderTaskContext.FrameWidth = width;
		m_renderTaskContext.FrameHeight = height;
	}
	else
	{
		m_renderTaskContext.FrameWidth = m_pRenderer->GetDevice()->GetWidth();
		m_renderTaskContext.FrameHeight = m_pRenderer->GetDevice()->GetHeight();
	}

	m_renderTaskContext.TilesPerRow = m_renderTaskContext.FrameWidth / m_renderTaskContext.TileWidth;
	m_renderTaskContext.TilesPerColumn = m_renderTaskContext.FrameHeight / m_renderTaskContext.TileHeight;
	m_renderTaskContext.TotalTiles = m_renderTaskContext.TilesPerColumn * m_renderTaskContext.TilesPerRow;
		
	if (m_renderTaskContext.AdaptiveTiles)
	{
		messageLog << std::endl << "RenderTaskWorker :: Adaptive tiling enabled with batch size [" << m_renderTaskContext.TileBatchSize << "]";

		m_renderTaskContext.TilePackets.GeneratePackets(m_renderTaskContext.FrameWidth, m_renderTaskContext.FrameHeight,
			Maths::Max(m_renderTaskContext.TileWidth, m_renderTaskContext.TileHeight), m_renderTaskContext.TileBatchSize);
	} 
	else
	{
		messageLog << std::endl << "RenderTaskWorker :: Adaptive tiling disabled.";

		m_renderTaskContext.TilePackets.GeneratePackets(m_renderTaskContext.FrameWidth, m_renderTaskContext.FrameHeight,
			Maths::Max(m_renderTaskContext.TileWidth, m_renderTaskContext.TileHeight), 10000);
	}

	// Log accumulated messages
	logger->Write(messageLog.str(), LL_Info);

	return true;
}

//----------------------------------------------------------------------------------------------
void RenderTaskWorker::OnShutdown(void) 
{
	ServiceManager::GetInstance()->GetLogger()->Write("RenderTaskWorker :: Handling event [OnShutdown].", LL_Info);

	// Shutdown renderer, integrator
	m_pRenderer->Shutdown();
	m_pIntegrator->Shutdown();

	// Shutdown and delete sandbox
	m_pSandbox->Shutdown();
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

	/*
	std::cout << "Worker got sync packet [0] : " << packet->observerPosition.ToString() << std::endl;
	std::cout << "Worker got sync packet [1] : " << packet->observerTarget.ToString() << std::endl;
	std::cout << "Worker get sync packet [2] : " << packet->resetSeed << std::endl;
	*/

	m_unSamplerSeed = (unsigned int)packet->seed;

	/*
	int meid = ServiceManager::GetInstance()->GetResourceManager()->Me()->GetID();
	std::cout << "---[" << meid << "] Seed = " << m_unSamplerSeed << "s" << std::endl;
	*/

	/*
	if (packet->resetSeed != 0)
		m_unSamplerSeed = 0x03170317;
	else
		m_unSamplerSeed += 0x0101;
	*/

	m_pCamera->MoveTo(packet->observerPosition);
	m_pCamera->LookAt(packet->observerTarget);
	
	return true;
}
//----------------------------------------------------------------------------------------------
