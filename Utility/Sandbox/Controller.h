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
#include "Resource.h"

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
class IController
{
public:
	static bool ReadFromSocket(boost::asio::ip::tcp::socket *p_pSocket, char *p_pCommandBuffer, size_t p_nCommandBufferSize)
	{
		boost::system::error_code error;
		Int32 sizeBuffer = 0;

		// Receive length in bytes first
		size_t bytesRead = boost::asio::read(*p_pSocket, boost::asio::buffer(&sizeBuffer, sizeof(sizeBuffer)), error);
		
		// On error, exit
		if (bytesRead != sizeof(sizeBuffer))
		{
			Logger::Message("Error reading client command header!", true, Logger::Critical);
			return false;
		}

		// Convert from network to host order
		sizeBuffer = ntohl(sizeBuffer);

		// If command buffer is not large enough, exit
		if ((size_t)sizeBuffer > p_nCommandBufferSize)
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

	static bool WriteToSocket(boost::asio::ip::tcp::socket *p_pSocket, const char *p_pCommandBuffer, size_t p_nCommandBufferSize)
	{
		boost::system::error_code error;

		// Convert host to network order
		Int32 sizeBuffer = htonl((Int32)p_nCommandBufferSize);

		size_t bytesWritten = boost::asio::write(*p_pSocket, boost::asio::buffer(&sizeBuffer, sizeof(sizeBuffer)), error);
		
		// On error, exit
		if (bytesWritten != sizeof(sizeBuffer))
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
	virtual ~IController() { };

	virtual bool Bind(boost::asio::ip::tcp::socket *p_pSocket, ICommandParser *p_pCommandParser) = 0;
	virtual bool Start(void) = 0;
	virtual void Stop(void) = 0;
};
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
struct ResourceControllerInfo
{
	int ID,
		Allocation;

	float Load;

	std::string Description;
};
//----------------------------------------------------------------------------------------------
class IResourceController
	: public IController
{ 
protected:
	int m_nID;

public:
	IResourceController(int p_nID) : m_nID(p_nID) { }
	int GetID(void) const { return m_nID; }

	// Resource change events
	virtual void OnResourceAdd(const std::vector<Resource*> &p_resourceList) = 0;
	virtual void OnResourceRemove(int p_nResourceCount, std::vector<Resource*> &p_resourceListOut) = 0;

	virtual int GetResourceCount(void) const = 0;
	virtual void GetControllerInfo(ResourceControllerInfo &p_info) = 0;
};
//----------------------------------------------------------------------------------------------
