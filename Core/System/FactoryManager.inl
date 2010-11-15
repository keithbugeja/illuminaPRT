//----------------------------------------------------------------------------------------------
//	Filename:	FactoryManager.inl
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
template<class T>
bool FactoryManager<T>::ContainsFactory(const std::string& p_strFactoryName)
{
	return (m_factoryInstanceMap.find(p_strFactoryName) != m_factoryInstanceMap.end());
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
template<class T>
Factory<T>* FactoryManager<T>::FindFactory(const std::string& p_strFactoryName)
{
	if (ContainsFactory(p_strFactoryName)) 
		return m_factoryInstanceMap[p_strFactoryName];

	return NULL;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
template<class T>
bool FactoryManager<T>::ContainsItem(const std::string& p_strItemName)
{
	return (m_instanceMap.find(p_strItemName) != m_instanceMap.end());
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
template<class T>
T* FactoryManager<T>::FindItem(const std::string& p_strItemName)
{
	if (ContainsItem(p_strItemName))
		return m_instanceMap[p_strItemName];

	return NULL;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
template<class T>
void FactoryManager<T>::RegisterFactory(const std::string& p_strFactoryName, Factory<T>* p_pFactory)
{
	std::cout << "Factory Manager registering factory '" << p_strFactoryName << "' ..." << std::endl;

	if (ContainsFactory(p_strFactoryName))
		throw new Exception("Cannot register factory. Another factory is already registered with the same name!");

	m_factoryInstanceMap[p_strFactoryName] = p_pFactory;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
template<class T>
void FactoryManager<T>::UnregisterFactory(const std::string& p_strFactoryName)
{
	std::cout << "Factory Manager unregistering factory '" << p_strFactoryName << "' ..." << std::endl;

	if (ContainsFactory(p_strFactoryName))
		m_factoryInstanceMap.erase(p_strFactoryName);

	throw new Exception("Cannot unregister factory. No factory found!");
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
template<class T>
bool FactoryManager<T>::QueryFactory(const std::string& p_strFactoryName)
{
	return ContainsFactory(p_strFactoryName);
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
template<class T>
Factory<T>* FactoryManager<T>::RequestFactory(const std::string& p_strFactoryName)
{
	return FindFactory(p_strFactoryName);
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
template<class T>
T* FactoryManager<T>::CreateInstance(const std::string& p_strFactoryName, const std::string& p_strInstanceName)
{
	std::cout << "FactoryManager creating instance '" << p_strInstanceName << "' from factory '" << p_strFactoryName << "' ..." << std::endl;

	// Check that factory exists and instance name is unique.
	if (!ContainsFactory(p_strFactoryName)) throw new Exception("No factory registered under specified name!");
	if (ContainsItem(p_strInstanceName)) throw new Exception("Instance name must be unique!");

	// Find factory through which instance creation is required
	Factory<T>* pFactory = FindFactory(p_strFactoryName);

	// Create instance
	T* pInstance = pFactory->CreateInstance();
	
	// If instance creation was unsuccessful, thrown an exception
	if (pInstance == NULL) throw new Exception("Instance creation failed!");

	// Add object to managed instances
	m_instanceMap[p_strInstanceName] = pInstance;

	return pInstance;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
template<class T>
T* FactoryManager<T>::RequestInstance(const std::string& p_strInstanceName)
{
	if (ContainsItem(p_strInstanceName))
		return FindItem(p_strInstanceName);

	throw new Exception("Instance does not exist!");
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
template<class T>
T* FactoryManager<T>::ReleaseInstance(const std::string& p_strInstanceName)
{
	if (ContainsItem(p_strInstanceName))
	{
		T* pInstance = FindItem(p_strInstanceName);
				
		if (p_strInstanceName != NULL)
			delete pInstance;

		m_instanceMap.erase(p_strInstanceName);
	}
	else
		throw new Exception("Instance does not exist!");
}