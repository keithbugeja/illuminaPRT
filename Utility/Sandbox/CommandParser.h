//----------------------------------------------------------------------------------------------
//	Filename:	CommandParser.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once

#include <map>
#include <vector>
#include <string>

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
class ICommandParser
{
protected:
	std::map<std::string, std::vector<std::string> > m_commandMap;

protected:
	ICommandParser(void) { }

public:
	bool Add(const std::string &p_strCommandSpecification);
	bool Parse(const std::string &p_strCommand, std::string &p_strCommandName, std::map<std::string, std::string> &p_strArgumentMap);
	void Display(const std::string &p_strCommandName, const std::map<std::string, std::string> &p_strArgumentMap);
};
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
class ClientCommandParser 
	: public ICommandParser
{ 
public:
	ClientCommandParser(void);
};

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
class AdminCommandParser
	: public ICommandParser
{ 
public:
	AdminCommandParser(void);
};
//----------------------------------------------------------------------------------------------
