//----------------------------------------------------------------------------------------------
//	Filename:	Lexer.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "System/Lexer.h"
#include "Exception/Exception.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
Lexer::Lexer(std::istream *p_pInputStream)
	: m_pInputStream(p_pInputStream)
{ }
//----------------------------------------------------------------------------------------------
bool Lexer::ReadToken(LexerToken &p_token)
{
	p_token = ExtractToken();
	return (p_token.Type != LexerToken::EndOfStream);
}
//----------------------------------------------------------------------------------------------
bool Lexer::ReadToken(LexerToken::TokenType p_tokenType, LexerToken &p_token)
{
	p_token = ExtractToken();
	return (p_token.Type == p_tokenType);
}
//----------------------------------------------------------------------------------------------
LexerToken Lexer::ExtractToken(void)
{
	LexerToken lexerToken;

	std::string strProperty,
		strValue;

	char ch;

	// Lexer begins in start state
	state = Lexer::Start;

	// Read from stream until it runs out
	while(m_pInputStream->get(ch))
	{
		// Lex according to current state
		switch (state)
		{
			case Lexer::Start:
			{
				if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
					; // ingore whitespace
				else if (ch == '#')
				{
					state = Lexer::Comment;
				}
				else if (ch == '{')
				{
					lexerToken.Type = LexerToken::LeftCurly;
					lexerToken.Value = "{";
					return lexerToken;
				}
				else if (ch == '}')
				{
					lexerToken.Type = LexerToken::RightCurly;
					lexerToken.Value = "}";
					return lexerToken;
				}
				else if (ch == '=')
				{
					state = Lexer::PropertyValue;
					strValue.clear();
				}
				else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_')
				{
					state = Lexer::PropertyName;
					strProperty = ch;							
				}
				break;
			}

			case Lexer::PropertyName:
			{
				if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
				{
					lexerToken.Value = strProperty;
					lexerToken.Type = LexerToken::PropertyName;
								
					return lexerToken;
				}
				else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_')
				{
					strProperty += ch;
				}

				break;
			}

			case Lexer::PropertyValue:
			{
				if (ch == '\r' || ch == '\n')
				{
					boost::trim(strValue);

					lexerToken.Value = strValue;
					lexerToken.Type = LexerToken::PropertyValue;

					return lexerToken;
				}
				else
				{
					strValue += ch;
				}

				break;
			}

			case Lexer::Comment:
			{
				if (ch == '\n')
					state = Lexer::Start;

				break;
			}
		}
	}

	if (state != Lexer::Start)
		throw new Exception("Parsing error : Unexpected end of file!");

	lexerToken.Type = LexerToken::EndOfStream;
	return lexerToken;
}
//----------------------------------------------------------------------------------------------