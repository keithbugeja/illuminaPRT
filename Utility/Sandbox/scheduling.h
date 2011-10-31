#pragma once

#include "../../Core/Object/Object.h"

#include "boost/mpi.hpp"
namespace mpi = boost::mpi;

// get the maximum number of pe's available

// Start with the temp scheduling staff before moving them into their proper places
// Start by defining a resource, and a resource pool
typedef int ResourceHandle;
typedef int ResourceGroup;



class IResource :
	public Object
{
public:
	enum State
	{
		Idle,
		Busy,
		Blocked,
		Unused
	};

private:
	State m_resourceState;

public:
	State GetState(void) const { return m_resourceState; }
	virtual ResourceHandle GetHandle(void) const = 0;
};



class ComputeResource :
	public IResource
{
private:
	ResourceGroup m_resourceGroup;

public:
	ResourceGroup GetGroup(void) const { return m_resourceGroup; }
};



class MPIComputeResource
	: public ComputeResource
{
private:
	mpi::communicator* m_pMPICommunicator;

public:
	MPIComputeResource(mpi::communicator* p_pMPICommunicator)
	{
		m_pMPICommunicator = p_pMPICommunicator;
	}

	ResourceHandle GetHandle(void) const
	{
		return m_pMPICommunicator->rank();	
	}
};

void TestScheduler(int argc, char **argv)
{
	std::cout << "LIBA" << std::endl;

	MPI_Init(&argc, &argv);

	int rank;

	// master / spawn
	if (argc == 1)
	{
		int slaveCount = 4,
			ierr;

		char *aargv[] = {"Just to say something", "2"};

		MPI_Comm slaveComm;
		MPI_Comm_spawn("Sandbox.exe", aargv, slaveCount, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &slaveComm, &ierr);
	}

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0)
		std::cout << "Me is chief" << std::endl;
	else
		std::cout << "Me is " << rank << std::endl;

	MPI_Finalize();

	//mpi::environment* m_pMPIEnvironment = new mpi::environment();
	//mpi::communicator* m_pMPICommunicator = new mpi::communicator();

	// Haven't figured out the proper way out of boost MPI
	//m_pMPICommunicator->abort(0);
	//m_pMPIEnvironment->abort(0);

	//delete m_pMPICommunicator;
	//delete m_pMPIEnvironment;

}