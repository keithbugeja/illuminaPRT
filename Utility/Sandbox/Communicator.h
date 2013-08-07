#pragma once

#include <mpi.h>

class Communicator
{
public:
	typedef MPI_Status Status;
	typedef MPI_Request Request;

	enum TagBase
	{
		// Controller -> Task channel
		Controller_Task				= 0x00000100,

		// Controller <-> Coordinator channels
		Controller_Coordinator		= 0x00000200,
		Coordinator_Controller		= 0x00000400,

		// Coordinator <-> Worker channels
		Coordinator_Worker			= 0x00001000,
		Worker_Coordinator			= 0x00002000,

		// Registration / Unregistration channel
		//Coordinator_Worker_Unreg	= 0x00004000,
		//Worker_Coordinator_Unreg	= 0x00004001,

		Coordinator_Worker_Reg		= 0x00004002,
		//Worker_Coordinator_Reg	= 0x00004003,

		// Synch
		Coordinator_Worker_Sync		= 0x00004004,
		Worker_Coordinator_Sync		= 0x00004005,
		 
		// Synch/Hbeat data
		Coordinator_Worker_SyncData	= 0x00004010,
		Worker_Coordinator_SyncData = 0x00004011,

		// Job
		Coordinator_Worker_Job		= 0x00010000,
		Worker_Coordinator_Job		= 0x00020000,

		// Resource -> Manager channel
		Resource_Manager			= 0x00100000
	};

	static const int Manager_Rank = 0;
	static const int Controller_Rank = 0;
	static const int Source_Any = MPI_ANY_SOURCE;
	static const int Tag_Any	= MPI_ANY_TAG;

public:
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Blocking send/receive
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static bool Send(void *p_buffer, int p_size, int p_rank, int p_tag)
	{
		return MPI_SUCCESS == MPI_Send(p_buffer, p_size, MPI_BYTE, p_rank, p_tag, MPI_COMM_WORLD);
	}

	static bool Receive(void *p_buffer, int p_size, int p_rank, int p_tag)
	{
		return MPI_SUCCESS == MPI_Recv(p_buffer, p_size, MPI_BYTE, p_rank, p_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}

	static bool Receive(void *p_buffer, int p_size, int p_rank, int p_tag, MPI_Status *p_status)
	{
		return MPI_SUCCESS == MPI_Recv(p_buffer, p_size, MPI_BYTE, p_rank, p_tag, MPI_COMM_WORLD, p_status);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Asynchronous send/receive 
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static bool SendAsynchronous(void *p_buffer, int p_size, int p_rank, int p_tag, MPI_Request *p_request, MPI_Status *p_status = MPI_STATUSES_IGNORE)
	{
		MPI_Isend(p_buffer, p_size, MPI_BYTE, p_rank, p_tag, MPI_COMM_WORLD, p_request);
		return IsRequestComplete(p_request, p_status);
	}

	static bool ReceiveAsynchronous(void *p_buffer, int p_size, int p_rank, int p_tag, MPI_Request *p_request, MPI_Status *p_status = MPI_STATUSES_IGNORE)
	{
		MPI_Irecv(p_buffer, p_size, MPI_BYTE, p_rank, p_tag, MPI_COMM_WORLD, p_request);
		return IsRequestComplete(p_request, p_status);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Request cancellation
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static bool CancelRequest(MPI_Request *p_request)
	{
		return MPI_Cancel(p_request);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Asynchronous request handling
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static bool IsRequestComplete(MPI_Request *p_request)
	{
		int flag;

		MPI_Test(p_request, &flag, MPI_STATUSES_IGNORE);

		return (flag != 0);
	}

	static bool IsRequestComplete(MPI_Request *p_request, MPI_Status *p_statusOut)
	{
		int flag; 
				
		MPI_Test(p_request, &flag, p_statusOut);
				
		return (flag != 0);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Variable-sized message handling
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static bool Probe(int p_rank, int p_tag, MPI_Status *p_statusOut)
	{
		return MPI_SUCCESS == MPI_Probe(p_rank, p_tag, MPI_COMM_WORLD, p_statusOut);
	}

	static bool ProbeAsynchronous(int p_rank, int p_tag, MPI_Status *p_statusOut)
	{
		int flag;

		MPI_Iprobe(p_rank, p_tag, MPI_COMM_WORLD, &flag, p_statusOut);
			
		return (flag != 0);
	}

	static int GetSize(MPI_Status *p_statusOut, MPI_Datatype p_datatype = MPI_BYTE)
	{
		int size;

		MPI_Get_count(p_statusOut, p_datatype, &size);

		return size;
	}
};
