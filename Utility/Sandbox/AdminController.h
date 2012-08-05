//----------------------------------------------------------------------------------------------
//	Filename:	AdminController.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include <boost/asio.hpp>
//----------------------------------------------------------------------------------------------
#include "Controller.h"
//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
class AdminController
	: public IController
{
protected:
	boost::asio::ip::tcp::socket *m_pSocket;
	ICommandParser *m_pCommandParser; 

protected:
	bool ProcessClientInput(void);

public:
	AdminController(void);
	~AdminController(void);

	bool Bind(boost::asio::ip::tcp::socket *p_pSocket, ICommandParser *p_pCommandParser);

	bool Start(void); 
	void Stop(void);
};