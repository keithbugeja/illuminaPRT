//----------------------------------------------------------------------------------------------
//	Filename:	FactoryManager.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <map>
#include <string>
#include <iostream>

#include "Factory.h"

namespace Illumina
{
	namespace Core
	{
		template<class T>
		class FactoryManager
		{
		protected:
			std::map<std::string, Factory<T>*> m_factoryInstanceMap;
			std::map<std::string, T*> m_instanceMap;

		protected:
			bool ContainsFactory(const std::string &p_strFactoryName);
			Factory<T>* FindFactory(const std::string &p_strFactoryName);
			
			bool ContainsItem(const std::string &p_strItemName);
			T* FindItem(const std::string &p_strItemName);

		public:
			void RegisterFactory(const std::string &p_strFactoryName, Factory<T> *p_pFactory);
			void UnregisterFactory(const std::string &p_strFactoryName);
			
			bool QueryFactory(const std::string &p_strFactoryName);
			Factory<T>* RequestFactory(const std::string &p_strFactoryName);

			bool QueryInstance(const std::string &p_strInstanceName);
			void RegisterInstance(const std::string &p_strInstanceName, T *p_pInstance);
			
			T* CreateInstance(const std::string &p_strFactoryName, const std::string &p_strInstanceName);
			T* CreateInstance(const std::string &p_strFactoryName, const std::string &p_strInstanceName, const std::string& p_strArguments);
			T* RequestInstance(const std::string &p_strInstanceName);
			T* ReleaseInstance(const std::string &p_strInstanceName);
		};
	}
}

#include "FactoryManager.inl"