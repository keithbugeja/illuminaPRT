//----------------------------------------------------------------------------------------------
//	Filename:	ResourceManager.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include <mpi.h>

//----------------------------------------------------------------------------------------------
#include "ResourceManager.h"
#include "ServiceManager.h"
#include "Logger.h"
//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
ResourceManager::ResourceManager(void) { }
//----------------------------------------------------------------------------------------------
ResourceManager::~ResourceManager(void) { }
//----------------------------------------------------------------------------------------------
bool ResourceManager::AllocateResources(void)
{
	// Get size of communicator
	MPI_Comm_size(MPI_COMM_WORLD, &m_nResourceCount);

	int resourceID = WhoAmI();

	for (int resourceIdx = 0; resourceIdx < m_nResourceCount; resourceIdx++)
	{
		Resource *pResource = new Resource(resourceIdx, Resource::ST_Idle);		

		// Set resource for THIS PROCESS 
		if (resourceID == resourceIdx)
			m_pMe = pResource;

		// Push resource on directory
		m_resourceIndexList.push_back(pResource);
		m_resourceIndexMap[resourceIdx] = pResource;

		// Only resources of worker type are up for allocation
		if (WhatIs(resourceIdx) == ResourceManager::Worker)
		{
			// Push resource on free list
			m_resourceFreeList.push_back(pResource);
		}
	}

	std::stringstream message;
	message << "Resource Manager allocating " << m_nResourceCount << " resources for use." << std::endl;
	Logger::Message(message.str(), ServiceManager::GetInstance()->IsVerbose());

	return true;
}
//----------------------------------------------------------------------------------------------
void ResourceManager::FreeResources(void)
{
	for (std::vector<Resource*>::iterator resourceIterator = m_resourceIndexList.begin();
		 resourceIterator != m_resourceIndexList.end(); ++resourceIterator)
		delete *resourceIterator;

	m_resourceFreeList.clear();

	m_resourceIndexList.clear();
	m_resourceIndexMap.clear();

	m_resourceAllocationList.clear();
	m_resourceAllocationMap.clear();
}
//----------------------------------------------------------------------------------------------
Resource* ResourceManager::Me(void) 
{
	return m_pMe;
}
//----------------------------------------------------------------------------------------------
ResourceManager::ResourceType ResourceManager::WhatIs(int p_nResourceID)
{
	return (p_nResourceID == 0) 
		? ResourceManager::Master 
		: ResourceManager::Worker;
}
//----------------------------------------------------------------------------------------------
ResourceManager::ResourceType ResourceManager::WhatAmI(void) 
{
	return (WhoAmI() == 0) 
		? ResourceManager::Master 
		: ResourceManager::Worker;
}
//----------------------------------------------------------------------------------------------
int ResourceManager::WhoAmI(void)
{
	int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	return rank;
}
//----------------------------------------------------------------------------------------------
void ResourceManager::Initialise(void)
{
	// Initialise MPI with support for calls from multiple-threads
	int provided; MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &provided);

	AllocateResources();
}
//----------------------------------------------------------------------------------------------
void ResourceManager::Shutdown(void)
{
	FreeResources();

	// Terminate application
	MPI_Finalize();
}
//----------------------------------------------------------------------------------------------
bool ResourceManager::RequestResources(int p_nTaskID, int p_nResourceCount)
{
	// Cannot allocate more resources than available!
	if (p_nResourceCount > m_resourceFreeList.size())
	{
		Logger::Message("ResourceManager::RequestResources : Not enough free resources to satisfy allocation request!", 
			ServiceManager::GetInstance()->IsVerbose(), Logger::Error);

		return false;
	}

	// Check if Task ID is valid.
	IResourceController *pController = GetInstance(p_nTaskID);
	if (pController == NULL)
	{
		Logger::Message("ResourceManager::RequestResources : Unknown Task ID!", 
			ServiceManager::GetInstance()->IsVerbose(), Logger::Error);

		return false;
	}

	// -- REQUIRES MUTEX ACCESS TO DS!
	// Choose resources from free list
	std::vector<Resource*> resourceAllocation;

	for (int resourceIdx = 0; resourceIdx < p_nResourceCount; ++resourceIdx)
	{
		Resource *pResource = m_resourceFreeList.back();
		m_resourceFreeList.pop_back();
		
		// Push onto temporary task allocation list
		resourceAllocation.push_back(pResource);

		// Update resource allocation directory
		m_resourceAllocationList.push_back(pResource);
		m_resourceAllocationMap[pResource->GetID()] = p_nTaskID;
	}
	// -- REQUIRES MUTEX ACCESS TO DS!

	// Trigger notification on controller
	pController->OnResourceAdd(resourceAllocation);

	return true;
}
//----------------------------------------------------------------------------------------------
bool ResourceManager::ReleaseResources(int p_nTaskID, int p_nResourceCount)
{
	// Check if Task ID is valid.
	IResourceController *pController = GetInstance(p_nTaskID);
	if (pController == NULL)
	{
		Logger::Message("ResourceManager::ReleaseResources : Unknown Task ID!", 
			ServiceManager::GetInstance()->IsVerbose(), Logger::Error);

		return false;
	}
	
	// Check if task has the requested number of allocated resources
	if (p_nResourceCount > pController->GetResourceCount())
	{
		Logger::Message("ResourceManager::ReleaseResources : Unable to release the requested number of resources!", 
			ServiceManager::GetInstance()->IsVerbose(), Logger::Error);

		return false;
	}

	// Call controller to populate released resource list
	std::vector<Resource*> resourceList; pController->OnResourceRemove(p_nResourceCount, resourceList);

	// Now remove resources and put them back on free list
	for (std::vector<Resource*>::iterator resourceIterator = resourceList.begin();
		 resourceIterator != resourceList.end(); resourceIterator++)
	{
		Resource *pResource = *resourceIterator;

		// Update resource allocation directory
		std::vector<Resource*>::iterator resourceFindIterator = 
			std::find(m_resourceAllocationList.begin(), m_resourceAllocationList.end(), pResource);

		if (resourceFindIterator != m_resourceAllocationList.end())
		{
			m_resourceAllocationList.erase(resourceFindIterator);
			m_resourceAllocationMap.erase(pResource->GetID());

			// Update free list
			m_resourceFreeList.push_back(pResource);
	
			std::cout << "Resource [" << pResource->GetID() << "] freed." << std::endl;
		}
		else
			std::cerr << "Unable to delete resource [" << pResource->GetID() << "]. Resource not in allocation list!" << std::endl;
	}

	std::cout << "Freed " << resourceList.size() << " resources." << std::endl;

	return true;
}
//----------------------------------------------------------------------------------------------
void ResourceManager::GetControllerInfo(std::vector<ResourceControllerInfo> &p_controllerInfoList)
{
	ResourceControllerInfo info;

	for (std::vector<IResourceController*>::iterator controllerIterator = m_controllerList.begin();
		 controllerIterator != m_controllerList.end(); ++controllerIterator)
	{
		(*controllerIterator)->GetControllerInfo(info);
		p_controllerInfoList.push_back(info);
	}
}
