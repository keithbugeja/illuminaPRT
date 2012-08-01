//----------------------------------------------------------------------------------------------
//	Filename:	ResourceManager.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------------------------
#include "Logger.h"

#include "TaskGroup.h"
#include "Task.h"

using namespace Illumina::Core;

class ResourceManager
{
protected:
	TaskGroup m_resourcePool;



public:
	enum ResourceType
	{
		Resource,
		Master
	};

public:
	ResourceManager(void);
	~ResourceManager(void);

	ResourceType WhatAmI(void) 
	{
		return Master;
		// int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);
		// return (rank == 0) ? Master : Resource;
	}

	void Initialise(void)
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

	void Shutdown(void)
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
};