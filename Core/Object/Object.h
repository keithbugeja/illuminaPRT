//----------------------------------------------------------------------------------------------
//	Filename:	Object.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//  TODO : Consider adding a generic property table interface to objects.
//		   e.g. Set(string name, <T> value) and <T> Get(name)
//		   possibly use ArgumentMap as a base implementation and let property
//		   hardwiring be applied at derived-class level.
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>
#include <boost/functional/hash.hpp>

#include "Cloneable.h"
#include "UniqueID.h"

namespace Illumina 
{
	namespace Core
	{
		class Object
			: public ICloneable
		{
		protected:
			UniqueID m_uid;
			std::string m_strName;

		private:
			inline void Initialise(UniqueIDType p_uidType = Illumina::Core::Transient);

		public:
			Object(UniqueIDType p_uidType = Illumina::Core::Transient);
			Object(std::string p_strObjectName, UniqueIDType p_uidType = Illumina::Core::Transient);

			virtual ~Object();

			inline const UniqueID& GetID(void) const;
			inline const std::string& GetName(void) const;

			virtual size_t GetHashCode(void) const;
			virtual std::string ToString(void) const; 
		};
	} 
}

#include "Object.inl"