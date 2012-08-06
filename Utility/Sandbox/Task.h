//----------------------------------------------------------------------------------------------
//	Filename:	ServiceManager.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
//	TODO:
//		Resource unregister : try synchronous and asynchronous versions.
//							: asynchronous version could create problems within task pipeline
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include "Resource.h"
//----------------------------------------------------------------------------------------------
class Task
{
public:
	// Task not assigned a coordinator 
	static const int UndefinedCoordinator = -1;

protected:
	// Task directory
	std::vector<Resource*> m_resourceList;
	std::map<int, Resource*> m_resourceMap;

	// Coordinator ID and Task
	int m_nCoordinatorID;
	Resource *m_pCoordinatorResource;

public:
	Task(void)
		: m_nCoordinatorID(UndefinedCoordinator)
		, m_pCoordinatorResource(NULL)
	{ }


	int Size(void) const {
		return m_resourceList.size();
	}

	
	bool HasCoordinator(void) const {
		return m_pCoordinatorResource != NULL;
	}

	int GetCoordinatorID(void) const {
		return m_nCoordinatorID;
	}

	Resource* GetCoordinator(void) {
		return m_pCoordinatorResource;
	}

	void SetCoordinator(Resource *p_pResource)
	{
		if (p_pResource != NULL)
		{
			m_nCoordinatorID = p_pResource->GetID();
			m_pCoordinatorResource = p_pResource;
		}
		else
		{
			m_nCoordinatorID = UndefinedCoordinator;
			m_pCoordinatorResource = NULL;
		}
	}


	Resource* operator[](int p_nIndex)
	{
		BOOST_ASSERT(p_nIndex >=0 && p_nIndex < m_resourceList.size());
		return m_resourceList[p_nIndex];
	}



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

		if (p_pResource->GetID() == GetCoordinatorID())
			SetCoordinator(NULL);
	}

	void Remove(std::vector<Resource*> &p_resourceList)
	{
		for (std::vector<Resource*>::const_iterator resourceIterator = p_resourceList.begin();
			 resourceIterator != p_resourceList.end(); ++resourceIterator)
			 Remove(*resourceIterator);
	}

	void RemoveAll(void)
	{
		for (std::vector<Resource*>::iterator resourceIterator = m_resourceList.begin();
			 resourceIterator != m_resourceList.end(); ++resourceIterator)
			 Remove(*resourceIterator);
	}
};