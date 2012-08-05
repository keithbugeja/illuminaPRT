//----------------------------------------------------------------------------------------------
//	Filename:	Coordinator.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include <System/ArgumentMap.h>
//----------------------------------------------------------------------------------------------
#include "MessageQueue.h"
//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
class ICoordinator
{
	std::vector<int> m_registered;
	std::vector<int> m_ready;

	std::string m_strArguments;
	ArgumentMap m_argumentMap;
	bool m_bIsRunning;

protected:
	bool HandleRegister(ResourceMessage *p_pMessage);
	bool HandleUnregister(ResourceMessage *p_pMessage); 

public:
	void ControllerCommunication(ResourceMessageQueue *p_pMessageQueue);
	void WorkerCommunication(ResourceMessageQueue *p_pMessageQueue);

	void SetArguments(const std::string &p_strArguments);
	std::string GetArguments(void) const;

	bool IsRunning(void) const;

	bool Initialise(void);
	void Shutdown(void);
	bool Synchronise(void);
	bool EvaluateMessageQueue(ResourceMessageQueue *p_pMessageQueue);

	virtual bool Compute(void);

	// User hooks for init, shutdown, sync and message-in
	virtual bool OnInitialise(void) { return true; }
	virtual void OnShutdown(void) { }
	virtual bool OnSynchronise(void) { return true; }
	virtual bool OnMessageReceived(ResourceMessage *p_pMessage) { return true; }
};
//----------------------------------------------------------------------------------------------