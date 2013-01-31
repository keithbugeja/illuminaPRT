//----------------------------------------------------------------------------------------------
//	Filename:	Controller.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
#include "Controller.h"
#include "ServiceManager.h"
#include "CommandParser.h"
#include "Resource.h"

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
bool IController::ReadFromSocket(boost::asio::ip::tcp::socket *p_pSocket, char *p_pCommandBuffer, size_t p_nCommandBufferSize)
{
	boost::system::error_code error;
	Int32 sizeBuffer = 0;

	// Get logger instance
	Logger *logger = ServiceManager::GetInstance()->GetLogger();

	// Receive length in bytes first
	size_t bytesRead = boost::asio::read(*p_pSocket, boost::asio::buffer(&sizeBuffer, sizeof(sizeBuffer)), error);
		
	// On error, exit
	if (bytesRead != sizeof(sizeBuffer))
	{
		logger->Write("Controller :: Error reading client command header!", LL_Critical);
		return false;
	}

	// Convert from network to host order
	sizeBuffer = ntohl(sizeBuffer);

	// If command buffer is not large enough, exit
	if ((size_t)sizeBuffer > p_nCommandBufferSize)
	{
		logger->Write("Controller :: Client command data is larger than allocated command buffer!", LL_Critical);
		return false;
	}

	// Read command data
	std::stringstream message;
	message << "Controller :: Preamble returned command size of [" << sizeBuffer << "]";
	logger->Write(message.str(), LL_Info);
	
	memset(p_pCommandBuffer, 0, p_nCommandBufferSize);
	bytesRead = boost::asio::read(*p_pSocket, boost::asio::buffer(p_pCommandBuffer, sizeBuffer), error);
		
	// If we didn't read the expected data, report an error
	if (bytesRead != sizeBuffer)
	{
		logger->Write("Controller :: Error reading client command data!", LL_Critical);
		return false;
	}

	// Message data
	message.str(std::string()); message << "Controller :: Command read [" << p_pCommandBuffer << "]";
	logger->Write(message.str(), LL_Info);

	return true;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
bool IController::WriteToSocket(boost::asio::ip::tcp::socket *p_pSocket, const char *p_pCommandBuffer, size_t p_nCommandBufferSize)
{
	Logger *logger = ServiceManager::GetInstance()->GetLogger();
	boost::system::error_code error;

	// Convert host to network order
	Int32 sizeBuffer = htonl((Int32)p_nCommandBufferSize);

	size_t bytesWritten = boost::asio::write(*p_pSocket, boost::asio::buffer(&sizeBuffer, sizeof(sizeBuffer)), error);
		
	// On error, exit
	if (bytesWritten != sizeof(sizeBuffer))
	{
		logger->Write("Controller :: Error writing client command header!", LL_Critical);
		return false;
	}

	bytesWritten =  boost::asio::write(*p_pSocket, boost::asio::buffer(p_pCommandBuffer, p_nCommandBufferSize), error);

	// On error, exit
	if (bytesWritten != p_nCommandBufferSize)
	{
		logger->Write("Controller :: Error writing client command data!", LL_Critical);
		return false;
	}

	return true;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------