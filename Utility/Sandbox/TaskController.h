//----------------------------------------------------------------------------------------------
//	Filename:	TaskController.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------------------------
#include <boost/asio.hpp>

#include "Logger.h"
#include "Controller.h"
#include "Task.h"

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
	Task m_task;

protected:
	bool ProcessClientInput(void);

public:
	// Constructor / Destructor
	TaskController(int p_nControllerId);
	~TaskController(void);

	// Bind socket and command parser to Controller
	bool Bind(boost::asio::ip::tcp::socket *p_pSocket, ICommandParser *p_pCommandParser);

	// Start and stop controller
	bool Start(void);
	void Stop(void);

	// Resource management event callbacks
	void OnResourceAdd(const std::vector<Resource*> &p_resourceList);
	void OnResourceRemove(int p_nResourceCount, std::vector<Resource*> &p_resourceListOut);
};