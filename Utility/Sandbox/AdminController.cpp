//----------------------------------------------------------------------------------------------
//	Filename:	AdminController.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include <sstream>
//----------------------------------------------------------------------------------------------
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
//----------------------------------------------------------------------------------------------
#include "ServiceManager.h"
#include "Logger.h"
//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
AdminController::AdminController(void) { }
//----------------------------------------------------------------------------------------------
AdminController::~AdminController(void) { }
//----------------------------------------------------------------------------------------------
bool AdminController::ProcessClientInput(void)
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
		std::string response;

		std::vector<ResourceControllerInfo> controllerInfoList;
		ServiceManager::GetInstance()->GetResourceManager()->GetControllerInfo(controllerInfoList);
		
		if (controllerInfoList.size() == 0)
		{
			response = "-:-:-:-";
		}
		else
		{
			std::stringstream strmResponse;

			for (std::vector<ResourceControllerInfo>::iterator infoIterator = controllerInfoList.begin();
				 infoIterator != controllerInfoList.end(); infoIterator++)
			{
				strmResponse << infoIterator->ID << ":" 
					<< infoIterator->Allocation << ":" 
					<< infoIterator->Load << ":" 
					<< infoIterator->Description << ";";
			}

			response = strmResponse.str().substr(0, strmResponse.str().length() - 1);
		}

		IController::WriteToSocket(m_pSocket, response.c_str(), response.length());
	}
	else if (strCommandName == "stats")
	{
		// To remove gcc warning until we implement stat functionality
		// int id = boost::lexical_cast<int>(argumentMap["id"]);

		std::string response = "0:35:150%:Scene/sponza.ilm";
		IController::WriteToSocket(m_pSocket, response.c_str(), response.length());
	}
	else if (strCommandName == "add")
	{
		int id = boost::lexical_cast<int>(argumentMap["id"]);
		int count = boost::lexical_cast<int>(argumentMap["count"]);

		std::cout << "Admin :: Request [" << count << "] resources for Task Group [" << id << "]" << std::endl;
		
		if (ServiceManager::GetInstance()->GetResourceManager()->RequestResources(id, count))
			IController::WriteToSocket(m_pSocket, "OK", 2);
		else
			IController::WriteToSocket(m_pSocket, "KO", 2);
	}
	else if (strCommandName == "sub")
	{
		int id = boost::lexical_cast<int>(argumentMap["id"]);
		int count = boost::lexical_cast<int>(argumentMap["count"]);

		std::cout << "Admin :: Yielding [" << count << "] resources from Task Group [" << id << "]" << std::endl;

		if (ServiceManager::GetInstance()->GetResourceManager()->ReleaseResources(id, count))
			IController::WriteToSocket(m_pSocket, "OK", 2);
		else
			IController::WriteToSocket(m_pSocket, "KO", 2);
	}
	else if (strCommandName == "set")
	{
		bool bSuccess = false;

		int id = boost::lexical_cast<int>(argumentMap["id"]);
		int count = boost::lexical_cast<int>(argumentMap["count"]);

		std::cout << "Admin :: Setting resource count to [" << count << "] for Task Group [" << id << "]" << std::endl;

		IResourceController *pController = ServiceManager::GetInstance()->GetResourceManager()->GetInstance(id);

		if (pController)
		{
			int countDifference = count - pController->GetResourceCount();
			
			if (countDifference < 0)
			{
				std::cout << "Admin :: Yielding [" << -countDifference << "] resources from Task Group [" << id << "]" << std::endl;
				bSuccess = ServiceManager::GetInstance()->GetResourceManager()->ReleaseResources(id, -countDifference);
			}
			else if (countDifference > 0)
			{
				std::cout << "Admin :: Request [" << countDifference << "] resources for Task Group [" << id << "]" << std::endl;
				bSuccess = ServiceManager::GetInstance()->GetResourceManager()->RequestResources(id, countDifference);
			}
		}

		if (bSuccess)
			IController::WriteToSocket(m_pSocket, "OK", 2);
		else
			IController::WriteToSocket(m_pSocket, "KO", 2);
	}
	else if (strCommandName == "exit")
	{
		IController::WriteToSocket(m_pSocket, "OK", 2);
		return false;
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool AdminController::Bind(boost::asio::ip::tcp::socket *p_pSocket, ICommandParser *p_pCommandParser)
{
	BOOST_ASSERT(p_pSocket != NULL && p_pCommandParser != NULL);

	m_pCommandParser = p_pCommandParser;
	m_pSocket = p_pSocket;

	return true;
}
//----------------------------------------------------------------------------------------------
bool AdminController::Start(void) 
{
	while (ProcessClientInput());
	return true;
}
//----------------------------------------------------------------------------------------------
void AdminController::Stop(void) { }
//----------------------------------------------------------------------------------------------
