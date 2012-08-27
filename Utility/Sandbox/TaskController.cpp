//----------------------------------------------------------------------------------------------
//	Filename:	TaskController.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
//----------------------------------------------------------------------------------------------
#include "TaskController.h"
#include "Logger.h"
//----------------------------------------------------------------------------------------------
TaskController::TaskController(int p_nControllerId)
	: IResourceController(p_nControllerId)
{ }
//----------------------------------------------------------------------------------------------
TaskController::~TaskController(void) { }
//----------------------------------------------------------------------------------------------
void TaskController::OnResourceAdd(const std::vector<Resource*> &p_resourceList)
{
	BOOST_ASSERT(p_resourceList.size() > 0);

	// Add resource to task container
	m_task.Add(p_resourceList);

	// Notify resources and coordinator (or create coord if not available)
	if (!m_task.HasCoordinator()) m_task.SetCoordinator(p_resourceList[0]);
	Resource::Register(m_strArguments, m_task.GetCoordinatorID(), p_resourceList);
}
//----------------------------------------------------------------------------------------------
void TaskController::OnResourceRemove(int p_nResourceCount, std::vector<Resource*> &p_resourceListOut)
{
	BOOST_ASSERT(p_nResourceCount <= m_task.Size());

	p_resourceListOut.clear();

	// Do we have to unregister whole task?
	if (p_nResourceCount == m_task.Size())
	{
		for (int nResourceIdx = 0; nResourceIdx < p_nResourceCount; nResourceIdx++)
			p_resourceListOut.push_back(m_task[nResourceIdx]);
	}
	else
	{
		for(int nIdx = 0, nResourceIdx = 0; nResourceIdx < p_nResourceCount; nIdx++)
		{
			if (m_task[nIdx] != m_task.GetCoordinator())
			{
				p_resourceListOut.push_back(m_task[nIdx]);
				nResourceIdx++;
			}
		}
	}

	// Unregister resources
	Resource::Unregister(m_task.GetCoordinatorID(), p_resourceListOut);

	// Remove from task
	m_task.Remove(p_resourceListOut);
}
//----------------------------------------------------------------------------------------------
bool TaskController::Bind(boost::asio::ip::tcp::socket *p_pSocket, ICommandParser *p_pCommandParser)
{
	BOOST_ASSERT(p_pSocket != NULL && p_pCommandParser != NULL);

	m_pCommandParser = p_pCommandParser;
	m_pSocket = p_pSocket;

	return true;
}
//----------------------------------------------------------------------------------------------
bool TaskController::Start(void) 
{
	while(ProcessClientInput());
	return true;
}
//----------------------------------------------------------------------------------------------
void TaskController::Stop(void) { }
//----------------------------------------------------------------------------------------------
bool TaskController::ProcessClientInput(void)
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
		// Need to push them to local structure too
		std::stringstream argumentStream;
		argumentStream << "script=" << argumentMap["script"] 
		<< ";min=" << argumentMap["min"] 
		<< ";max=" << argumentMap["max"]
		<< ";width=" << argumentMap["width"]
		<< ";height=" << argumentMap["height"]
		<< ";batchsize=" << argumentMap["batchsize"]
		<< ";useadaptive=" << argumentMap["useadaptive"]
		<< ";fps=" << argumentMap["fps"]
		<< ";usefps=" << argumentMap["usefps"]
		<< ";rtpaddr=" << argumentMap["rtpaddr"]
		<< ";rtpport=" << argumentMap["rtpport"]
		<< ";usertp=" << argumentMap["usertp"]
		<< ";"; 

		m_strArguments = argumentStream.str();

		IController::WriteToSocket(m_pSocket, "OK", 2);
	}
	else if (strCommandName == "move")
	{
		std::stringstream moveCommandStream;

		moveCommandStream << "command=" << strCommandName 
		<< ";action=" << argumentMap["action"]
		<< ";direction=" << argumentMap["direction"]
		<< ";";

		if (m_task.HasCoordinator())
		{
			std::cout << "Sending [" << moveCommandStream.str() << "]" << std::endl;
			Resource::Send(moveCommandStream.str(), m_task.GetCoordinatorID(), true);
		}
		
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
		return false;
	}

	return true;
}
//----------------------------------------------------------------------------------------------
void TaskController::GetControllerInfo(ResourceControllerInfo &p_info)
{
	p_info.ID = GetID();
	p_info.Allocation = m_task.Size();
	p_info.Description = "No Description yet. Need to implement TASK PIPELINE";
	p_info.Load = 0.5;
}
//----------------------------------------------------------------------------------------------
int TaskController::GetResourceCount(void) const
{
	return m_task.Size();
}
//----------------------------------------------------------------------------------------------

