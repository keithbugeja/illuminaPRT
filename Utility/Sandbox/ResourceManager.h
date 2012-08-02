//----------------------------------------------------------------------------------------------
//	Filename:	ResourceManager.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------------------------
#include "Controller.h"
#include "Resource.h"
#include "UniqueId.h"

//----------------------------------------------------------------------------------------------
class ResourceManager
{
protected:
	UniqueID m_uniqueID;

	std::vector<IResourceController*> m_controllerList;
	std::map<int, IResourceController*> m_controllerMap;
	
	std::vector<Resource*> m_resourceList;
	std::map<int, Resource*> m_resourceMap;
	std::map<int, int> m_resourceAllocationMap;

	int m_nResourceCount;

	Resource *m_pMe;

public:
	enum ResourceType
	{
		Master,
		Worker
	};

protected:
	bool AllocateResources(void);
	void FreeResources(void);

public:
	ResourceManager(void);
	~ResourceManager(void);

	Resource* Me(void);
	ResourceType WhatAmI(void);
	int WhoAmI(void);

	template<class T> 
	T* CreateInstance(void)
	{
		int nID = m_uniqueID.GetNext();
		T* pController = new T(nID);

		m_controllerList.push_back(pController);
		m_controllerMap[nID] = pController;

		return pController;
	}

	void DestroyInstance(IResourceController *p_pController)
	{
		std::vector<IResourceController*>::iterator controllerIterator = std::find(m_controllerList.begin(), m_controllerList.end(), p_pController);
		
		if (controllerIterator != m_controllerList.end())
			m_controllerList.erase(controllerIterator);

		m_controllerMap.erase(p_pController->GetID());
	}

	void Initialise(void);
	void Shutdown(void);
};