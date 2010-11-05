//----------------------------------------------------------------------------------------------
//	Filename:	Object.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
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
			Object(std::string p_strObjectID, UniqueIDType p_uidType = Illumina::Core::Transient);

			virtual ~Object();

			inline const UniqueID& GetID(void) const;
			inline const std::string& GetName(void) const;

			virtual size_t GetHashCode(void) const;
			virtual std::string ToString(void) const; 
		};
	} 
}

#include "Object.inl"