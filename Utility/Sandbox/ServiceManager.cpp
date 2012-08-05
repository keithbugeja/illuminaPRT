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
void ServiceManager::Initialise(int p_nServicePort, int p_nAdminPort, const std::string p_strPath, bool p_bVerbose)
{ 
	m_nServicePort = p_nServicePort;
	m_nAdminPort = p_nAdminPort;
	m_cwdPath = p_strPath;
	m_bVerbose = p_bVerbose;
}
//----------------------------------------------------------------------------------------------
void ServiceManager::Shutdown(void) { }
//----------------------------------------------------------------------------------------------
void ServiceManager::Stop(void) { }
//----------------------------------------------------------------------------------------------
void ServiceManager::Start(void) 
{
	try {
		boost::filesystem::current_path(m_cwdPath);
		std::cout << "Working directory [" << m_cwdPath.string() << "]" << std::endl;;
	} catch (...) { std::cerr << "Error : Unable to set working directory to " << m_cwdPath.string() << std::endl; }

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
	ServiceManager::GetInstance()->GetResourceManager()->Me()->Start(&pipeline);
}
//----------------------------------------------------------------------------------------------
void ServiceManager::RunAsMaster(void)
{
	Logger::Message("Starting Illumina PRT Service Manager...", m_bVerbose);
		
	// Kick admin service thread
	boost::thread adminThread(
			boost::bind(&ServiceManager::AdminService, this));

	// Client thread
	std::stringstream message;
	message << "Service :: Listening for connections on port " << m_nServicePort << "...";
	Logger::Message(message.str(), m_bVerbose);

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
	std::stringstream message;
	message << "Admin :: Listening for connections on port " << m_nAdminPort << "...";
	Logger::Message(message.str(), m_bVerbose);

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
		std::stringstream message;
		message << "Master :: Accepting connection from [" << p_pSocket->remote_endpoint().address().to_string() << "]";
		Logger::Message(message.str(), m_bVerbose);
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
		std::stringstream message;
		message << "Closing connection from [" << p_pSocket->remote_endpoint().address().to_string() << "]";
		Logger::Message(message.str(), m_bVerbose);
	}

	p_pSocket->close();
	delete p_pSocket;
}
//----------------------------------------------------------------------------------------------