//----------------------------------------------------------------------------------------------
//	Filename:	FilterFactories.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <map>
#include <string>
#include <iostream>

#include "Filter/Filter.h"
#include "Filter/BoxFilter.h"
#include "Filter/TentFilter.h"

namespace Illumina
{
	namespace Core
	{
		class BoxFilterFactory : public Illumina::Core::Factory<Illumina::Core::IFilter>
		{
		public:
			Illumina::Core::IFilter *CreateInstance(void)
			{
				return new BoxFilter();
			}

			// Arguments
			// -- Id {String}
			Illumina::Core::IFilter *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strId;

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId);

				return CreateInstance();
			}

			Illumina::Core::IFilter *CreateInstance(const std::string &p_strId)
			{
				return new BoxFilter(p_strId);
			}
		};

		class TentFilterFactory : public Illumina::Core::Factory<Illumina::Core::IFilter>
		{
		public:
			Illumina::Core::IFilter *CreateInstance(void)
			{
				return new TentFilter();
			}

			// Arguments
			// -- Id {String}
			Illumina::Core::IFilter *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strId;

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId);

				return CreateInstance();
			}

			Illumina::Core::IFilter *CreateInstance(const std::string &p_strId)
			{
				return new TentFilter(p_strId);
			}
		};
	}
}