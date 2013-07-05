//----------------------------------------------------------------------------------------------
//	Filename:	Coordinator.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include <set>
#include <boost/thread/mutex.hpp>
//----------------------------------------------------------------------------------------------
#include <System/ArgumentMap.h>
//----------------------------------------------------------------------------------------------
#include "MessageQueue.h"
//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
class ICoordinator
{
	// Message and release mutex (could look into making these look free)
	boost::mutex m_releaseMutex;
	boost::mutex m_registeredMutex;
	boost::mutex m_messageQueueMutex;

	// List of registered and ready workers
	std::set<int> m_registered;
	std::vector<int> m_ready;

	// Workers pending release / deregister signal
	std::set<int> m_release;

	// Argument map
	std::string m_strArguments;
	ArgumentMap m_argumentMap;
	
	// Task state
	bool m_bIsRunning;

protected:
	bool HandleRegister(ResourceMessage *p_pMessage);
	bool HandleUnregister(ResourceMessage *p_pMessage); 

public:
	// Communication handling
	void ControllerCommunication(ResourceMessageQueue *p_pMessageQueue);
	void WorkerCommunication(ResourceMessageQueue *p_pMessageQueue);

	// Argument map handling
	ArgumentMap* GetArgumentMap(void);

	void SetArguments(const std::string &p_strArguments);
	std::string GetArguments(void) const;

	// Get list of available workers
	std::vector<int>& GetAvailableWorkerList(void);
	std::set<int>& GetRegisteredWorkerList(void);

	// Check task state
	bool IsRunning(void) const;

	// Task lifecycle
	bool Initialise(void);
	void Shutdown(void);
	bool Synchronise(void);
	bool Heartbeat(void);
	bool EvaluateMessageQueue(ResourceMessageQueue *p_pMessageQueue);

	// Computation 
	virtual bool Compute(void);

	// User hooks for init, shutdown, sync and message-in
	virtual bool OnInitialise(void) { return true; }
	virtual void OnShutdown(void) { }
	virtual bool OnSynchronise(void) { return true; }
	virtual bool OnSynchroniseAbort(void) { return true; }
	virtual bool OnHeartbeat(void) { return true; }
	virtual bool OnHeartbeatAbort(void) { return true; }
	virtual bool OnMessageReceived(ResourceMessage *p_pMessage) { return true; }
};
//----------------------------------------------------------------------------------------------