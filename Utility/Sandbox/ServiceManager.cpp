//----------------------------------------------------------------------------------------------
//	Filename:	ServiceManager.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include "ServiceManager.h"
#include "Worker.h"

#include "RenderTaskPipeline.h"
//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
bool ServiceManager::IsVerbose(void) const {
	return m_bVerbose;
}
//----------------------------------------------------------------------------------------------
ResourceManager *ServiceManager::GetResourceManager(void) { 
	return &m_resourceManager; 
}
//----------------------------------------------------------------------------------------------
Logger *ServiceManager::GetLogger(void) { 
	return &m_logger; 
}
//----------------------------------------------------------------------------------------------
void ServiceManager::Initialise(int p_nServicePort, int p_nAdminPort, const std::string p_strPath, bool p_bVerbose)
{ 
	m_nServicePort = p_nServicePort;
	m_nAdminPort = p_nAdminPort;
	m_cwdPath = p_strPath;
	m_bVerbose = p_bVerbose;

	m_logger.SetLoggingFilter(p_bVerbose ? LL_All : LL_ErrorLevel);
}
//----------------------------------------------------------------------------------------------
void ServiceManager::Shutdown(void) { }
//----------------------------------------------------------------------------------------------
void ServiceManager::Stop(void) { }
//----------------------------------------------------------------------------------------------
void ServiceManager::Start(void) 
{
	// Set working directory
	try {
		boost::filesystem::current_path(m_cwdPath);
		std::stringstream message; message << "ServiceManager :: Working directory [" << m_cwdPath.string() << "]";
		GetLogger()->Write(message.str(), LL_Info);
	} catch (...) { 
		std::stringstream message; message << "ServiceManager :: Unable to set working directory to [" << m_cwdPath.string() << "]";
		GetLogger()->Write(message.str(), LL_Critical);
		return;
	}

	// Initialise resource manager and run process according to rank/id
	m_resourceManager.Initialise();

	if (m_resourceManager.WhatAmI() == ResourceManager::Master) 
		RunAsMaster();
	else 
		RunAsResource();

	m_resourceManager.Shutdown();
}
//----------------------------------------------------------------------------------------------
void ServiceManager::RunAsResource(void)
{
	RenderTaskPipeline pipeline;

	// Start resource
	m_resourceManager.Me()->Start(&pipeline);
	// ServiceManager::GetInstance()->GetResourceManager()->Me()->Start(&pipeline);
}
//----------------------------------------------------------------------------------------------
void ServiceManager::RunAsMaster(void)
{
	// Kick admin service thread
	boost::thread adminThread(
			boost::bind(&ServiceManager::AdminService, this));

	// Client thread
	std::stringstream message;
	message << "ServiceManager :: Listening for client connections on port " << m_nServicePort << "..." << std::endl;
	message << "ServiceManager :: Started";
	m_logger.Write(message.str(), LL_Info);

	// Start server listening
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), m_nServicePort);
	boost::asio::ip::tcp::acceptor acceptor(m_ioService, endpoint);
		
	for (;;)
	{
		boost::asio::ip::tcp::socket *pSocket = 
			new boost::asio::ip::tcp::socket(m_ioService);

		acceptor.accept(*pSocket);

		boost::thread handlerThread(
			boost::bind(&ServiceManager::AcceptConnection, this, pSocket, false));
	}
}
//----------------------------------------------------------------------------------------------
void ServiceManager::AdminService(void)
{
	std::stringstream message; message << "ServiceManager :: Listening for admin connections on port " << m_nAdminPort << "...";
	ServiceManager::GetInstance()->GetLogger()->Write(message.str(), LL_Info);

	// Start server listening
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), m_nAdminPort);
	boost::asio::ip::tcp::acceptor acceptor(m_ioService, endpoint);
		
	for (;;)
	{
		boost::asio::ip::tcp::socket *pSocket = 
			new boost::asio::ip::tcp::socket(m_ioService);

		acceptor.accept(*pSocket);

		boost::thread handlerThread(
			boost::bind(&ServiceManager::AcceptConnection, this, pSocket, true));
	}
}
//----------------------------------------------------------------------------------------------
void ServiceManager::AcceptConnection(boost::asio::ip::tcp::socket *p_pSocket, bool p_bIsAdmin)
{
	{
		std::stringstream message; message << "ServiceManager :: Accepting connection from [" << p_pSocket->remote_endpoint().address().to_string() << "]";
		ServiceManager::GetInstance()->GetLogger()->Write(message.str(), LL_Info);
	}

	if (p_bIsAdmin)
	{
		AdminController *pController = new AdminController();
			
		pController->Bind(p_pSocket, &m_adminCommandParser);
		pController->Start();

		delete pController;
	}
	else
	{
		TaskController *pController =
			m_resourceManager.CreateInstance<TaskController>();

		pController->Bind(p_pSocket, &m_clientCommandParser);
		pController->Start();

		m_resourceManager.DestroyInstance(pController);
	}

	{
		std::stringstream message; message << "ServiceManager :: Closing connection from [" << p_pSocket->remote_endpoint().address().to_string() << "]";
		ServiceManager::GetInstance()->GetLogger()->Write(message.str(), LL_Info);
	}

	p_pSocket->close();
	delete p_pSocket;
}
//----------------------------------------------------------------------------------------------