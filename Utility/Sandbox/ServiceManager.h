//----------------------------------------------------------------------------------------------
//	Filename:	ServiceManager.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------------------------
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

#include <System/Singleton.h>

#include "Logger.h"
#include "ResourceManager.h"
#include "TaskController.h"
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
	ResourceManager *GetResourceManager(void);

public:
	void Initialise(int p_nServicePort, int p_nAdminPort, const std::string p_strPath, bool p_bVerbose);
	void Shutdown(void);

	void Start(void); 
	void Stop(void);

	void RunAsMaster(void);
	void RunAsResource(void);

	void AdminService(void);

	void AcceptConnection(boost::asio::ip::tcp::socket *p_pSocket, bool p_bIsAdmin);
};