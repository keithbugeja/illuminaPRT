//----------------------------------------------------------------------------------------------
//	Filename:	Logger.h
//	Author:		Keith Bugeja
//	Date:		18/07/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <map>

//----------------------------------------------------------------------------------------------
/*
 * Log levels 
 */
//----------------------------------------------------------------------------------------------
enum LoggingLevel
{
	LL_Critical = 0x01,
	LL_Error	= 0x02,
	LL_Warning	= 0x04,
	LL_Info		= 0x08,

	LL_CriticalLevel	= LL_Critical,
	LL_ErrorLevel		= LL_CriticalLevel | LL_Error,
	LL_WarningLevel		= LL_ErrorLevel | LL_Warning,
	LL_InfoLevel		= LL_WarningLevel | LL_Info,

	LL_All		= 0x0F
};

//----------------------------------------------------------------------------------------------
/*
 * Logger sink interface
 */
//----------------------------------------------------------------------------------------------
class ILoggerSink
{
public:
	virtual bool IsOpen(void) { return true; }
	virtual bool Open(void) { return true; }
	virtual void Close(void) { };
	virtual void Flush(void) { };

	virtual int Write(const std::string &p_strMessage) = 0;
};

//----------------------------------------------------------------------------------------------
/*
 * Console Logger Sink
 * Produces console output
 */
//----------------------------------------------------------------------------------------------
class ConsoleLoggerSink
	: public ILoggerSink
{
public:
	void Flush(void) {
		std::cout.flush();
	}

	int Write(const std::string &p_strMessage)
	{
		std::cout << p_strMessage << std::endl;
		return p_strMessage.length();
	}
};

//----------------------------------------------------------------------------------------------
/*
 * File Logger Sink
 * Produces file output
 */
//----------------------------------------------------------------------------------------------
class FileLoggerSink
	: public ILoggerSink
{
protected:
	std::fstream m_fileStream;
	std::string m_strFilename;

public:
	FileLoggerSink(const std::string &p_strFilename)
		: m_strFilename(p_strFilename)
	{ }

	bool IsOpen(void) {
		return m_fileStream.is_open();
	}

	bool Open(void) 
	{
		m_fileStream.open(m_strFilename.c_str(), std::fstream::out);
		return m_fileStream.is_open();
	}

	void Close(void) {
		m_fileStream.close();
	}

	void Flush(void) {
		m_fileStream.flush();
	}

	int Write(const std::string &p_strMessage)
	{
		m_fileStream << p_strMessage;
		return p_strMessage.length();
	}
};

//----------------------------------------------------------------------------------------------
/*
 * Asynchronous File Logger Sink
 * Produces file output
 */
//----------------------------------------------------------------------------------------------
class AsynchronousFileSink
	: public ILoggerSink
{
protected:
	std::string m_strFilename;
	std::fstream m_fileStream;

	std::queue<std::string> m_outputQueue;

	boost::thread m_sinkThread;
	boost::condition_variable m_sinkThreadConditionVariable;

	bool m_bFileStreamOpen;

protected:
	static void OutputThreadHandler(AsynchronousFileSink *p_pLoggerSink)
	{
		boost::mutex bufferMutex;
		boost::unique_lock<boost::mutex> bufferLock(bufferMutex);

		while(p_pLoggerSink->m_bFileStreamOpen)
		{
			if (p_pLoggerSink->m_outputQueue.empty())
				p_pLoggerSink->m_sinkThreadConditionVariable.wait(bufferLock);

			while(!p_pLoggerSink->m_outputQueue.empty()) 
			{
				p_pLoggerSink->m_fileStream << p_pLoggerSink->m_outputQueue.front();
				p_pLoggerSink->m_outputQueue.pop();
			}
		}

		p_pLoggerSink->m_fileStream.close();
	}

public:
	AsynchronousFileSink(const std::string &p_strFilename)
		: m_strFilename(p_strFilename)
	{ }

	bool IsOpen(void) {
		return m_fileStream.is_open();
	}

	bool Open(void) 
	{
		// Open file stream
		m_fileStream.open(m_strFilename.c_str(), std::fstream::out);
		
		// On success set open flag, otherwise return error
		if (!(m_bFileStreamOpen = m_fileStream.is_open()))
			return false;

		m_sinkThread = boost::thread(boost::bind(AsynchronousFileSink::OutputThreadHandler, this));

		return false;
	}

	void Close(void) 
	{
		// Signal thread to close
		m_bFileStreamOpen = false;
		m_sinkThreadConditionVariable.notify_all();
	}

	void Flush(void) 
	{
		// Flush file stream
		m_fileStream.flush();
	}

	int Write(const std::string &p_strMessage)
	{
		m_outputQueue.push(p_strMessage);
		m_sinkThreadConditionVariable.notify_all();
		return p_strMessage.length();
	}
};

//----------------------------------------------------------------------------------------------
/*
 * ILoggerChannel
 * 
 */
//----------------------------------------------------------------------------------------------
class ILoggerChannel
{
protected:
	bool m_bTimeStampEnabled;
	int m_nLoggingFilter;

	std::string m_channelName;
	std::vector<ILoggerSink*> m_loggerSinkList;

protected:
	ILoggerChannel(const std::string p_strChannelName)
		: m_nLoggingFilter(LL_All)
		, m_channelName(p_strChannelName)
	{ }

public:
	const std::string& GetName(void) { return m_channelName; }
	
	bool IsTimeStampEnabled(void) { return m_bTimeStampEnabled; }
	void EnableTimeStamp(bool p_bEnable) { m_bTimeStampEnabled = p_bEnable; }

	void SetLoggingFilter(int p_nLoggingFilter) { m_nLoggingFilter = p_nLoggingFilter; }
	int GetLoggingFilter(void) { return m_nLoggingFilter; }

	void AddSink(ILoggerSink *p_pLoggerSink) 
	{
		if (!p_pLoggerSink->IsOpen())
			p_pLoggerSink->Open();

		m_loggerSinkList.push_back(p_pLoggerSink); 
	}

	virtual void Write(const std::string &p_strMessage, LoggingLevel p_filter = LL_All) 
	{
		if (p_filter & m_nLoggingFilter)
		{
			for (std::vector<ILoggerSink*>::iterator sinkIterator = m_loggerSinkList.begin();
				 sinkIterator != m_loggerSinkList.end(); sinkIterator++)
			{
				(*sinkIterator)->Write(p_strMessage);
			}
		}
	}
};

//----------------------------------------------------------------------------------------------
/*
 * LoggerChannel 
 * ILoggerChannel basic implementations
 */
//----------------------------------------------------------------------------------------------
class LoggerChannel
	: public ILoggerChannel
{
public:
	LoggerChannel(const std::string &p_strChannelName)
		: ILoggerChannel(p_strChannelName) 
	{ }
};

//----------------------------------------------------------------------------------------------
/*
 * Logger
 */
//----------------------------------------------------------------------------------------------
class Logger
{
protected:
	std::map<std::string, ILoggerChannel*> m_channelList;
	std::vector<ILoggerChannel*> m_defaultGroup;

	LoggerChannel m_loggerChannel;
	ConsoleLoggerSink m_consoleLoggerSink;

	int m_nLoggingFilter;

public:
	Logger(void)
		: m_loggerChannel("Debug")
		, m_nLoggingFilter(LL_All)
	{
		m_loggerChannel.AddSink(&m_consoleLoggerSink);
		AddChannel("Debug", &m_loggerChannel);
	}

	void AddChannel(const std::string &p_strName, ILoggerChannel *p_pChannel, bool p_bDefaultGroup = true) 
	{
		m_channelList[p_strName] = p_pChannel;
		
		if (p_bDefaultGroup) 
			m_defaultGroup.push_back(p_pChannel);
	}

	ILoggerChannel *operator[](const std::string &p_strChannelName) {
		return m_channelList[p_strChannelName];
	}

	void SetLoggingFilter(int p_nLoggingFilter) { m_nLoggingFilter = p_nLoggingFilter; }
	int GetLoggingFilter(void) { return m_nLoggingFilter; }

	virtual void Write(const std::string &p_strMessage, LoggingLevel p_filter = LL_All) 
	{
		std::vector<ILoggerChannel*>::iterator channelIterator;

		if (p_filter & m_nLoggingFilter)
		{
			for (channelIterator = m_defaultGroup.begin();
				 channelIterator != m_defaultGroup.end(); 
				 channelIterator++)
			{
				(*channelIterator)->Write(p_strMessage, p_filter);
			}
		}
	}
};

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------