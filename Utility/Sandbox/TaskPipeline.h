class ITaskPipeline
{
	void OnRegisterWorker(void){ }
	void OnUnregisterWorker(void){ }

	void RegisterWorker(void){ }
	void UnregisterWorker(void){ }

	void SynchroniseWorkers(void){ }
	void OnSynchroniseWorker(void){ }

	virtual void ExecuteCoordinator(void){ }
	virtual void ExecuteWorker(void){ }

	// Should contain methods for:
	// Coordinator:
	//	Register/Deregister workers
	//  Synchronise frame with workers
	//	Distributed/Gather work from workers
	
	// Worker:
	//	Register/Deregister with coordinator
	//	Synchronise frame with coordinator
	//	Receive/Return work from coordinators
};

class RenderPipeline
	: public ITaskPipeline
{ };