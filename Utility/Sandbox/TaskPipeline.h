class ITaskPipeline
{
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