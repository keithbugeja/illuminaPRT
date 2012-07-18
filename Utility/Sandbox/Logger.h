//----------------------------------------------------------------------------------------------
//	Filename:	Logger.h
//	Author:		Keith Bugeja
//	Date:		18/07/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <iostream>
#include <fstream>

//----------------------------------------------------------------------------------------------
class Logger
{
public:
	enum MessageLevel
	{
		Info,
		Warning,
		Error,
		Critical
	};

	static void Message(const std::string& p_strMessage, bool p_bVerbose, MessageLevel p_eLevel = Info)
	{
		if (p_bVerbose) 
		{
			switch (p_eLevel)
			{
				case Critical:
					std::cerr << "Criticial Error :: " << p_strMessage << std::endl;
					break;

				case Error:
					std::cerr << "Error :: " << p_strMessage << std::endl;
					break;

				case Warning:
					std::cout << "Warning :: " << p_strMessage << std::endl;
					break;

				case Info:
					std::cout << p_strMessage << std::endl;
					break;
			}
		}
	}
};
