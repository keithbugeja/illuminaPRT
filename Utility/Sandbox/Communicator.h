#include <mpi.h>

class Communicator
{
public:
	typedef MPI_Status Status;

	enum TagBase
	{
		Coordinator_Worker		= 0x1000,
		Worker_Coordinator		= 0x2000,

		Controller_Worker		= 0x3000,
		Worker_Controller		= 0x4000,
		Controller_Task			= 0x5000,

		Controller_Coordinator	= 0x6000,
		Coordinator_Controller	= 0x7000
	};

	static const int CoordinatorRank = 0;

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
