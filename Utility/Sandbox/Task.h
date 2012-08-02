#include "Resource.h"

class Task
{
protected:
	std::vector<Resource*> m_resourceList;
	std::map<int, Resource*> m_resourceMap;

	int m_nCoordinatorID;
	Resource *m_pCoordinatorResource;

public:
	void SetCoordinator(Resource *p_pResource);

	void Add(Resource *p_pResource)
	{
		m_resourceList.push_back(p_pResource);
		m_resourceMap[p_pResource->GetID()] = p_pResource;
	}

	void Add(const std::vector<Resource*> &p_resourceList)
	{
		for (std::vector<Resource*>::const_iterator resourceIterator = p_resourceList.begin();
			 resourceIterator != p_resourceList.end(); ++resourceIterator)
			 Add(*resourceIterator);
	}

	void Remove(Resource *p_pResource)
	{
		std::vector<Resource*>::iterator resourceIterator = std::find(
			m_resourceList.begin(), m_resourceList.end(), p_pResource);
		
		m_resourceList.erase(resourceIterator);
		m_resourceMap.erase(p_pResource->GetID());
	}

	void RemoveAll(void)
	{
		for (std::vector<Resource*>::iterator resourceIterator = m_resourceList.begin();
			 resourceIterator != m_resourceList.end(); ++resourceIterator)
			 Remove(*resourceIterator);
	}
};