#pragma once

#include <vector>
#include "taskgroup.h"
#include "taskpipeline.h"
#include "renderpipeline.h"

using namespace Illumina::Core;

void Master(Environment *p_environment, bool p_bVerbose)
{
	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Initialise Master
	//////////////////////////////////////////////////////////////////////////////////////////////////

	// Create two task groups
	// 1. Master : holds all the PEs including the master PE
	// 2. Idle : holds currently idle PEs
	TaskGroup *masterGroup = new TaskGroup(0, 0, 0),
		*idleGroup = new TaskGroup(1, 0);

	// Keeps track of the various groups the system PEs have
	// been partitioned in.
	TaskGroupList taskGroupList;

	// Get size of global communicator
	int size; MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	// Initialise all tasks within communicator
	for (int index = 0; index < size; ++index)
	{
		Task *task = new Task(index, 0, -1);
		masterGroup->TaskList.push_back(task);
	}

	// Get master task
	Task *masterTask = masterGroup->GetMasterTask();

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// DEBUG OUTPUT

	BOOST_ASSERT(masterTask != NULL);
	std::cout << "[" << masterTask->GetWorkerRank() << "] :: Master started." << std::endl;

	// Create idle subgroup
	masterGroup->CreateSubGroup(idleGroup, 1, masterGroup->Size() - 1);
	std::cout << "[" << masterTask->GetWorkerRank() << "] :: Master created idle group of size [" << idleGroup->Size() << "]." << std::endl;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Initialise Master
	//////////////////////////////////////////////////////////////////////////////////////////////////

	// Control communicator
	MasterCommunicator masterCommunicator(masterTask);

	// Group Id counter
	int groupIDSource = 2;

	// Buffers for asynchronous receive
	MPI_Request receiveRequest;
	MPI_Status receiveStatus;
	Message receiveMessage;	

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Start master message-loop
	//////////////////////////////////////////////////////////////////////////////////////////////////
	bool satisfiedRequest = true;
	bool displayInfo = false;
	int requestSize = 6;

	while(true)
	{
		// Set up an asynchronous receive on ChannelMasterStatic
		if (!masterCommunicator.ReceiveAsynchronous(receiveMessage, MPI_ANY_SOURCE, &receiveRequest, &receiveStatus))
		{
			// Loop until a receive buffer contains a new message
			while(!masterCommunicator.IsRequestComplete(&receiveRequest, &receiveStatus))
			{
				//boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

				/*
				// Generate request
				if (satisfiedRequest) 
				{
					requestSize = rand() % 2 + 6;
					std::cout << "[" << masterTask->GetRank() << "] :: Master received request size of [" << requestSize << "]." << std::endl;
				}
				else 
				{
					if (requestSize > idleGroup->Size()) 
					{
						if (!displayInfo)
						{
							displayInfo = true;
							std::cout << "[" << masterTask->GetRank() << "] :: Task group list dump : " << std::endl << 
								taskGroupList.ToString() << std::endl <<
								"Idle group size [" << idleGroup->Size() << "]" << std::endl;
						}

						continue;
					}
					else
						displayInfo = false;
				}
				*/
				
				// Check if we can handle a request of the specified size
				if (requestSize > idleGroup->Size())
				{
/*					std::cout << "[" << masterTask->GetWorkerRank() << "] :: Master cannot satisfy request!" << std::endl;
					
					TerminateMessage terminateMessage;
					ReleaseMessage releaseMessage(5);

					if (taskGroupList.Size() > 0)
					{
						// Choose group to terminate
						int terminateIndex = rand() % taskGroupList.Size();
						TaskGroup *terminateGroup = taskGroupList.GetTaskGroupByIndex(terminateIndex);

						// Send termination / release message
						masterCommunicator.Send(releaseMessage, terminateGroup->GetCoordinatorRank());

						std::cout << "[" << masterTask->GetWorkerRank() << "] :: Master sent [RELEASE] to coordinator [" << 
							terminateGroup->GetCoordinatorRank() << "] for [" << 
							releaseMessage.GetReleaseCount() << "] units." << std::endl;
					}

*/
					satisfiedRequest = false;
					boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
				}
				else
				{
					// If request can be satisifed, split idle group
					TaskGroup *taskGroup = new TaskGroup(groupIDSource++);
					taskGroupList.AddTaskGroup(taskGroup->GetId(), taskGroup);
					idleGroup->Split(taskGroup, 0, requestSize - 1);

					// Set group coordinator
					taskGroup->SetCoordinatorRank(taskGroup->TaskList[0]->GetRank());

					// Now we must inform PEs that they have been assigned to a new task group
					RequestMessage requestMessage(taskGroup->GetId(), 
						taskGroup->GetCoordinatorRank(), 
						taskGroup->Size());

					taskGroup->Broadcast(masterTask, requestMessage, MM_ChannelMasterStatic);
					
					std::cout << "[" << masterTask->GetRank() << "] :: Master created new task group with Id [" << taskGroup->GetId() << "]." << std::endl;

					satisfiedRequest = true;
					boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////
		// Handle async received messages
		//////////////////////////////////////////////////////////////////////////////////////////////////
		switch(receiveMessage.Id)
		{
			case MT_Completed:
			{
				CompletedMessage *completedMessage = (CompletedMessage*)&receiveMessage;

				int releaseIndex = receiveStatus.MPI_SOURCE,
					groupId = completedMessage->GetGroupId();
			
				std::cout << "[" << masterTask->GetRank() << "] :: Master received [COMPLETED] from worker [" << releaseIndex << 
					"] of group [" << groupId << "]." << std::endl;

				TaskGroup *taskGroup = taskGroupList.GetTaskGroupById(groupId);

				if (taskGroup != NULL)
				{
					Task *task = taskGroup->FindTask(releaseIndex);
					
					if (task != NULL)
					{
						idleGroup->Merge(task);
						taskGroup->Remove(task);

						if (taskGroup->Size() == 0)
						{
							std::cout << "[" << masterTask->GetRank() << "] :: Master is disposing of group [" << groupId << "]." << std::endl;
							taskGroupList.RemoveTaskGroup(taskGroup->GetId());
						}
					}
				}

				break;
			}
		}
	}
}


void Idle(Environment *p_environment, bool p_bVerbose)
{
	// Determine task rank
	int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// Declare master and idle tasks
	Task *masterTask = new Task(),
		*idleTask = new Task();

	// Initialise master and idle tasks
	masterTask->SetMasterRank(0);
	masterTask->SetCoordinatorRank(-1);
	masterTask->SetWorkerRank(0);

	idleTask->SetMasterRank(0);
	idleTask->SetCoordinatorRank(-1);
	idleTask->SetWorkerRank(rank);

	std::cout << "[" << idleTask->GetWorkerRank() << "] :: Idle task started." << std::endl;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	MasterCommunicator masterCommunicator(idleTask);
	
	MPI_Request receiveRequest;
	MPI_Status receiveStatus;
	
	Message receiveMessage;

	while(true)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));
		
		// Set up receive from Master on ChannelMasterStatic
		if (!masterCommunicator.ReceiveAsynchronous(receiveMessage, idleTask->GetMasterRank(), &receiveRequest, &receiveStatus))
		{
			std::cout << "[" << idleTask->GetRank() << "] :: Idle task awaiting assignment." << std::endl;

			while(!masterCommunicator.IsRequestComplete(&receiveRequest, &receiveStatus))
			{
				boost::this_thread::sleep(boost::posix_time::milliseconds(250));
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////
		// Handle received message
		//////////////////////////////////////////////////////////////////////////////////////////////////	
		switch (receiveMessage.Id)
		{
			case MT_Request:
			{
				RequestMessage *requestMessage = (RequestMessage*)&receiveMessage;

				std::cout << "[" << idleTask->GetRank() << "] :: Idle task received [REQUEST] for " 					
					<< "group [" << requestMessage->GetGroupId() << "], "
					<< "coordinator [" << requestMessage->GetCoordinatorId() << "], " 
					<< "worker count [" << requestMessage->GetWorkerCount() << "]."
					<< std::endl;

				int groupId = requestMessage->GetGroupId();

				// Set coordinator for idle task
				idleTask->SetCoordinatorRank(requestMessage->GetCoordinatorId());

				// If this task is the coordinator, spawn coordinator code
				if (idleTask->GetCoordinatorRank() == idleTask->GetWorkerRank())
				{
					std::cout << "[" << idleTask->GetRank() << "] :: Idle task changing to communicator for group [" << requestMessage->GetGroupId() << "]." << std::endl;

					//ITaskPipeline pipeline;
					RenderPipeline pipeline(p_environment, p_bVerbose);

					// Might have to revise constructor for coordinator!!!
					ITaskPipeline::CoordinatorTask coordinator;
					coordinator.task = idleTask;
					coordinator.workerCount = 0;
					coordinator.group.SetMasterRank(idleTask->GetMasterRank());
					coordinator.group.SetCoordinatorRank(idleTask->GetWorkerRank());

					pipeline.Coordinator(coordinator);

					std::cout << "[" << idleTask->GetRank() << "] :: Coordinator changing back to idle task for group [" << requestMessage->GetGroupId() << "]." << std::endl;

					// We need to tell master that we are ready
					CompletedMessage completedMessage(groupId);
					masterCommunicator.SendToMaster(completedMessage);
				}
				else
				{
					std::cout << "[" << idleTask->GetRank() << "] :: Idle task changing to worker for group [" << requestMessage->GetGroupId() << "]." << std::endl;

					//ITaskPipeline pipeline;
					RenderPipeline pipeline(p_environment, p_bVerbose);

					pipeline.Worker(idleTask);

					std::cout << "[" << idleTask->GetRank() << "] :: Worker changing back to idle task for group [" << requestMessage->GetGroupId() << "]." << std::endl;

					// We need to tell master that we are ready
					CompletedMessage completedMessage(groupId);
					masterCommunicator.SendToMaster(completedMessage);
				}
				break;
			}

			default:
				break;
		}
	}
}

void InitialiseIllumina(Environment **p_environment, bool p_bVerbose)
{
	//----------------------------------------------------------------------------------------------
	// Engine Kernel
	//----------------------------------------------------------------------------------------------
	MessageOut("\nInitialising EngineKernel...", p_bVerbose);
	EngineKernel *engineKernel = new EngineKernel();
	// Initialise factories -- note, factories should be moved to plug-ins a dynamically loaded

	//----------------------------------------------------------------------------------------------
	// Sampler
	//----------------------------------------------------------------------------------------------
	MessageOut("Registering Samplers...", p_bVerbose);
	engineKernel->GetSamplerManager()->RegisterFactory("Random", new RandomSamplerFactory());
	engineKernel->GetSamplerManager()->RegisterFactory("Jitter", new JitterSamplerFactory());
	engineKernel->GetSamplerManager()->RegisterFactory("Multijitter", new MultijitterSamplerFactory());

	//----------------------------------------------------------------------------------------------
	// Filter
	//----------------------------------------------------------------------------------------------
	MessageOut("Registering Filters...", p_bVerbose);
	engineKernel->GetFilterManager()->RegisterFactory("Box", new BoxFilterFactory());
	engineKernel->GetFilterManager()->RegisterFactory("Tent", new TentFilterFactory());

	//----------------------------------------------------------------------------------------------
	// Space
	//----------------------------------------------------------------------------------------------
	MessageOut("Registering Spaces...", p_bVerbose);
	engineKernel->GetSpaceManager()->RegisterFactory("Basic", new BasicSpaceFactory());

	//----------------------------------------------------------------------------------------------
	// Integrator
	//----------------------------------------------------------------------------------------------
	MessageOut("Registering Integrators...", p_bVerbose);
	engineKernel->GetIntegratorManager()->RegisterFactory("PathTracing", new PathIntegratorFactory());
	engineKernel->GetIntegratorManager()->RegisterFactory("IGI", new IGIIntegratorFactory());
	engineKernel->GetIntegratorManager()->RegisterFactory("Photon", new PhotonIntegratorFactory());
	engineKernel->GetIntegratorManager()->RegisterFactory("Whitted", new WhittedIntegratorFactory());
	engineKernel->GetIntegratorManager()->RegisterFactory("Test", new TestIntegratorFactory());

	//----------------------------------------------------------------------------------------------
	// Renderer
	//----------------------------------------------------------------------------------------------
	MessageOut("Registering Renderers...", p_bVerbose);
	engineKernel->GetRendererManager()->RegisterFactory("Basic", new BasicRendererFactory());
	engineKernel->GetRendererManager()->RegisterFactory("Multithreaded", new MultithreadedRendererFactory());
	engineKernel->GetRendererManager()->RegisterFactory("Distributed", new DistributedRendererFactory());

	//----------------------------------------------------------------------------------------------
	// Device
	//----------------------------------------------------------------------------------------------
	MessageOut("Registering Devices...", p_bVerbose);
	engineKernel->GetDeviceManager()->RegisterFactory("Image", new ImageDeviceFactory());

	//----------------------------------------------------------------------------------------------
	// Cameras
	//----------------------------------------------------------------------------------------------
	MessageOut("Registering Cameras...", p_bVerbose);
	engineKernel->GetCameraManager()->RegisterFactory("Perspective", new PerspectiveCameraFactory());
	engineKernel->GetCameraManager()->RegisterFactory("ThinLens", new ThinLensCameraFactory());

	//----------------------------------------------------------------------------------------------
	// Lights
	//----------------------------------------------------------------------------------------------
	MessageOut("Registering Lights...", p_bVerbose);
	engineKernel->GetLightManager()->RegisterFactory("Point", new PointLightFactory());
	engineKernel->GetLightManager()->RegisterFactory("DiffuseArea", new DiffuseAreaLightFactory());
	engineKernel->GetLightManager()->RegisterFactory("InfiniteArea", new InfiniteAreaLightFactory());

	//----------------------------------------------------------------------------------------------
	// Shapes
	//----------------------------------------------------------------------------------------------
	MessageOut("Registering Shapes...", p_bVerbose);
	engineKernel->GetShapeManager()->RegisterFactory("KDTreeMesh", new KDTreeMeshShapeFactory());
	engineKernel->GetShapeManager()->RegisterFactory("Quad", new QuadMeshShapeFactory());
	engineKernel->GetShapeManager()->RegisterFactory("Triangle", new TriangleShapeFactory());
	engineKernel->GetShapeManager()->RegisterFactory("Sphere", new SphereShapeFactory());

	//----------------------------------------------------------------------------------------------
	// Textures
	//----------------------------------------------------------------------------------------------
	MessageOut("Registering Textures...", p_bVerbose);
	engineKernel->GetTextureManager()->RegisterFactory("Image", new ImageTextureFactory());
	engineKernel->GetTextureManager()->RegisterFactory("Noise", new NoiseTextureFactory());
	engineKernel->GetTextureManager()->RegisterFactory("Marble", new MarbleTextureFactory());

	//----------------------------------------------------------------------------------------------
	// Materials
	//----------------------------------------------------------------------------------------------
	MessageOut("Registering Materials...", p_bVerbose);
	engineKernel->GetMaterialManager()->RegisterFactory("Matte", new MatteMaterialFactory());
	engineKernel->GetMaterialManager()->RegisterFactory("Mirror", new MirrorMaterialFactory());
	engineKernel->GetMaterialManager()->RegisterFactory("Glass", new GlassMaterialFactory());
	engineKernel->GetMaterialManager()->RegisterFactory("Group", new MaterialGroupFactory());
	
	//----------------------------------------------------------------------------------------------
	// Environment
	//----------------------------------------------------------------------------------------------
	MessageOut("Initialising Environment...", p_bVerbose);
	*p_environment = new Environment(engineKernel);
}

void ShutdownIllumina(Environment *p_environment, bool p_bVerbose)
{
	p_environment->GetRenderer()->Shutdown();
	p_environment->GetIntegrator()->Shutdown();
}

/*
void randomshit(void)
{
		// Load environment script
	Message("Loading Environment script...", p_bVerbose);
	if (!environment.Load(p_strScript))
	{
		std::cerr << "Error : Unable to load environment script." << std::endl;
		exit(-1);
	}

	// Alias required components
	IIntegrator *pIntegrator = environment.GetIntegrator();
	IRenderer *pRenderer = environment.GetRenderer();
	ISpace *pSpace = environment.GetSpace();

	// Initialise integrator and renderer
	pIntegrator->Initialise(environment.GetScene(), environment.GetCamera());
	pRenderer->Initialise();

	// Initialisation complete
	Message("Initialisation complete. Rendering in progress...", p_bVerbose);

	// Initialise timing
	boost::timer frameTimer;
	float fTotalFramesPerSecond = 0.f;

	ICamera *pCamera = environment.GetCamera();
	float alpha = Maths::Pi;

	// Cornell
	//Vector3 lookFrom(70, 0, 70),
	//	lookAt(0, 0, 0);
	
	// Kiti
	//Vector3 lookFrom(-19, 1, -19),
	//	lookAt(0, 8, 0);
	
	// Sponza
	//Vector3 lookFrom(800, 100, 200),
	//	lookAt(0, 200, 100);
	for (int nFrame = 0; nFrame < p_nIterations; ++nFrame)
	{
		//alpha += Maths::PiTwo / 256;

		frameTimer.restart();
		
		//pCamera->MoveTo(lookFrom);
		//pCamera->MoveTo(Vector3(Maths::Cos(alpha) * lookFrom.X, lookFrom.Y, Maths::Sin(alpha) * lookFrom.Z));
		//pCamera->LookAt(lookAt);
	 
		// Update space
		pSpace->Update();
	 
		// Render frame
		pRenderer->Render();
	 
		// Compute frames per second
		fTotalFramesPerSecond += (float)(1.0f / frameTimer.elapsed());
		
		if (p_bVerbose)
		{
			std::cout << std::endl;
			std::cout << "-- Frame Render Time : [" << frameTimer.elapsed() << "s]" << std::endl;
			std::cout << "-- Frames per second : [" << fTotalFramesPerSecond / nFrame << "]" << std::endl;
		}
	}
}
*/

void RunAsServer(int argc, char **argv, bool p_bVerbose)
{
	Environment *environment = NULL;

	// Initialise MPI
	MPI_Init(&argc, &argv);
	
	// Get Process Rank
	int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// Load Illumina Environment
	InitialiseIllumina(&environment, p_bVerbose);

	// We need to detect whether this is running as load balancer, coordinator or worker
	if (rank == 0) Master(environment, p_bVerbose);
	else Idle(environment, p_bVerbose);

	// Shutdown Illumina Environment
	ShutdownIllumina(environment, p_bVerbose);

	// Finalise MPI
	MPI_Finalize();
}