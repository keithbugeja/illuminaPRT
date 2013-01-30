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

#include "CommandParser.h"
#include "Resource.h"

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
class IController
{
public:
	static bool ReadFromSocket(boost::asio::ip::tcp::socket *p_pSocket, char *p_pCommandBuffer, size_t p_nCommandBufferSize);
	static bool WriteToSocket(boost::asio::ip::tcp::socket *p_pSocket, const char *p_pCommandBuffer, size_t p_nCommandBufferSize);

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
