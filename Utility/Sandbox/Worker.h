//----------------------------------------------------------------------------------------------
//	Filename:	Worker.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include <System/ArgumentMap.h>
//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
class IWorker
{
	ArgumentMap m_argumentMap;

	int m_nCoordinatorID;
	volatile bool m_bIsRunning;

public:
	ArgumentMap* GetArgumentMap(void);
	
	void SetCoordinatorID(int p_nCoordinatorID);
	int GetCoordinatorID(void) const;

	bool IsRunning(void) const;

	bool Initialise(void);
	void Shutdown(void);

	virtual bool Register(void);
	virtual bool CoordinatorMessages(void);
	virtual bool Synchronise(void);
	virtual bool Heartbeat(void);

	virtual bool Compute(void);

	// User handlers for init, shutdown and sync events

	virtual bool OnCoordinatorMessages(void *p_pMessage) { return true; }
	virtual bool OnInitialise(void) { return true; }
	virtual void OnShutdown(void) { }
	virtual bool OnSynchronise(void) { return true; }
	virtual bool OnHeartbeat(void) { return true; }
};
//----------------------------------------------------------------------------------------------