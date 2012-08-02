//----------------------------------------------------------------------------------------------
//	Filename:	TaskController.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once

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
	// Add resource to task container
	m_task.Add(p_resourceList);

	// Notify resources and coordinator (or create coord if not available)
}
//----------------------------------------------------------------------------------------------
void TaskController::OnResourceRemove(int p_nResourceCount, std::vector<Resource*> &p_resourceListOut)
{
	// Notify resources that they are now idle
	// Remove them from task
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
