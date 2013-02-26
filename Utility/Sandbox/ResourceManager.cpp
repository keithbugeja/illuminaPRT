//----------------------------------------------------------------------------------------------
//	Filename:	ResourceManager.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include "ResourceManager.h"
#include "ServiceManager.h"
#include "Communicator.h"
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

	if (WhoAmI() == Master)
	{
		std::stringstream message;
		message << "ResourceManager :: Allocating [" << m_nResourceCount << "] resources for use.";
		ServiceManager::GetInstance()->GetLogger()->Write(message.str(), LL_Info);
	}

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
void ResourceManager::StartService(void)
{
	Logger *logger = ServiceManager::GetInstance()->GetLogger();

	// Install service handler only if Master
	if (WhoAmI() == ResourceManager::Master)
	{
		m_bIsRunning = true;

		boost::thread serviceHandler = 
			boost::thread(boost::bind(ResourceManager::ServiceHandler, this));
	
		logger->Write("ResourceManager :: Service handler started.", LL_Info);
	}
}
//----------------------------------------------------------------------------------------------
void ResourceManager::StopService(void)
{
	Logger *logger = ServiceManager::GetInstance()->GetLogger();

	// Remove service handler
	if (WhoAmI() == ResourceManager::Master)
	{
		m_bIsRunning = false;

		logger->Write("ResourceManager :: Server handler stopped.", LL_Error);
	}
}
//----------------------------------------------------------------------------------------------
void ResourceManager::Initialise(void)
{
	// Initialise MPI with support for calls from multiple-threads
	int provided; MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &provided);

	// Allocate required resources
	AllocateResources();

	// Start asynch service
	StartService();
}
//----------------------------------------------------------------------------------------------
void ResourceManager::Shutdown(void)
{
	// Stop service handler
	StopService();

	// Free allocated resources
	FreeResources();

	// Terminate application
	MPI_Finalize();
}
//----------------------------------------------------------------------------------------------
void ResourceManager::ServiceHandler(ResourceManager *p_pResourceManager)
{
	Logger *logger = ServiceManager::GetInstance()->GetLogger();
	std::stringstream message;

	int *pCommandBuffer = new int[1024];
	Communicator::Status status;
	
	/* IsRunning() */
	// Why not synchronous send/receive??
	while(p_pResourceManager->m_bIsRunning)
	{
		// while (Communicator::ProbeAsynchronous(Communicator::Source_Any, Communicator::Resource_Manager, &status))
		while(Communicator::Probe(Communicator::Source_Any, Communicator::Resource_Manager, &status))
		{
			Communicator::Receive(pCommandBuffer, Communicator::GetSize(&status), Communicator::Source_Any, Communicator::Resource_Manager, &status);

			switch(*pCommandBuffer)
			{
				case MessageIdentifiers::ID_Resource_Idle:
				{
					p_pResourceManager->OnResourceFree(status.MPI_SOURCE);
					break;
				}

				default:
				{
					message.str(std::string()); message << "ResourceManager :: Received unhandled message [" << *pCommandBuffer << "]";
					logger->Write(message.str(), LL_Error);
				}
			}
		}

		boost::this_thread::sleep(boost::posix_time::microsec(250));
	}
}
//----------------------------------------------------------------------------------------------
int ResourceManager::GetResourceCount(void) const {
	return m_nResourceCount;
}
//----------------------------------------------------------------------------------------------
// Resource requests / releases implement coarse grained atomic semantics - could use some work
//
bool ResourceManager::RequestResources(int p_nTaskID, int p_nResourceCount)
{
	Logger *logger = ServiceManager::GetInstance()->GetLogger();
	std::stringstream message;

	// Check if Task ID is valid.
	IResourceController *pController = GetInstance(p_nTaskID);
	if (pController == NULL)
	{
		message.str(std::string()); message << "ResourceManager :: Unable to allocate resources: Task Id [" << p_nTaskID << "] unknown!";
		logger->Write(message.str(), LL_Error);
		return false;
	}

	/*
	 * Begin critical section
	 */
	m_resourceMutex.lock();

	// Cannot allocate more resources than available!
	if ((unsigned int)p_nResourceCount > m_resourceFreeList.size())
	{
		/*
		 * End critical section
		 */
		m_resourceMutex.unlock();

		message.str(std::string()); message << "ResourceManager :: Unable to allocate resources: Not enough free resources [" << m_resourceFreeList.size() << "] to satisfy allocation request [" << p_nResourceCount << "]!";
		logger->Write(message.str(), LL_Error);
		return false;
	}

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
	
	/*
	 * End critical section
	 */
	m_resourceMutex.unlock();

	// Trigger notification on controller
	pController->OnResourceAdd(resourceAllocation);

	return true;
}
//----------------------------------------------------------------------------------------------
bool ResourceManager::ReleaseResources(int p_nTaskID, int p_nResourceCount)
{
	std::stringstream message;
	Logger *logger = ServiceManager::GetInstance()->GetLogger();

	// Check if Task ID is valid.
	IResourceController *pController = GetInstance(p_nTaskID);
	if (pController == NULL)
	{
		message.str(std::string()); message << "ResourceManager :: Unable to release resources: Task Id [" << p_nTaskID << "] unknown!";
		logger->Write(message.str(), LL_Error);
		return false;
	}
	
	// Check if task has the requested number of allocated resources
	if (p_nResourceCount > pController->GetResourceCount())
	{
		message.str(std::string()); message << "ResourceManager :: Unable to release resources: Request [" << p_nResourceCount << "] exceeds task resources [" << pController->GetResourceCount() << "]!";
		logger->Write(message.str(), LL_Error);
		return false;
	}

	// Call controller to populate released resource list
	std::vector<Resource*> resourceList; pController->OnResourceRemove(p_nResourceCount, resourceList);

	/*
	 * Begin critical section
	 */
	m_resourceMutex.lock();

	// Now remove resources and schedule them to go back on free list
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

			/* 
			 * Free operation is completed asynchronously by resource, which sends
			 * a resource_idle message to the resource manager.
			 */
		}
		else
		{
			message.str(std::string()); message << "ResourceManager :: Unable to move resource [" << pResource->GetID() << "] to the free list: Resource not on allocation list.";
			logger->Write(message.str(), LL_Error);
		}
	}

	/*
	 * End critical section
	 */
	m_resourceMutex.unlock();

	message.str(std::string()); message << "ResourceManager :: Issued free to [" << resourceList.size() << "] resources.";
	logger->Write(message.str(), LL_Info);

	return true;
}
//----------------------------------------------------------------------------------------------
void ResourceManager::OnResourceFree(int p_nResourceID)
{
	std::stringstream messageLog;
	Logger *logger = ServiceManager::GetInstance()->GetLogger();
	logger->Write("ResourceManager :: Handling event [OnResourceFree].", LL_Info);

	/*
	 * Begin critical section
	 */
	m_resourceMutex.lock();

	std::map<int, Resource*>::iterator resourceIterator = m_resourceIndexMap.find(p_nResourceID);

	if (resourceIterator != m_resourceIndexMap.end())
	{
		m_resourceFreeList.push_back(resourceIterator->second);
		
		/*
		 * End critical section
		 */
		m_resourceMutex.unlock();

		std::stringstream message;
		message.str(std::string()); message << "ResourceManager :: Moved resource [" << p_nResourceID << "] to free list.";
		logger->Write(message.str(), LL_Info);
	}
	else
	{
		/*
		 * End critical section
		 */
		m_resourceMutex.unlock();

		std::stringstream message;
		message.str(std::string()); message << "ResourceManager :: Cannot find resource [" << p_nResourceID << "]! Unable to move to free list.";
		logger->Write(message.str(), LL_Error);
	}
}
//----------------------------------------------------------------------------------------------
IResourceController* ResourceManager::GetInstance(int p_nResourceControllerID)
{
	std::map<int, IResourceController*>::iterator controllerIterator 
		= m_controllerMap.find(p_nResourceControllerID);

	if (controllerIterator == m_controllerMap.end())
		return NULL;

	return (*controllerIterator).second;
}
//----------------------------------------------------------------------------------------------
void ResourceManager::DestroyInstance(IResourceController *p_pController)
{
	std::vector<IResourceController*>::iterator controllerIterator = std::find(m_controllerList.begin(), m_controllerList.end(), p_pController);
		
	if (controllerIterator != m_controllerList.end())
		m_controllerList.erase(controllerIterator);

	m_controllerMap.erase(p_pController->GetID());
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
//----------------------------------------------------------------------------------------------
