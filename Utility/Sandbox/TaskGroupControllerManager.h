//----------------------------------------------------------------------------------------------
//	Filename:	TaskGroupControllerManager.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------------------------
#include "TaskGroupController.h"
#include "UniqueID.h"
#include "Logger.h"

using namespace Illumina::Core;

/*
class TaskGroupControllerManager
{
	UniqueID m_uniqueID;

	std::vector<TaskController*> m_controllerList;
	std::map<int, TaskController*> m_controllerMap;

public:
	TaskController *CreateInstance(void)
	{
		int nextID = m_uniqueID.GetNext();

		TaskGroupController *pController = 
			new TaskGroupController(nextID);
		
		m_controllerList.push_back(pController);
		m_controllerMap[nextID] = pController;

		return pController;
	}

	void DestroyInstance(TaskGroupController *p_pController)
	{
		std::vector<TaskGroupController*>::iterator iterator = 
			std::find(m_controllerList.begin(), m_controllerList.end(), p_pController);
		m_controllerList.erase(iterator);

		m_controllerMap.erase(p_pController->GetId());

		delete p_pController;
	}
};
*/