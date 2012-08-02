//----------------------------------------------------------------------------------------------
//	Filename:	ResourceManager.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include <mpi.h>

//----------------------------------------------------------------------------------------------
#include "ResourceManager.h"
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
		Resource *pResource = new Resource(resourceIdx, Resource::Idle);		
		m_resourceList.push_back(pResource);

		if (resourceID == resourceIdx)
			m_pMe = pResource;
	}

	std::cout << "Allocated " << m_nResourceCount << " resources for use." << std::endl;

	return true;
}
//----------------------------------------------------------------------------------------------
void ResourceManager::FreeResources(void)
{
	for (std::vector<Resource*>::iterator resourceIterator = m_resourceList.begin();
		 resourceIterator != m_resourceList.end(); ++resourceIterator)
		delete *resourceIterator;

	m_resourceMap.clear();
	m_resourceList.clear();
	m_resourceAllocationMap.clear();
}
//----------------------------------------------------------------------------------------------
Resource* ResourceManager::Me(void) 
{
	return m_pMe;
}
//----------------------------------------------------------------------------------------------
ResourceManager::ResourceType ResourceManager::WhatAmI(void) 
{
	return (WhoAmI() == 0) ? ResourceManager::Master 
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
