//----------------------------------------------------------------------------------------------
//	Filename:	SpaceManager.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <map>
#include <string>
#include <iostream>

#include "Space/Space.h"
#include "Space/BasicSpace.h"

namespace Illumina
{
	namespace Core
	{		
		class BasicSpaceFactory : public Illumina::Core::Factory<Illumina::Core::ISpace>
		{
		public:
			Illumina::Core::ISpace *CreateInstance(void)
			{
				return new BasicSpace();
			}

			// Arguments
			// -- Id
			Illumina::Core::ISpace *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strId;

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId);

				return CreateInstance();
			}

			Illumina::Core::ISpace *CreateInstance(const std::string &p_strId)
			{
				return new BasicSpace(p_strId);
			}
		};
	}
}