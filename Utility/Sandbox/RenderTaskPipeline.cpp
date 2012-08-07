//----------------------------------------------------------------------------------------------
//	Filename:	RenderTaskPipeline.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include "RenderTaskPipeline.h"
//----------------------------------------------------------------------------------------------
bool RenderTaskWorker::Compute(void) 
{ 
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000 / 60));
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

	if (!pArgumentMap->GetArgument("script", strScriptName))
		return false;

	m_pSandbox = new SandboxEnvironment();
	m_pSandbox->Initialise(false);
	if (m_pSandbox->LoadScene(strScriptName, true))
		std::cout << "Scene [" << strScriptName << "] loaded." << std::endl;

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

	return true;
}
//----------------------------------------------------------------------------------------------
void RenderTaskWorker::OnShutdown(void) 
{
	m_pRenderer->Shutdown();
	m_pIntegrator->Shutdown();

	m_pSandbox->Shutdown(false);
	delete m_pSandbox;
}
//----------------------------------------------------------------------------------------------
bool RenderTaskWorker::OnSynchronise(void) 
{
	return true;
}
//----------------------------------------------------------------------------------------------
bool RenderTaskCoordinator::Compute(void) 
{
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000 / 60));
	return true;
}
//----------------------------------------------------------------------------------------------
bool RenderTaskCoordinator::OnInitialise(void) 
{
	std::cout << "RenderTaskCoordinator::OnInitialise()" << std::endl;

	std::string strScriptName;
	ArgumentMap *pArgumentMap = GetArgumentMap();

	if (!pArgumentMap->GetArgument("script", strScriptName))
		return false;

	m_pSandbox = new SandboxEnvironment();
	m_pSandbox->Initialise(false);
	if (m_pSandbox->LoadScene(strScriptName, true))
		std::cout << "Scene [" << strScriptName << "] loaded." << std::endl;

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

	return true;
}
//----------------------------------------------------------------------------------------------
void RenderTaskCoordinator::OnShutdown(void)
{
	m_pRenderer->GetDevice()->Close();

	m_pRenderer->Shutdown();
	m_pIntegrator->Shutdown();

	m_pSandbox->Shutdown(false);
	delete m_pSandbox;
}
//----------------------------------------------------------------------------------------------
bool RenderTaskCoordinator::OnSynchronise(void) {
	return true;
}
//----------------------------------------------------------------------------------------------
bool RenderTaskCoordinator::OnMessageReceived(ResourceMessage *p_pMessage) 
{
	return true; 
}
//----------------------------------------------------------------------------------------------