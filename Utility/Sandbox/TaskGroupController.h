//----------------------------------------------------------------------------------------------
//	Filename:	TaskGroupController.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------------------------
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include "Logger.h"
using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
//	TaskGroupState
//----------------------------------------------------------------------------------------------
class TaskGroupState
{
	std::vector<Task*> m_taskList;
};

//----------------------------------------------------------------------------------------------
//	The task group controller should contain some state for the task group. This state shares
//	generic information regarding the task group (but not its domain specific application and
//	implementation) as well as task-specific details.
//
//	The controller is also reponsible of processing client requests / commands. Specialised
//	command processors are applied to received commands, which are either parsed and passed
//	on to the task group, or executed at the head node, or both.
//----------------------------------------------------------------------------------------------
class TaskGroupController
{
protected:
	boost::asio::ip::tcp::socket *m_pSocket;

public:
	TaskGroupController(void) { }
	~TaskGroupController(void) { }

	bool Bind(boost::asio::ip::tcp::socket *p_pSocket)
	{
		BOOST_ASSERT(p_pSocket != NULL);
		m_pSocket = p_pSocket;

		return true;
	}

	bool Start(void) 
	{
		// Ok, now we wait for client to start sending commands... 
		//	There is a number of command variants:
		//		Some are executed only on head node
		//		Some are executed both on head node and at remote tasks
		//		Some are parsed on head node and executed only at remote tasks
		return true;
	}
	
	// Needs reference to global state manager
	// Requires open client connection
};