//----------------------------------------------------------------------------------------------
//	Filename:	AdminController.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------------------------
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include "Logger.h"
#include "Controller.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
class AdminController
	: public IController
{
protected:
	boost::asio::ip::tcp::socket *m_pSocket;
	ICommandParser *m_pCommandParser; 

protected:
	bool ProcessClientInput(void)
	{
		char m_pCommandBuffer[1024];

		std::string strCommandName, strCommand;
		std::map<std::string, std::string> argumentMap;

		// Read from socket
		IController::ReadFromSocket(m_pSocket, m_pCommandBuffer, 1024);

		// Convert input to string
		strCommand = m_pCommandBuffer;

		// Parse command
		m_pCommandParser->Parse(strCommand, strCommandName, argumentMap);
		m_pCommandParser->DisplayCommand(strCommandName, argumentMap);

		// Process commands
		if (strCommandName == "count")
		{
			IController::WriteToSocket(m_pSocket, "OK", 2);
		}
		else if (strCommandName == "procs")
		{
			std::string response = "0:35:150%:Scene/sponza.ilm;0:35:150%:Scene/sponza.ilm;0:35:150%:Scene/sponza.ilm;0:35:150%:Scene/sponza.ilm";
			IController::WriteToSocket(m_pSocket, response.c_str(), response.length());
		}
		else if (strCommandName == "stats")
		{
			int id = boost::lexical_cast<int>(argumentMap["id"]);
			std::string response = "0:35:150%:Scene/sponza.ilm";
			IController::WriteToSocket(m_pSocket, response.c_str(), response.length());
		}
		else if (strCommandName == "add")
		{
			int id = boost::lexical_cast<int>(argumentMap["id"]);
			int count = boost::lexical_cast<int>(argumentMap["count"]);

			std::cout << "Admin :: Request [" << count << "] resources for Task Group [" << id << "]" << std::endl;

			IController::WriteToSocket(m_pSocket, "OK", 2);
		}
		else if (strCommandName == "sub")
		{
			int id = boost::lexical_cast<int>(argumentMap["id"]);
			int count = boost::lexical_cast<int>(argumentMap["count"]);

			std::cout << "Admin :: Yielding [" << count << "] resources from Task Group [" << id << "]" << std::endl;

			IController::WriteToSocket(m_pSocket, "OK", 2);
		}
		else if (strCommandName == "set")
		{
			int id = boost::lexical_cast<int>(argumentMap["id"]);
			int count = boost::lexical_cast<int>(argumentMap["count"]);

			std::cout << "Admin :: Setting resource count to [" << count << "] for Task Group [" << id << "]" << std::endl;

			IController::WriteToSocket(m_pSocket, "OK", 2);
		}
		else if (strCommandName == "exit")
		{
			IController::WriteToSocket(m_pSocket, "OK", 2);
			return false;
		}

		return true;
	}

public:
	AdminController(void) { }
	~AdminController(void) { }

	bool Bind(boost::asio::ip::tcp::socket *p_pSocket, ICommandParser *p_pCommandParser)
	{
		BOOST_ASSERT(p_pSocket != NULL && p_pCommandParser != NULL);

		m_pCommandParser = p_pCommandParser;
		m_pSocket = p_pSocket;

		return true;
	}

	bool Start(void) 
	{
		while (ProcessClientInput());
		return true;
	}

	void Stop(void) { }
};