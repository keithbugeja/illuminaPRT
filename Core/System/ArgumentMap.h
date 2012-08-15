//----------------------------------------------------------------------------------------------
//	Filename:	ArgumentMap.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <map>
#include <string>
#include <sstream>
#include <iostream>

#include <boost/tokenizer.hpp>

#include "System/IlluminaPRT.h"
#include "Exception/Exception.h"
#include "Geometry/Vector3.h"
#include "Geometry/Vector2.h"
#include "Spectrum/Spectrum.h"
#include "Image/RGBPixel.h"

namespace Illumina
{
	namespace Core
	{
		class ArgumentMap
		{
		protected:
			std::map<std::string, std::string> m_argumentMap;

		public:
			ArgumentMap(void) { }

			ArgumentMap(const ArgumentMap &p_argumentMap)
			{
				Initialise(p_argumentMap.m_argumentMap);
			}

			ArgumentMap(const std::map<std::string, std::string> p_argumentMap)
			{
				Initialise(p_argumentMap);
			}

			ArgumentMap(const std::string &p_strArgumentList)
			{
				Initialise(p_strArgumentList);
			}

			void Initialise(const std::map<std::string, std::string> p_argumentMap)
			{
				m_argumentMap = p_argumentMap;
			}

			void Initialise(const std::string &p_strArgumentList)
			{
				//std::cout << "Parsing argument map ..." << std::endl;
				m_argumentMap.clear();

				boost::char_separator<char> separator("=;");
				boost::tokenizer<boost::char_separator<char> > tokens(p_strArgumentList, separator);

				for (boost::tokenizer<boost::char_separator<char> >::iterator iterator = tokens.begin(); iterator != tokens.end(); ++iterator)
				{
					std::string name = *iterator;

					if (++iterator == tokens.end()) 
						throw new Exception("Invalid argument string!");

					std::string value = *iterator;

					// Push into map
					m_argumentMap[name] = value;

					// std::cout << "\t[" << name << " = " << value << "]" << std::endl;
				}
			}

			bool ContainsArgument(const std::string &p_strArgumentName)
			{
				return (m_argumentMap.find(p_strArgumentName) != m_argumentMap.end());
			}
			
			template <class T> 
			bool GetArgument(const std::string &p_strArgumentName, T &p_argumentValue)
			{
				if (ContainsArgument(p_strArgumentName))
				{
					std::stringstream argumentValue(m_argumentMap[p_strArgumentName]);
					if (argumentValue >> p_argumentValue)
						return true;
				}

				return false;
			}

			bool GetArgument(const std::string &p_strArgumentName, std::vector<std::string> &p_argumentValue)
			{
				if (ContainsArgument(p_strArgumentName))
				{
					std::string strArgumentList = m_argumentMap[p_strArgumentName];

					boost::char_separator<char> separator("{,}");
					boost::tokenizer<boost::char_separator<char> > tokens(strArgumentList, separator);

					for (boost::tokenizer<boost::char_separator<char> >::iterator iterator = tokens.begin(); iterator != tokens.end(); ++iterator)
					{
						p_argumentValue.push_back(*iterator);
					}
				}

				return true;
			}

			bool GetArgument(const std::string &p_strArgumentName, Vector2 &p_argumentValue)
			{
				if (ContainsArgument(p_strArgumentName))
				{
					float value[2];
					char separator;

					std::stringstream argumentValue(m_argumentMap[p_strArgumentName]);
					// argument type : {0.0f, 0.0f}
					if(argumentValue>>separator>>value[0]>>separator>>value[1])
					{
						p_argumentValue.Set(value[0], value[1]);
						return true;
					}
				}

				return false;
			}

			bool GetArgument(const std::string &p_strArgumentName, Vector3 &p_argumentValue)
			{
				if (ContainsArgument(p_strArgumentName))
				{
					float value[3];
					char separator;

					std::stringstream argumentValue(m_argumentMap[p_strArgumentName]);
					// argument type : {0.0f, 0.0f, 0.0f}
					if(argumentValue>>separator>>value[0]>>separator>>value[1]>>separator>>value[2])
					{
						p_argumentValue.Set(value[0], value[1], value[2]);
						return true;
					}
				}

				return false;
			}

			bool GetArgument(const std::string &p_strArgumentName, std::vector<Vector3> &p_argumentValue)
			{
				if (ContainsArgument(p_strArgumentName))
				{
					float value[3];
					char separator;
					
					std::stringstream argumentValue(m_argumentMap[p_strArgumentName]);
					// argument type : {{0.0f, 0.0f, 0.0f}, ... , {0.0f, 0.0f, 0.0f}}

					argumentValue>>separator; // {

					do 
					{
						// argument type : {0.0f, 0.0f, 0.0f}
						argumentValue>>separator>>value[0]>>separator>>value[1]>>separator>>value[2]>>separator;
						p_argumentValue.push_back(Vector3(value[0], value[1], value[2]));

						argumentValue>>separator; // , -or- }
					} 
					while (separator == ',');
				}

				return true;
			}

			bool GetArgument(const std::string &p_strArgumentName, RGBPixel &p_argumentValue)
			{
				if (ContainsArgument(p_strArgumentName))
				{
					float value[3];
					char separator;

					std::stringstream argumentValue(m_argumentMap[p_strArgumentName]);
					// argument type : {0.0f, 0.0f, 0.0f}
					if(argumentValue>>separator>>value[0]>>separator>>value[1]>>separator>>value[2])
					{
						p_argumentValue.Set(value[0], value[1], value[2]);
						return true;
					}
				}

				return false;
			}

			bool GetArgument(const std::string &p_strArgumentName, Spectrum &p_argumentValue)
			{
				if (ContainsArgument(p_strArgumentName))
				{
					float value[3];
					char separator;

					std::stringstream argumentValue(m_argumentMap[p_strArgumentName]);
					// argument type : {0.0f, 0.0f, 0.0f}
					if(argumentValue>>separator>>value[0]>>separator>>value[1]>>separator>>value[2])
					{
						p_argumentValue.Set(value);
						return true;
					}
				}

				return false;
			}

			bool GetArgument(const std::string &p_strArgumentName, std::vector<Spectrum> &p_argumentValue)
			{
				if (ContainsArgument(p_strArgumentName))
				{
					float value[3];
					char separator;
					
					std::stringstream argumentValue(m_argumentMap[p_strArgumentName]);
					// argument type : {{0.0f, 0.0f, 0.0f}, ... , {0.0f, 0.0f, 0.0f}}
					argumentValue>>separator; // {
					do 
					{
						// argument type : {0.0f, 0.0f, 0.0f}
						argumentValue>>separator>>value[0]>>separator>>value[1]>>separator>>value[2]>>separator;
						p_argumentValue.push_back(Spectrum(value[0], value[1], value[2]));

						argumentValue>>separator; // , -or- }
					} 
					while (separator == ',');
				}

				return true;
			}

			std::string ToString(void) const
			{
				std::stringstream argumentMap;
				std::map<std::string, std::string>::const_iterator mapIterator;

				for (mapIterator = m_argumentMap.begin(); mapIterator != m_argumentMap.end(); ++mapIterator)
				{
					argumentMap << (*mapIterator).first << '=' << (*mapIterator).second << std::endl;
				}

				return argumentMap.str();
			}
		};
	}
}

		
