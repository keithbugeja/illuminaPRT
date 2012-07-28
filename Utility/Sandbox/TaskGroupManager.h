//----------------------------------------------------------------------------------------------
//	Filename:	TaskGroupManager.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------------------------
#include "TaskGroupController.h"
#include "Logger.h"

using namespace Illumina::Core;

class TaskGroupManager
{
	std::vector<TaskGroupController*> m_taskGroupControllerList;

public:
	void AddController(TaskGroupController *p_pController)
	{
		m_taskGroupControllerList.push_back(p_pController);
	}

	void RemoveController(TaskGroupController *p_pController)
	{
		std::vector<TaskGroupController*>::iterator iterator = std::find(m_taskGroupControllerList.begin(), m_taskGroupControllerList.end(), p_pController);
		m_taskGroupControllerList.erase(iterator);
	}
};