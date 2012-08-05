//----------------------------------------------------------------------------------------------
//	Filename:	CommandParser.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------------------------
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "Logger.h"

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
class ICommandParser
{
protected:
	std::map<std::string, std::vector<std::string> > m_commandMap;

protected:
	ICommandParser(void) { }

public:
	bool AddCommand(const std::string &p_strCommandSpecification)
	{
		std::vector<std::string> argumentList;
		boost::split(argumentList, p_strCommandSpecification, boost::is_any_of("|"));

		if (argumentList.empty())
			return false;

		std::vector<std::string> commandArgs;

		for (int j = 1; j < argumentList.size(); j++)
			commandArgs.push_back(argumentList[j]);

		m_commandMap[argumentList[0]] = commandArgs;

		return true;
	}

	bool Parse(const std::string &p_strCommand, std::string &p_strCommandName, std::map<std::string, std::string> &p_strArgumentMap)
	{
		std::vector<std::string> argumentList;
		boost::split(argumentList, p_strCommand, boost::is_any_of("|"));

		if (argumentList.empty() ||
			m_commandMap.find(argumentList[0]) == m_commandMap.end())
			return false;

		p_strCommandName = argumentList[0];
		p_strArgumentMap.clear();

		if (argumentList.size() > 1)
		{
			const std::vector<std::string> &argumentNameList = m_commandMap[p_strCommandName];
			for (int j = 0; j < argumentNameList.size(); j++)
			{
				p_strArgumentMap[argumentNameList[j]] = argumentList[j + 1];
			}
		}

		return true;
	}

	void DisplayCommand(const std::string &p_strCommandName, const std::map<std::string, std::string> &p_strArgumentMap)
	{
		std::cout << "Command : [" << p_strCommandName << "]" << std::endl;
		
		for (std::map<std::string, std::string>::const_iterator argIterator = p_strArgumentMap.begin();
			 argIterator != p_strArgumentMap.end(); argIterator++)
		{
			std::cout << "\t[" << argIterator->first << " = " << argIterator->second << "]" << std::endl;
		}
	}
};
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
class ClientCommandParser 
	: public ICommandParser
{ 
public:
	ClientCommandParser(void) 
	{
		this->AddCommand("init|script|max|min|width|height|fps|use");
		this->AddCommand("exit");
	}
};

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
class AdminCommandParser
	: public ICommandParser
{ 
public:
	AdminCommandParser(void)
	{
		this->AddCommand("count");
		this->AddCommand("procs");
		this->AddCommand("stats|id");
		this->AddCommand("add|id|count");
		this->AddCommand("sub|id|count");
		this->AddCommand("set|id|count");
		this->AddCommand("exit");
	}
};
//----------------------------------------------------------------------------------------------
