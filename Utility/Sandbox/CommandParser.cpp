//----------------------------------------------------------------------------------------------
//	Filename:	CommandParser.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------------------------
#include "ServiceManager.h"
#include "CommandParser.h"

//----------------------------------------------------------------------------------------------
bool ICommandParser::Add(const std::string &p_strCommandSpecification)
{
	std::vector<std::string> argumentList;
	boost::split(argumentList, p_strCommandSpecification, boost::is_any_of("|"));

	if (argumentList.empty())
		return false;

	std::vector<std::string> commandArgs;

	for (size_t j = 1; j < argumentList.size(); j++)
		commandArgs.push_back(argumentList[j]);

	m_commandMap[argumentList[0]] = commandArgs;

	return true;
}
//----------------------------------------------------------------------------------------------
bool ICommandParser::Parse(const std::string &p_strCommand, std::string &p_strCommandName, std::map<std::string, std::string> &p_strArgumentMap)
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
		for (size_t j = 0; j < argumentNameList.size(); j++)
		{
			p_strArgumentMap[argumentNameList[j]] = argumentList[j + 1];
		}
	}

	return true;
}
//----------------------------------------------------------------------------------------------
void ICommandParser::Display(const std::string &p_strCommandName, const std::map<std::string, std::string> &p_strArgumentMap)
{
	std::stringstream outputString;

	outputString << "CommandParser :: Command Name [" << p_strCommandName << "], Argument List : [";

	for (std::map<std::string, std::string>::const_iterator argIterator = p_strArgumentMap.begin();
		argIterator != p_strArgumentMap.end(); argIterator++)
	{
		outputString << std::endl << "\t[" << argIterator->first << " = " << argIterator->second << "]";
	}

	outputString << "]";

	ServiceManager::GetInstance()->GetLogger()->Write(outputString.str(), LL_Info);
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
ClientCommandParser::ClientCommandParser(void) 
{
	this->Add("init2|job_user|job_name"
		  "|device_override|device_width|device_height|device_type"
		  "|device_stream_ip|device_stream_port|device_stream_codec|device_stream_bitrate|device_stream_framerate"
		  "|device_sequence_prefix|device_sequence_details|device_sequence_format|device_sequence_bufferedframes"
		  "|device_image_prefix|device_image_format|device_image_timestamp"
		  "|tile_distribution_adaptive|tile_distribution_batchsize"
		  "|tile_width|tile_height"
		  "|resource_cap_min|resource_cap_max|resource_deadline_fps|resource_deadline_enabled"
		  "|script_name");

	this->Add("init|username|jobname|script|max|min|width|height|batchsize|useadaptive|fps|usefps|rtpaddr|rtpport|usertp");
	this->Add("move|action|direction");
	this->Add("path|vertices");
	this->Add("exit");
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
AdminCommandParser::AdminCommandParser(void)
{
	this->Add("count");
	this->Add("procs");
	this->Add("stats|id");
	this->Add("add|id|count");
	this->Add("sub|id|count");
	this->Add("set|id|count");
	this->Add("exit");
}
//----------------------------------------------------------------------------------------------
