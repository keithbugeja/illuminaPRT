//----------------------------------------------------------------------------------------------
//	Filename:	Controller.h
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
#include "CommandParser.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
class IController
{
public:
	static bool ReadFromSocket(boost::asio::ip::tcp::socket *p_pSocket, char *p_pCommandBuffer, int p_nCommandBufferSize)
	{
		boost::system::error_code error;
		unsigned int sizeBuffer = 0;

		// Receive length in bytes first
		size_t bytesRead = boost::asio::read(*p_pSocket, boost::asio::buffer(&sizeBuffer, sizeof(sizeBuffer)), error);
		
		// On error, exit
		if (bytesRead != sizeof(sizeBuffer))
		{
			Logger::Message("Error reading client command header!", true, Logger::Critical);
			return false;
		}

		// If command buffer is not large enough, exit
		if (sizeBuffer > p_nCommandBufferSize)
		{
			Logger::Message("Client command data is larger than allocated command buffer!", true, Logger::Critical);
			return false;
		}

		// Read command data
		std::stringstream message;
		message << "ReadFromSocket() :: Command Size [" << sizeBuffer << "]" << std::endl;
		Logger::Message(message.str(), true);
	
		memset(p_pCommandBuffer, 0, p_nCommandBufferSize);
		bytesRead = boost::asio::read(*p_pSocket, boost::asio::buffer(p_pCommandBuffer, sizeBuffer), error);
		
		// If we didn't read the expected data, report an error
		if (bytesRead != sizeBuffer)
		{
			Logger::Message("Error reading client command data!", true, Logger::Critical);
			return false;
		}

		std::cout << "[" << p_pCommandBuffer << "]" << std::endl;

		return true;
	}

	static bool WriteToSocket(boost::asio::ip::tcp::socket *p_pSocket, const char *p_pCommandBuffer, int p_nCommandBufferSize)
	{
		boost::system::error_code error;

		size_t bytesWritten = boost::asio::write(*p_pSocket, boost::asio::buffer(&p_nCommandBufferSize, sizeof(int)), error);
		
		// On error, exit
		if (bytesWritten != sizeof(int))
		{
			Logger::Message("Error writing client command header!", true, Logger::Critical);
			return false;
		}

		bytesWritten =  boost::asio::write(*p_pSocket, boost::asio::buffer(p_pCommandBuffer, p_nCommandBufferSize), error);

		// On error, exit
		if (bytesWritten != p_nCommandBufferSize)
		{
			Logger::Message("Error writing client command data!", true, Logger::Critical);
			return false;
		}

		return true;
	}

public:
	virtual bool Bind(boost::asio::ip::tcp::socket *p_pSocket, ICommandParser *p_pCommandParser) = 0;
	virtual bool Start(void) = 0;
};