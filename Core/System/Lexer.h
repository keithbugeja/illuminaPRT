#pragma once

#include <iostream>
#include "boost/algorithm/string.hpp"

namespace Illumina
{
	namespace Core
	{
		struct LexerToken
		{
			enum TokenType
			{
				Undefined,
				LeftCurly,
				RightCurly,
				PropertyName,
				PropertyValue,
				EndOfStream
			} Type;

			std::string Value;
		};

		class Lexer
		{
			enum LexerState
			{
				Start,
				Comment,
				PropertyName,
				PropertyValue,
				Error
			} state;

			std::istream m_inputStream;

			LexerToken ExtractToken(void)
			{
				char ch;
				std::string property;
				std::string value;
				LexerToken lexerToken;
				state = Lexer::Start;

				while(m_inputStream.get(ch))
				{
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
							
								property.clear();
							}
							else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_')
							{
								state = Lexer::PropertyName;
							
								value.clear();
								value += ch;
							}
							break;
						}

						case Lexer::PropertyName:
						{
							if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
							{
								lexerToken.Value = property;
								lexerToken.Type = LexerToken::PropertyName;
								
								return lexerToken;
							}
							else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_')
							{
								property += ch;
							}

							break;
						}

						case Lexer::PropertyValue:
						{
							if (ch == '\r' || ch == '\n')
							{
								boost::trim(value);

								lexerToken.Value = value;
								lexerToken.Type = LexerToken::PropertyValue;

								return lexerToken;
							}
							else
							{
								value += ch;
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

					if (state != Lexer::Start)
						std::cout << "Parsing error!" << std::endl;

					lexerToken.Type = LexerToken::EndOfStream;
					return lexerToken;
				}
			}

			public:
			Lexer(std::istream &p_inputStream)
				: m_inputStream(p_inputStream)
			{ }

			LexerToken ReadToken(void)
			{
				return ExtractToken();
			}
		};
	}
}