//----------------------------------------------------------------------------------------------
//	Filename:	Lexer.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <iostream>
#include "boost/algorithm/string.hpp"

//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		/*
		 * Token for hierarchical property map streams
		 */
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

			LexerToken &operator=(const LexerToken &p_token)
			{
				Type = p_token.Type;
				Value = p_token.Value;

				return *this;
			}
		};

		/*
		 * Lexer for hierarchical property maps
		 */ 
		class Lexer
		{
		protected:
			enum LexerState
			{
				Start,
				Comment,
				PropertyName,
				PropertyValue,
				Error
			} state;

		protected:
			std::istream *m_pInputStream;

		protected:
			LexerToken ExtractToken(void);

		public:
			Lexer(std::istream *p_pInputStream);
			bool ReadToken(LexerToken &p_token);
			bool ReadToken(LexerToken::TokenType p_tokenType, LexerToken &p_token);
		};
	}
}