//----------------------------------------------------------------------------------------------
//	Filename:	EngineKernel.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <map>

namespace Illumina
{
	namespace Core
	{
		template<class T>
		class Factory
		{
		public:
			virtual T *CreateInstance(void) = 0;
		};

		template<class T>
		class FactoryManager
		{
		protected:
			std::map<std::string, Factory<T>*> m_factoryInstanceMap;
			std::map<std::string, T*> m_instanceMap;

			bool ContainsFactory(const std::string& p_strFactoryName)
			{
				return (m_factoryInstanceMap.find(p_strFactoryName) != m_factoryInstanceMap.end());
			}

			Factory<T>* FindFactory(const std::string& p_strFactoryName)
			{
				if (!ContainsFactory(p_strFactoryName)) 
					return NULL;

				return m_factoryInstanceMap[p_strFactoryName];
			}

			bool ContainsItem(const std::string& p_strItemName)
			{
				return (m_instanceMap.find(p_strItemName) != m_instanceMap.end());
			}

			T* FindItem(const std::string& p_strItemName)
			{
				if (!ContainsItem(p_strItemName))
					return NULL;

				return m_instanceMap[p_strItemName];
			}

		public:
			void RegisterFactory(const std::string& p_strFactoryName, Factory<T>* p_pFactory)
			{
				if (ContainsFactory(p_strFactoryName))
					throw new Exception("Cannot register factory. Another factory is already registered with the same name!");

				m_factoryInstanceMap.insert(std::pair<std::string,Factory<T>*>(p_strFactoryName, p_pFactory));
			}

			void UnregisterFactory(const std::string& p_strFactoryName)
			{
				if (!ContainsFactory(p_strFactoryName))
					throw new Exception("Cannot unregister factory. No factory found!");

				m_factoryInstanceMap.erase(p_strFactoryName);
			}

			bool QueryFactory(const std::string& p_strFactoryName)
			{
				return ContainsFactory(p_strFactoryName);
			}

			Factory<T>* RequestFactory(const std::string& p_strFactoryName)
			{
				return FindFactory(p_strFactoryName);
			}

			T* CreateInstance(const std::string& p_strFactoryName, const std::string& p_strInstanceName)
			{
				if (!ContainsFactory(p_strFactoryName))
					throw new Exception("No factory registered under specified name!");

				if (ContainsItem(p_strInstanceName))
					throw new Exception("Instance name must be unique!");

				Factory<T>* pFactory = FindFactory(p_strFactoryName);
				T* pInstance = pFactory->CreateInstance();
				
				if (pInstance == NULL)
					throw new Exception("Instance creation failed!");

				m_instanceMap.insert(std::pair<std::string,T*>(p_strInstanceName, pInstance));

				return pInstance;
			}

			T* RequestInstance(const std::string& p_strInstanceName)
			{
				if (!ContainsItem(p_strInstanceName))
					throw new Exception("Instance does not exist!");
				
				return FindItem(p_strInstanceName);
			}

			T* ReleaseInstance(const std::string& p_strInstanceName)
			{
				if (!ContainsItem(p_strInstanceName))
					throw new Exception("Instance does not exist!");

				T* pInstance = FindItem(p_strInstanceName);
				
				if (p_strInstanceName != NULL)
					delete pInstance;

				m_instanceMap.erase(p_strInstanceName);
			}
		};

		class Dummy
		{
		public:
			std::string GetId()
			{
				return std::string("I am dummy!");
			}
		};


		class DummyFactory : public Factory<Dummy>
		{
		public:
			Dummy* CreateInstance(void)
			{
				return new Dummy();
			}
		};

		class DummyManager : public FactoryManager<Dummy>
		{
		};

		class EngineKernel
		{
		public:
			DummyManager dummyManager;
		};
	}
}