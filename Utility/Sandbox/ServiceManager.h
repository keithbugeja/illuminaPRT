//----------------------------------------------------------------------------------------------
//	Filename:	ServiceManager.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------------------------
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include <System/Singleton.h>

#include "Logger.h"
#include "TaskGroupControllerManager.h"
#include "ResourceManager.h"
#include "AdminController.h"
#include "CommandParser.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
class ServiceManager 
	: public TSingleton<ServiceManager>
{
protected:
	ResourceManager m_resourceManager;

	AdminCommandParser m_adminCommandParser;
	ClientCommandParser m_clientCommandParser;

	boost::asio::io_service m_ioService;
	boost::filesystem::path m_cwdPath;
	
	bool m_bVerbose;
	
	int m_nServicePort,
		m_nAdminPort;

public:
	ResourceManager *GetResourceManager(void) { return &m_resourceManager; }

public:
	void Initialise(int p_nServicePort, int p_nAdminPort, const std::string p_strPath, bool p_bVerbose)
	{ 
		m_nServicePort = p_nServicePort;
		m_nAdminPort = p_nAdminPort;
		m_cwdPath = p_strPath;
		m_bVerbose = p_bVerbose;
	}

	void Shutdown(void)
	{
	}

	void Start(void) 
	{
		try {
			boost::filesystem::current_path(m_cwdPath);
			std::cout << "Working directory [" << m_cwdPath.string() << "]" << std::endl;;
		} catch (...) { std::cerr << "Error : Unable to set working directory to " << m_cwdPath.string() << std::endl; }

		/*
		// Initialise MPI with support for calls from multiple-threads
		int provided; MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

		// Get Process Rank
		int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);

		// If rank is zero, this process will run as server front end
		if (rank == 0) RunAsServer();
		else RunAsResource();

		// Terminate application
		MPI_Finalize();
		*/

		m_resourceManager.Initialise();

		if (m_resourceManager.WhatAmI() == ResourceManager::Master) 
			RunAsServer();
		else 
			RunAsResource();

		m_resourceManager.Shutdown();
	}

	void RunAsServer(void)
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

	void AdminService(void)
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

	void AcceptConnection(boost::asio::ip::tcp::socket *p_pSocket, bool p_bIsAdmin)
	{
		std::cout << "Accepting connection from [" << p_pSocket->remote_endpoint().address().to_string() << "]" << std::endl;

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

		std::cout << "Closing connection from [" << p_pSocket->remote_endpoint().address().to_string() << "]" << std::endl;
		
		p_pSocket->close();
		delete p_pSocket;
	}

	void RunAsResource(void)
	{
		// Initialise own PE structure and go to standby mode
		// Wait for task group assignment
	}
};