//----------------------------------------------------------------------------------------------
//	Filename:	ResourceManager.cpp
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#include "ResourceManager.h"
#include "Logger.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
ResourceManager::ResourceManager(void) { }
//----------------------------------------------------------------------------------------------
ResourceManager::~ResourceManager(void) { }
//----------------------------------------------------------------------------------------------
ResourceManager::ResourceType ResourceManager::WhatAmI(void) 
{
	return ResourceManager::Master;
	// int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	// return (rank == 0) ? Master : Resource;
}
//----------------------------------------------------------------------------------------------
void ResourceManager::Initialise(void)
{
	// Initialise MPI with support for calls from multiple-threads
	/*
	int provided; MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

	// Keep track of tasks, if master
	if (WhatAmI() == Master)
	{
		for (int taskId = 0; taskId < resourceCount; taskId++)
		{
			Task *pTask = new Task();
			pTask->SetRank(taskId);

			m_resourcePool.AddTask(pTask);
		}
	}
	*/
}
//----------------------------------------------------------------------------------------------
void ResourceManager::Shutdown(void)
{
	/*
	// Release tasks if master
	if (WhatAmI() == Master)
	{
	}

	// Terminate application
	MPI_Finalize();
	*/
}
//----------------------------------------------------------------------------------------------
