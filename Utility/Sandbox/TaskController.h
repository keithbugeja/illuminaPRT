//----------------------------------------------------------------------------------------------
//	Filename:	TaskGroupController.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------------------------
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include "Logger.h"
#include "Controller.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
//	The task group controller should contain some state for the task group. This state shares
//	generic information regarding the task group (but not its domain specific application and
//	implementation) as well as task-specific details.
//
//	The controller is also reponsible of processing client requests / commands. Specialised
//	command processors are applied to received commands, which are either parsed and passed
//	on to the task group, or executed at the head node, or both.
//----------------------------------------------------------------------------------------------
class TaskController
	: public IResourceController
{
protected:
	boost::asio::ip::tcp::socket *m_pSocket;
	ICommandParser *m_pCommandParser; 

	int m_nId;

protected:

public:
	TaskController(int p_nControllerId)
		: m_nId(p_nControllerId)
	{ }

	~TaskController(void) { }

	int GetId(void) const { return m_nId; }

	bool Bind(boost::asio::ip::tcp::socket *p_pSocket, ICommandParser *p_pCommandParser)
	{
		BOOST_ASSERT(p_pSocket != NULL && p_pCommandParser != NULL);

		m_pCommandParser = p_pCommandParser;
		m_pSocket = p_pSocket;

		return true;
	}

	bool Start(void) 
	{
		ProcessClientInput();

		// Ok, now we wait for client to start sending commands... 
		//	There is a number of command variants:
		//		Some are executed only on head node
		//		Some are executed both on head node and at remote tasks
		//		Some are parsed on head node and executed only at remote tasks
		return true;
	}
	
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
		if (strCommandName == "init")
		{
			IController::WriteToSocket(m_pSocket, "OK", 2);
		}
		else if (strCommandName == "play")
		{
			IController::WriteToSocket(m_pSocket, "OK", 2);
		}
		else if (strCommandName == "pause")
		{
			IController::WriteToSocket(m_pSocket, "OK", 2);
		}
		else if (strCommandName == "exit")
		{
			IController::WriteToSocket(m_pSocket, "OK", 2);
		}

		return true;
	}

	// Needs reference to global state manager
	// Requires open client connection
};