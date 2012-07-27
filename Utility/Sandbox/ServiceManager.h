class ServiceManager
{
	bool m_bVerbose;

	ServiceManager(bool p_bVerbose);
	~ServiceManager(void);

	void Start(void) 
	{/*
		// Initialise MPI
		int provided;

		MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

		// Get Process Rank
		int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);

		// We need to detect whether this is running as load balancer, coordinator or worker
		if (rank == 0)
		{
			Master(p_bVerbose);
		}
		else 
			Idle(p_bVerbose);

		// Finalise MPI
		MPI_Finalize();
	*/ }
};