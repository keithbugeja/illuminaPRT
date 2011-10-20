#pragma once

#include "boost/mpi.hpp"
namespace mpi = boost::mpi;

// get the maximum number of pe's available

void TestScheduler(void)
{
	mpi::environment* m_pMPIEnvironment = new mpi::environment();
	mpi::communicator* m_pMPICommunicator = new mpi::communicator();

	std::cout << "Initialised peer " << m_pMPICommunicator->rank() << "..." << std::endl;

	m_pMPICommunicator->barrier();
	
	// do shit

	m_pMPICommunicator->barrier();

	// Haven't figured out the proper way out of boost MPI
	m_pMPICommunicator->abort(0);
	m_pMPIEnvironment->abort(0);

	delete m_pMPICommunicator;
	delete m_pMPIEnvironment;
}