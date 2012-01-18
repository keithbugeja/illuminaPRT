#pragma once

#include <stack>
#include <queue>
#include <vector>

#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>

#include "defs.h"
#include "taskgroup.h"
#include "taskpipeline.h"
#include "renderpipeline.h"

#include "../../Core/Threading/Spinlock.h"

using boost::asio::ip::tcp;
using namespace Illumina::Core;

/***************************************************************************************************
 * Client communication shit
 ***************************************************************************************************/
struct ClientSessionInfo
{
	std::string Script;
	std::string IP;
	int Priority;
	int Size;
};

enum ClientControlMessageType
{
	CCMT_Terminate,
	CCMT_Direction
};

struct IClientControlMessage
{
	int Id;
	std::string ClientIP;
};

template<int T>
struct TClientControlMessage
	: public IClientControlMessage
{
	TClientControlMessage(std::string &p_clientIP)
	{
		ClientIP = p_clientIP;
		Id = T;
	}
};

typedef TClientControlMessage<CCMT_Terminate> TerminateClientMessage;

struct DirectionClientMessage
	: IClientControlMessage
{
	int Direction;

	DirectionClientMessage(std::string &p_clientIP, ClientControlDirectionMessageType p_direction)
	{
		Id = CCMT_Direction;
		ClientIP = p_clientIP;
		Direction = p_direction;
	}
};

std::queue<IClientControlMessage*> g_clientControlQueue;
std::queue<ClientSessionInfo*> g_clientConnectQueue;

std::map<std::string, ClientSessionInfo*> g_clientIPSessionMap;
std::map<std::string, TaskGroup*> g_clientIPTaskGroupMap;

Illumina::Core::Spinlock g_clientSessionInfoLock;
Illumina::Core::Spinlock g_clientControlQueueLock;

/***************************************************************************************************
 * Communication thread
 ***************************************************************************************************/
typedef boost::shared_ptr<tcp::socket> socket_ptr;

void TokeniseCommand(std::string &p_command, std::vector<std::string> &p_arguments)
{
	p_arguments.clear();
	boost::split(p_arguments, p_command, boost::is_any_of(":"));
}

void ClientSession(socket_ptr p_socket)
{
	std::string clientIP = p_socket->remote_endpoint().address().to_string();
	std::cout << "Connection established with [" << clientIP << "]" << std::endl;
	
	try
	{
		std::vector<std::string> commandTokens;
		std::string commandString;

		for (;;)
		{
			char commandBuffer[1024];

			boost::system::error_code error;
			memset(commandBuffer, 0, 1024);
			size_t length = p_socket->read_some(boost::asio::buffer(commandBuffer, 1024), error);

			commandString.assign(commandBuffer);
			std::cout << "Received :: " << commandString << std::endl;

			if (commandString.find("[CMD_CONN]") != std::string::npos)
			{
				// We found a connection command : parse arguments
				// Should be in the format : [CMD_CONN]:script_path:priority:units
				TokeniseCommand(commandString, commandTokens);

				// Send a new request
				ClientSessionInfo *request = new ClientSessionInfo();
	
				request->IP = clientIP;
				request->Script = commandTokens[1];
				request->Priority = boost::lexical_cast<int>(commandTokens[2]);
				request->Size = boost::lexical_cast<int>(commandTokens[3]);

				g_clientSessionInfoLock.Lock();
				g_clientConnectQueue.push(request);
				g_clientSessionInfoLock.Unlock();
			}
			else if (commandString.find("[CMD_TERM]") != std::string::npos)
			{
				TerminateClientMessage *message = new TerminateClientMessage(clientIP);

				g_clientControlQueueLock.Lock();
				g_clientControlQueue.push(message);
				g_clientControlQueueLock.Unlock();

				// close connection
				std::cout << "Closing connection with [" << message->ClientIP << "]" << std::endl;
				break;
			}
			else if (commandString.find("[CMD_RSCADD]") != std::string::npos)
			{
				// We found a grow command : parse arguments
				// Should be in the format : [CMD_RSCADD]:units
				TokeniseCommand(commandString, commandTokens);

				//ResourceAddClientMessage *message = 
				//	new ResourceAddClientMessage(clientIP, boost::lexical_cast<int>(commandTokens[1]));

				//g_clientControlQueueLock.Lock();
				//g_clientControlQueue.push(message);
				//g_clientControlQueueLock.Unlock();
			}
			else if (commandString.find("[CMD_RSCSUB]") != std::string::npos)
			{
				// We found a grow command : parse arguments
				// Should be in the format : [CMD_RSCSUB]:units
				TokeniseCommand(commandString, commandTokens);

				//ResourceSubClientMessage *message = 
				//	new ResourceSubClientMessage(clientIP, boost::lexical_cast<int>(commandTokens[1]));

				//g_clientControlQueueLock.Lock();
				//g_clientControlQueue.push(message);
				//g_clientControlQueueLock.Unlock();
				
			}
			else if (commandString.find("[CMD_LFT]") != std::string::npos)
			{
				std::cout << "[" << clientIP << "] : [Left]" << std::cout;
				
				DirectionClientMessage *message = new DirectionClientMessage(clientIP, CCDMT_Left);

				g_clientControlQueueLock.Lock();
				g_clientControlQueue.push(message);
				g_clientControlQueueLock.Unlock();
			}
			else if (commandString.find("[CMD_RGT]") != std::string::npos)
			{
				std::cout << "[" << clientIP << "] : [Right]" << std::cout;
				DirectionClientMessage *message = new DirectionClientMessage(clientIP, CCDMT_Right);

				g_clientControlQueueLock.Lock();
				g_clientControlQueue.push(message);
				g_clientControlQueueLock.Unlock();
			}
			else if (commandString.find("[CMD_FWD]") != std::string::npos)
			{
				std::cout << "[" << clientIP << "] : [Forwards]" << std::cout;
				DirectionClientMessage *message = new DirectionClientMessage(clientIP, CCDMT_Forwards);

				g_clientControlQueueLock.Lock();
				g_clientControlQueue.push(message);
				g_clientControlQueueLock.Unlock();
			}
			else if (commandString.find("[CMD_BWD]") != std::string::npos)
			{
				std::cout << "[" << clientIP << "] : [Backwards]" << std::cout;
				DirectionClientMessage *message = new DirectionClientMessage(clientIP, CCDMT_Backwards);

				g_clientControlQueueLock.Lock();
				g_clientControlQueue.push(message);
				g_clientControlQueueLock.Unlock();
			}
			else if (commandString.find("[CMD_UP]") != std::string::npos)
			{
				std::cout << "[" << clientIP << "] : [Up]" << std::cout;
				DirectionClientMessage *message = new DirectionClientMessage(clientIP, CCDMT_Up);

				g_clientControlQueueLock.Lock();
				g_clientControlQueue.push(message);
				g_clientControlQueueLock.Unlock();
			}
			else if (commandString.find("[CMD_DN]") != std::string::npos)
			{
				std::cout << "[" << clientIP << "] : [Down]" << std::cout;
				DirectionClientMessage *message = new DirectionClientMessage(clientIP, CCDMT_Down);

				g_clientControlQueueLock.Lock();
				g_clientControlQueue.push(message);
				g_clientControlQueueLock.Unlock();
			}
		}	
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception in thread: " << e.what() << "\n";
	}
}

void MasterCommunication(TaskGroupList *p_taskGroupList)
{
	boost::asio::io_service ios;

	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 66600);
	boost::asio::ip::tcp::acceptor acceptor(ios, endpoint);
	boost::asio::ip::tcp::socket socket(ios);
	
	for (;;)
	{
		socket_ptr clientSocket(new boost::asio::ip::tcp::socket(ios));
		acceptor.accept(*clientSocket);
		boost::thread handlerThread(boost::bind(ClientSession, clientSocket));
	}
}

/***************************************************************************************************
 * Master process
 ***************************************************************************************************/
void Master(bool p_bVerbose)
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

	
	// -- launch
	boost::thread communicationThread(MasterCommunication, &taskGroupList);  
	// -- launch


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

	ClientSessionInfo *clientRequest = NULL;

	while(true)
	{
		// Set up an asynchronous receive on ChannelMasterStatic
		if (!masterCommunicator.ReceiveAsynchronous(&receiveMessage, MPI_ANY_SOURCE, &receiveRequest, &receiveStatus))
		{
			// Loop until a receive buffer contains a new message
			while(!masterCommunicator.IsRequestComplete(&receiveRequest, &receiveStatus))
			{
				//boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

				//////////////////////////////////////////////////////////////////////////////////////////////////
				// Do we have any pending messages for clients?
				//////////////////////////////////////////////////////////////////////////////////////////////////
				if (g_clientControlQueue.size() > 0)
				{
					// Pop message, if available
					g_clientControlQueueLock.Lock();

					IClientControlMessage *message = NULL;

					if (g_clientControlQueue.size() > 0)
					{
						message = g_clientControlQueue.front();
						g_clientControlQueue.pop();
					} 
					else 
						message = NULL;

					g_clientControlQueueLock.Unlock();

					if (message != NULL)
					{
						// Parse message
						switch (message->Id)
						{
							case CCMT_Terminate:
							{
								std::cout << "[" << masterTask->GetWorkerRank() << "] :: Master popped [CLIENT_TERMINATE] from message queue" << std::endl;

								TerminateClientMessage *cstMessage = (TerminateClientMessage*)message;
								TaskGroup *taskGroup = g_clientIPTaskGroupMap[cstMessage->ClientIP];

								std::cout << "[" << masterTask->GetWorkerRank() << "] :: Master sent [TERMINATE] to coordinator [" << 
									taskGroup->GetCoordinatorRank() << "]." << std::endl;

								TerminateMessage terminateMessage;
								masterCommunicator.Send(&terminateMessage, taskGroup->GetCoordinatorRank());
								break;
							}

							case CCMT_Direction:
							{
								std::cout << "[" << masterTask->GetWorkerRank() << "] :: Master popped [CLIENT_DIRECTION] from message queue" << std::endl;

								DirectionClientMessage *cstMessage = (DirectionClientMessage*)message;
								TaskGroup *taskGroup = g_clientIPTaskGroupMap[cstMessage->ClientIP];

								std::cout << "[" << masterTask->GetWorkerRank() << "] :: Master sent [DIRECTION] to coordinator [" << 
									taskGroup->GetCoordinatorRank() << "]." << std::endl;

								DirectionMessage directionMessage(cstMessage->Direction);
								masterCommunicator.Send(&directionMessage, taskGroup->GetCoordinatorRank());
								break;
							}

							default:
								break;
						}

						delete message;
						message = NULL;
					}
				}

				//////////////////////////////////////////////////////////////////////////////////////////////////
				// If we have a pending connection request, serve
				//////////////////////////////////////////////////////////////////////////////////////////////////
				if (g_clientConnectQueue.size() > 0)
				{
					g_clientSessionInfoLock.Lock();
					
					if (g_clientConnectQueue.size() > 0)
					{
						clientRequest = g_clientConnectQueue.front();
						g_clientConnectQueue.pop();
					}

					g_clientSessionInfoLock.Unlock();
				}

				// Generate request
				if (clientRequest != NULL) 
				{
					std::cout << "[" << masterTask->GetRank() << "] :: Master received request size of [" << clientRequest->Size << "] from [" << clientRequest->IP << "]" << std::endl;

					// Check if we can handle a request of the specified size
					if (clientRequest->Size > idleGroup->Size())
					{
						// Push request to back of queue
						g_clientSessionInfoLock.Lock();
						g_clientConnectQueue.push(clientRequest);
						g_clientSessionInfoLock.Unlock();

						clientRequest = NULL;

						/*  std::cout << "[" << masterTask->GetWorkerRank() << "] :: Master cannot satisfy request!" << std::endl;
					
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
						} */

						satisfiedRequest = false;
						// boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
					}
					else
					{
						// If request can be satisifed, split idle group
						TaskGroup *taskGroup = new TaskGroup(groupIDSource++);
						taskGroupList.AddTaskGroup(taskGroup->GetId(), taskGroup);
						idleGroup->Split(taskGroup, 0, clientRequest->Size - 1);

						// Set group coordinator
						taskGroup->SetCoordinatorRank(taskGroup->TaskList[0]->GetRank());

						// Now we must inform PEs that they have been assigned to a new task group
						//RequestMessage requestMessage(taskGroup->GetId(), 
						//	taskGroup->GetCoordinatorRank(), 
						//	taskGroup->Size());

						RequestMessageVL requestMessage(taskGroup->GetId(), 
							taskGroup->GetCoordinatorRank(), 
							clientRequest->Script);

						std::cout << requestMessage.Data->Id << ", " <<
							requestMessage.Data->CoordinatorId << ", " <<
							requestMessage.Data->GroupId << ", " <<
							requestMessage.Data->Config << ", " << 
							requestMessage.MessageSize() << std::endl;

						// taskGroup->Broadcast(masterTask, requestMessage, MM_ChannelMasterStatic);
						taskGroup->Broadcast(masterTask, &requestMessage, MM_ChannelMasterDynamic);
					
						// Index requests
						g_clientIPTaskGroupMap[clientRequest->IP] = taskGroup;
						g_clientIPSessionMap[clientRequest->IP] = clientRequest;

						std::cout << "[" << masterTask->GetRank() << "] :: Master created new task group with Id [" << taskGroup->GetId() << "]." << std::endl;

						clientRequest = NULL;
						satisfiedRequest = true;
						//boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
					}
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

	// Join threads
	communicationThread.join();
}

void FreeEnvironment(Environment *p_environment)
{
	p_environment->GetEngineKernel()->GetCameraManager()->ReleaseInstances();
	p_environment->GetEngineKernel()->GetDeviceManager()->ReleaseInstances();
	p_environment->GetEngineKernel()->GetFilterManager()->ReleaseInstances();
	p_environment->GetEngineKernel()->GetIntegratorManager()->ReleaseInstances();
	p_environment->GetEngineKernel()->GetLightManager()->ReleaseInstances();
	p_environment->GetEngineKernel()->GetMaterialManager()->ReleaseInstances();
	p_environment->GetEngineKernel()->GetRendererManager()->ReleaseInstances();
	p_environment->GetEngineKernel()->GetSamplerManager()->ReleaseInstances();
	p_environment->GetEngineKernel()->GetShapeManager()->ReleaseInstances();
	p_environment->GetEngineKernel()->GetSpaceManager()->ReleaseInstances();
	p_environment->GetEngineKernel()->GetTextureManager()->ReleaseInstances();

	delete p_environment;
}

/***************************************************************************************************
 * Idle process
 ***************************************************************************************************/
void Idle(EngineKernel *p_engineKernel, bool p_bVerbose)
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
	
	char messageBuffer[2048];
	VarlenMessage messageVarlen(messageBuffer, 2048);
	Message receiveMessage;

	while(true)
	{
		/*
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));
		
		// Set up receive from Master on ChannelMasterStatic
		if (!masterCommunicator.ReceiveAsynchronous(receiveMessage, idleTask->GetMasterRank(), &receiveRequest, &receiveStatus))
		{
			std::cout << "[" << idleTask->GetRank() << "] :: Idle task awaiting assignment." << std::endl;

			while(!masterCommunicator.IsRequestComplete(&receiveRequest, &receiveStatus))
			{
				boost::this_thread::sleep(boost::posix_time::milliseconds(250));
			}
		}*/

		TaskCommunicator::Probe(idleTask->GetMasterRank(), MM_ChannelMasterDynamic, &receiveStatus);
		TaskCommunicator::Receive((void*)messageBuffer, TaskCommunicator::GetSize(&receiveStatus), idleTask->GetMasterRank(), MM_ChannelMasterDynamic, &receiveStatus);

		//////////////////////////////////////////////////////////////////////////////////////////////////
		// Handle received message
		//////////////////////////////////////////////////////////////////////////////////////////////////	
		switch (messageVarlen.MessageId())
		//switch (receiveMessage.Id)
		{
			case MT_Request:
			{
				// RequestMessage *requestMessage = (RequestMessage*)&receiveMessage;
				RequestMessageVL requestMessage((void*)messageBuffer);

				std::cout << "[" << idleTask->GetRank() << "] :: Idle task received [REQUEST] for " 					
					<< "group [" << requestMessage.GetGroupId() << "], "
					<< "coordinator [" << requestMessage.GetCoordinatorId() << "], " 
					<< "arguments [" << requestMessage.GetConfig() << "]. "
					<< std::endl;

				int groupId = requestMessage.GetGroupId();

				// Set coordinator for idle task
				idleTask->SetCoordinatorRank(requestMessage.GetCoordinatorId());

				// If this task is the coordinator, spawn coordinator code
				if (idleTask->GetCoordinatorRank() == idleTask->GetWorkerRank())
				{
					std::cout << "[" << idleTask->GetRank() << "] :: Idle task changing to communicator for group [" << requestMessage.GetGroupId() << "]." << std::endl;

					Environment *environment = new Environment(p_engineKernel);
					RenderPipeline pipeline(environment, std::string(requestMessage.GetConfig()), p_bVerbose);

					// Might have to revise constructor for coordinator!!!
					ITaskPipeline::CoordinatorTask coordinator;
					coordinator.task = idleTask;
					coordinator.group.SetMasterRank(idleTask->GetMasterRank());
					coordinator.group.SetCoordinatorRank(idleTask->GetWorkerRank());

					pipeline.Coordinator(coordinator);
					FreeEnvironment(environment);

					std::cout << "[" << idleTask->GetRank() << "] :: Coordinator changing back to idle task for group [" << requestMessage.GetGroupId() << "]." << std::endl;

					// We need to tell master that we are ready
					CompletedMessage completedMessage(groupId);
					masterCommunicator.SendToMaster((IMessage*)&completedMessage);
				}
				else
				{
					std::cout << "[" << idleTask->GetRank() << "] :: Idle task changing to worker for group [" << requestMessage.GetGroupId() << "]." << std::endl;

					Environment *environment = new Environment(p_engineKernel);
					RenderPipeline pipeline(environment, std::string(requestMessage.GetConfig()), p_bVerbose);

					pipeline.Worker(idleTask);
					FreeEnvironment(environment);
	
					std::cout << "[" << idleTask->GetRank() << "] :: Worker changing back to idle task for group [" << requestMessage.GetGroupId() << "]." << std::endl;

					// We need to tell master that we are ready
					CompletedMessage completedMessage(groupId);
					masterCommunicator.SendToMaster((IMessage*)&completedMessage);
				}
				break;
			}

			default:
				break;
		}
	}
}

/***************************************************************************************************
 * Initialise Illumina
 ***************************************************************************************************/
void InitialiseIllumina(EngineKernel **p_engineKernel, bool p_bVerbose)
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

	*p_engineKernel = engineKernel;
}

/***************************************************************************************************
 * Shutdown Illumina
 ***************************************************************************************************/
void ShutdownIllumina(EngineKernel *p_engineKernel, bool p_bVerbose)
{
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

/***************************************************************************************************
 * RunAs method
 ***************************************************************************************************/
void RunAsServer(int argc, char **argv, bool p_bVerbose)
{
	// Initialise MPI
	MPI_Init(&argc, &argv);
	
	// Get Process Rank
	int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// We need to detect whether this is running as load balancer, coordinator or worker
	if (rank == 0)
	{
		Master(p_bVerbose);
	}
	else 
	{
		EngineKernel *engineKernel = NULL;

		InitialiseIllumina(&engineKernel, p_bVerbose);
		Idle(engineKernel, p_bVerbose);
		ShutdownIllumina(engineKernel, p_bVerbose);
	}

	// Finalise MPI
	MPI_Finalize();
}