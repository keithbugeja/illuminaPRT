//----------------------------------------------------------------------------------------------
//	Filename:	Exception.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>

namespace Illumina
{
	namespace Core
	{
		class Exception
		{
		private:
			std::string m_strMessage;
			std::string m_strSourceFile;
			int m_nSourceLine;
			Exception* m_pInnerException;

		public: // public methods
			Exception(const std::string &p_strMessage,
				const std::string &p_strSourceFile,
				int p_nSourceLine,
				const Exception* p_pInnerException);
			Exception(const std::string &p_strMessage,
				const std::string &p_strSourceFile,
				int p_nSourceLine);
			Exception(const std::string &p_strMessage,
				const Exception* p_pInnerException);
			Exception(const std::string &p_strMessage);
			Exception(const char *p_szMessage);
			Exception(void);
			virtual ~Exception(void);

			const std::string& GetMessage(void) const;
			const std::string& GetSourceFile(void) const;
			int GetSourceLine(void) const;
			const Exception* GetInnerException(void) const;
			std::string GetMessageTrace(void) const;
		};
	}
}
