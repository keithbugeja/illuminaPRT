//----------------------------------------------------------------------------------------------
//	Filename:	Exception.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Description goes here...
//----------------------------------------------------------------------------------------------
#include <string>
#include <iostream>
#include <boost/format.hpp>
#include "Exception/Exception.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
Exception::Exception(const std::string &p_strMessage,
	const std::string &p_strSourceFile,
	int p_nSourceLine,
	const Exception* p_pInnerException)
	: m_strMessage(p_strMessage)
	, m_strSourceFile(p_strSourceFile)
	, m_nSourceLine(p_nSourceLine)
	, m_pInnerException((Exception*) p_pInnerException)
{ }
//----------------------------------------------------------------------------------------------
Exception::Exception(const std::string &p_strMessage,
	const std::string &p_strSourceFile,
	int p_nSourceLine)
	: m_strMessage(p_strMessage)
	, m_strSourceFile(p_strSourceFile)
	, m_nSourceLine(p_nSourceLine)
	, m_pInnerException(NULL)
{ }
//----------------------------------------------------------------------------------------------
Exception::Exception(const std::string &p_strMessage,
	const Exception* p_pInnerException)
	: m_strMessage(p_strMessage)
	, m_strSourceFile(__FILE__)
	, m_nSourceLine(__LINE__)
	, m_pInnerException((Exception*) p_pInnerException)
{ }
//----------------------------------------------------------------------------------------------
Exception::Exception(const std::string &p_strMessage)
	: m_strMessage(p_strMessage)
	, m_strSourceFile(__FILE__)
	, m_nSourceLine(__LINE__)
	, m_pInnerException(NULL)
{ }
//----------------------------------------------------------------------------------------------
Exception::Exception(const char* p_szMessage)
	: m_strMessage(p_szMessage)
	, m_strSourceFile(__FILE__)
	, m_nSourceLine(__LINE__)
	, m_pInnerException(NULL)
{ }
//----------------------------------------------------------------------------------------------
Exception::Exception(void)
	: m_strMessage("")
{ }
//----------------------------------------------------------------------------------------------
Exception::~Exception(void)
{
	if (m_pInnerException != NULL)
		delete m_pInnerException;
}
//----------------------------------------------------------------------------------------------
const std::string& Exception::GetMessage(void) const {
	return m_strMessage;
}
//----------------------------------------------------------------------------------------------
const std::string& Exception::GetSourceFile(void) const {
	return m_strSourceFile;
}
//----------------------------------------------------------------------------------------------
int Exception::GetSourceLine(void) const {
	return m_nSourceLine;
}
//----------------------------------------------------------------------------------------------
const Exception* Exception::GetInnerException(void) const {
	return m_pInnerException;
}
//----------------------------------------------------------------------------------------------
std::string Exception::GetMessageTrace(void) const
{
	if (m_pInnerException == NULL)
	{
		return boost::str(
			boost::format("%s [Source : %s Line : %d]")
				% m_strMessage % m_strSourceFile % m_nSourceLine);
	}
	else
	{
		return boost::str(
			boost::format("%s [Source : %s Line : %d] Inner Exception : ")
				% m_strMessage % m_strSourceFile % m_nSourceLine
				% m_pInnerException->GetMessageTrace());
	}
}
//----------------------------------------------------------------------------------------------

