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
#include "ServiceManager.h"
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

	// Get logger instance
	Logger *logger = ServiceManager::GetInstance()->GetLogger();

	// Read from socket
	IController::ReadFromSocket(m_pSocket, m_pCommandBuffer, 1024);

	// Convert input to string
	strCommand = m_pCommandBuffer;

	// Parse command
	m_pCommandParser->Parse(strCommand, strCommandName, argumentMap);
	m_pCommandParser->Display(strCommandName, argumentMap);

	// Process commands
	if (strCommandName == "init")
	{
		std::stringstream argumentStream;
		argumentStream << "taskid=" << GetID()
		<< ";job_user=" << argumentMap["job_user"]
		<< ";job_name=" << argumentMap["job_name"]
		<< ";device_override=" << argumentMap["device_override"]
		<< ";device_width=" << argumentMap["device_width"]
		<< ";device_height=" << argumentMap["device_height"]
		<< ";device_type=" << argumentMap["device_type"]
		<< ";device_stream_ip=" << argumentMap["device_stream_ip"]
		<< ";device_stream_port=" << argumentMap["device_stream_port"]
		<< ";device_stream_codec=" << argumentMap["device_stream_codec"]
		<< ";device_stream_bitrate=" << argumentMap["device_stream_bitrate"]
		<< ";device_stream_framerate=" << argumentMap["device_stream_framerate"]
		<< ";device_sequence_prefix=" << argumentMap["device_sequence_prefix"]
		<< ";device_sequence_details=" << argumentMap["device_sequence_details"]
		<< ";device_sequence_format=" << argumentMap["device_sequence_format"]
		<< ";device_sequence_bufferedframes=" << argumentMap["device_sequence_bufferedframes"]
		<< ";device_image_prefix=" << argumentMap["device_image_prefix"]
		<< ";device_image_format=" << argumentMap["device_image_format"]
		<< ";device_image_timestamp=" << argumentMap["device_image_timestamp"]
		<< ";tile_distribution_adaptive=" << argumentMap["tile_distribution_adaptive"]
		<< ";tile_distribution_batchsize=" << argumentMap["tile_distribution_batchsize"]
		<< ";tile_width=" << argumentMap["tile_width"]
		<< ";tile_height=" << argumentMap["tile_height"]
		<< ";resource_cap_min=" << argumentMap["resource_cap_min"]
		<< ";resource_cap_max=" << argumentMap["resource_cap_max"]
		<< ";resource_deadline_fps=" << argumentMap["resource_deadline_fps"]
		<< ";resource_deadline_enabled=" << argumentMap["resource_deadline_enabled"]
		<< ";script_name=" << argumentMap["script_name"]
		<< ";";

		m_strArguments = argumentStream.str();

		IController::WriteToSocket(m_pSocket, "OK", 2);
	}
	else if (strCommandName == "path")
	{
		std::stringstream pathCommandStream;

		pathCommandStream << "command=" << strCommandName
			<< ";delta=" << argumentMap["delta"]
			<< ";vertices=" << argumentMap["vertices"] 
			<< ";";

		if (m_task.HasCoordinator())
		{
			std::stringstream message; message << "TaskController :: Sending [" << pathCommandStream.str() << "]" << std::endl;
			logger->Write(message.str(), LL_Info);

			Resource::Send(pathCommandStream.str(), m_task.GetCoordinatorID(), true);
		}

		IController::WriteToSocket(m_pSocket, "OK", 2);
	}
	else if (strCommandName == "pathex")
	{
		std::stringstream pathCommandStream;

		pathCommandStream << "command=" << strCommandName
			<< ";delta=" << argumentMap["delta"]
			<< ";vertices=" << argumentMap["vertices"] 
			<< ";";

		if (m_task.HasCoordinator())
		{
			std::stringstream message; message << "TaskController :: Sending [" << pathCommandStream.str() << "]" << std::endl;
			logger->Write(message.str(), LL_Info);

			Resource::Send(pathCommandStream.str(), m_task.GetCoordinatorID(), true);
		}
		
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
			std::stringstream message; message << "TaskController :: Sending [" << moveCommandStream.str() << "]" << std::endl;
			logger->Write(message.str(), LL_Info);

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
	else
	{
		IController::WriteToSocket(m_pSocket, "KO", 2);
	}

	return true;
}
//----------------------------------------------------------------------------------------------
void TaskController::GetControllerInfo(ResourceControllerInfo &p_info)
{
	p_info.ID = GetID();
	p_info.Allocation = m_task.Size();
	p_info.Description = m_strArguments;
	p_info.Load =  ((float)GetResourceCount()) / (float)(ServiceManager::GetInstance()->GetResourceManager()->GetResourceCount());
}
//----------------------------------------------------------------------------------------------
int TaskController::GetResourceCount(void) const
{
	return m_task.Size();
}
//----------------------------------------------------------------------------------------------
