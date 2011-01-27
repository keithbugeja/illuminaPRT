//----------------------------------------------------------------------------------------------
//	Filename:	SceneLoader.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#include "Scene/SceneLoader.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
ISceneLoader::ISceneLoader(EngineKernel *p_pEngineKernel, Environment *p_pEnvironment)
	: m_pEngineKernel(p_pEngineKernel)
	, m_pEnvironment(p_pEnvironment)
{ }
//----------------------------------------------------------------------------------------------
int ISceneLoader::Tokenise(std::string &p_strText, char *p_pSeparators, std::vector<std::string> &p_tokenList)
{
	boost::char_separator<char> separator(p_pSeparators);
	boost::tokenizer<boost::char_separator<char> > tokens(p_strText, separator);

	p_tokenList.clear();

	for (boost::tokenizer<boost::char_separator<char> >::iterator iterator = tokens.begin(); iterator != tokens.end(); ++iterator)
	{
		std::string token = *iterator;
		p_tokenList.push_back(token);
	}

	return p_tokenList.size();
}
//----------------------------------------------------------------------------------------------
