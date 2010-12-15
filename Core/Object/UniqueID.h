//----------------------------------------------------------------------------------------------
//	Filename:	UniqueID.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//  TODO: 
//		(1) Provide a persistent implementation
//		(2) Clean up uidfactory policies
//		(3) Split into respective files
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>

#include "System/IlluminaPRT.h"
#include "Threading/Atomic.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		enum UniqueIDType 
		{
			Transient,
			Persistent
		};

		class UniqueID
		{
			template<class T>
			friend class UniqueIDFactory;

		protected:
			union 
			{
				unsigned char uc_id[8];
				long long ll_id;
			};
				
			UniqueID(long long p_llID)
				: ll_id(p_llID)
			{ }
	
		public:
			UniqueID(void) 
				: ll_id(0)
			{ }
			
			UniqueID(const UniqueID& p_uid)
				: ll_id(p_uid.ll_id)
			{ }

			UniqueID& operator=(const UniqueID &p_uid)
			{
				ll_id = p_uid.ll_id;
				return *this;
			}

			std::string ToString(void) const {
				return boost::str(boost::format("%02x%02x-%02x%02x-%02x%02x-%02x%02x") 
					% (short)uc_id[7] % (short)uc_id[6] % (short)uc_id[5] % (short)uc_id[4]
					% (short)uc_id[3] % (short)uc_id[2] % (short)uc_id[1] % (short)uc_id[0]
					);
			}
		};

		//----------------------------------------------------------------------------------------------
		template <class Policy>
		class UniqueIDFactory
		{
		public:
			UniqueIDType GetType(void) {
				return Policy::GetType();
			}

			static UniqueID GetUniqueID(void) {
				return UniqueID(Policy::GetUniqueID());
			}
		};

		//----------------------------------------------------------------------------------------------
		class TransientUID
		{
			static volatile long long m_llNextID;

		public:
			static UniqueIDType GetType() { return Illumina::Core::Transient; }

			static long long GetUniqueID(void) 
			{
				#if defined(__ARCHITECTURE_X64__)
					return AtomicInt64::Increment((Int64*)&m_llNextID);
				#else
					return AtomicInt32::Increment((Int32*)(((char*)&m_llNextID) + 4));
				#endif
			}
		};
	} 
}