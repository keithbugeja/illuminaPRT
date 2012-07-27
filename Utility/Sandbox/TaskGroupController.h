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
		return true;
	}
	
	// Needs reference to global state manager
	// Requires open client connection
};