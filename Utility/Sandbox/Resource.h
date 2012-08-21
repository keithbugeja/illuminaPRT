//----------------------------------------------------------------------------------------------
//	Filename:	Resource.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once

#include <vector>
//----------------------------------------------------------------------------------------------
#include "TaskPipeline.h"
//----------------------------------------------------------------------------------------------

class Resource
{
public:
	enum State 
	{
		ST_Idle,
		ST_Worker,
		ST_Coordinator
	};

protected:
	int m_nResourceID;
	State m_resourceState;

public:
	Resource(int p_nResourceID, State p_resourceState);
	
	void Start(ITaskPipeline *p_pPipeline);
	void Stop(void);

	int GetID(void) const;

	bool IsIdle(void);
	bool IsWorker(void);
	bool IsCoordinator(void);

	// Commands controller can issue:
public:
	// Supported only when idle
	static void Register(const std::string &p_strArgs, int p_nCoordinatorID, std::vector<Resource*> p_resourceList);
	
	// Supported only when registered to a coordinator
	static void Unregister(int p_nCoordinatorID, std::vector<Resource*> p_resourceList);

	// When idle or assigned
	static void Terminate(std::vector<Resource*> p_resourceList);

	// Supported only when coordinator is active
	static void Send(const std::string &p_strMessage, int p_nCoordinatorID, bool p_bHighPriority = false);
};