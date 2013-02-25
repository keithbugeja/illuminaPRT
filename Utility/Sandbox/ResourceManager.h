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
	// Unique ID for controller
	::UniqueID m_uniqueID;

	// Controller directory
	std::vector<IResourceController*> m_controllerList;
	std::map<int, IResourceController*> m_controllerMap;
	
	// Keep list of free resources (can allocate from)
	std::vector<Resource*> m_resourceFreeList;

	// Keep resource directory
	std::vector<Resource*> m_resourceIndexList;
	std::map<int, Resource*> m_resourceIndexMap;
	
	// Resource allocation directory
	std::vector<Resource*> m_resourceAllocationList;
	std::map<int, int> m_resourceAllocationMap;

	// Resource count == |m_resourceIndexList|
	int m_nResourceCount;

	// THIS PROCESS resource structure
	Resource *m_pMe;

public:
	enum ResourceType
	{
		Master,
		Worker
	};

protected:
	bool m_bIsRunning;

	void StartService(void);
	void StopService(void);

	static void ServiceHandler(ResourceManager *p_pResourceManager);

protected:
	bool AllocateResources(void);
	void FreeResources(void);

public:
	ResourceManager(void);
	~ResourceManager(void);

	Resource* Me(void);
	ResourceType WhatAmI(void);
	int WhoAmI(void);
	ResourceType WhatIs(int p_nResourceID);

	template<class T> 
	T* CreateInstance(void)
	{
		int nID = m_uniqueID.GetNext();
		T* pController = new T(nID);

		m_controllerList.push_back(pController);
		m_controllerMap[nID] = pController;

		return pController;
	}

	IResourceController* GetInstance(int p_nResourceControllerID);
	void DestroyInstance(IResourceController *p_pController);

	void Initialise(void);
	void Shutdown(void);

	int GetResourceCount(void) const;
	bool RequestResources(int p_nTaskID, int p_nResourceCount);
	bool ReleaseResources(int p_nTaskID, int p_nResourceCount);

	void OnResourceFree(int p_nResourceID);

	void GetControllerInfo(std::vector<ResourceControllerInfo> &p_controllerInfoList);
};