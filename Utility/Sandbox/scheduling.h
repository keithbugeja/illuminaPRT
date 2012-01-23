#pragma once

#include <stack>
#include <queue>
#include <vector>

#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "defs.h"
#include "taskgroup.h"
#include "taskpipeline.h"
#include "renderpipeline.h"

#include "../../Core/Threading/Spinlock.h"
#include "../../Core/Threading/Atomic.h"

using boost::asio::ip::tcp;
using namespace Illumina::Core;

/***************************************************************************************************
 * Client communication shit
 ***************************************************************************************************/
// Todo: Assign Token Information to client/admin

// Messages:
// 1. Admin : Get Session List
// 2. Admin : Get Session Resources
// 3. Admin : Set Session Resources

// Messages
// 1. Client : New Session
// 2. Client : Terminate Session
// 3. Client : Direction

Int32 g_clientUID = 0;

inline int GetNextClientID(void) {
	return (int)AtomicInt32::FetchAndAdd(&g_clientUID, (Int32)1);
}

struct ClientSessionInfo
{
	bool Admin;

	int Id;
	int Priority;
	int Size;

	TaskGroup *Group;

	std::string Arguments;
	std::string IP;
};

enum ClientMessageType
{
	CMT_Connect,
	CMT_Disconnect,
	CMT_Request,
	CMT_Release,
	CMT_Input
};

enum AdminMessageType
{
	CMT_QueryGroups,
	CMT_QueryResource,
	CMT_SetResource
};

struct IClientMessage
{
	int Id;
	ClientSessionInfo *SessionInfo;

	IClientMessage(int p_id, ClientSessionInfo *p_session) : Id(p_id), SessionInfo(p_session) { }
};

template<int T>
struct TClientMessage
	: public IClientMessage
{
	TClientMessage(ClientSessionInfo *p_session)
		: IClientMessage(T, p_session)
	{ }
};

typedef TClientMessage<CMT_Connect> ConnectMessage;
typedef TClientMessage<CMT_Disconnect> DisconnectMessage;

struct InputMessage
	: IClientMessage
{
	int Input;

	InputMessage(ClientSessionInfo *p_session, ClientInputType p_input)
		: IClientMessage(CMT_Input, p_session)
		, Input(p_input)
	{ }
};

struct ReleasePEMessage
	: IClientMessage
{
	int Count;

	ReleasePEMessage(ClientSessionInfo *p_session, int p_count)
		: IClientMessage(CMT_Release, p_session)
		, Count(p_count)
	{ }
};

struct RequestPEMessage
	: IClientMessage
{
	int Count;

	RequestPEMessage(ClientSessionInfo *p_session, int p_count)
		: IClientMessage(CMT_Request, p_session)
		, Count(p_count)
	{ }
};

std::map<int, ClientSessionInfo*> g_clientSessionMap;
std::queue<IClientMessage*> g_clientMessageQueue;
Illumina::Core::Spinlock g_clientMessageQueueLock;
Illumina::Core::Spinlock g_clientSessionMapLock;

/***************************************************************************************************
 * Communication thread
 ***************************************************************************************************/
typedef boost::shared_ptr<tcp::socket> socket_ptr;

void TokeniseCommand(std::string &p_command, std::vector<std::string> &p_arguments)
{
	p_arguments.clear();
	boost::split(p_arguments, p_command, boost::is_any_of(":"));
}

ClientSessionInfo* AdminConnect(int p_clientId, std::string &p_clientIP)
{
	// Create client session info
	ClientSessionInfo *clientSessionInfo = new ClientSessionInfo();

	clientSessionInfo->Id = p_clientId;
	clientSessionInfo->IP = p_clientIP;
	clientSessionInfo->Arguments.clear();

	clientSessionInfo->Admin = true;
	
	std::cout << "[" << p_clientIP << "] :: Opening admin connection from and assigning Id = [" << p_clientId << "]" << std::endl;
	
	return clientSessionInfo;
}

void AdminDisconnect(ClientSessionInfo *p_clientSessionInfo)
{
	// close connection
	std::cout << "[" << p_clientSessionInfo->IP << ":" << p_clientSessionInfo->Id << "] :: Received [Admin Terminate]." << std::endl;
	delete p_clientSessionInfo;
}

ClientSessionInfo* ClientConnect(std::vector<std::string> &p_commandTokens, int p_clientId, std::string &p_clientIP)
{
	// Create client session info
	ClientSessionInfo *clientSessionInfo = new ClientSessionInfo();

	clientSessionInfo->Id = p_clientId;
	clientSessionInfo->IP = p_clientIP;
	clientSessionInfo->Arguments = p_commandTokens[1];
	clientSessionInfo->Admin = false;
	clientSessionInfo->Priority = boost::lexical_cast<int>(p_commandTokens[2]);
	clientSessionInfo->Size = boost::lexical_cast<int>(p_commandTokens[3]);
	clientSessionInfo->Group = NULL;
	clientSessionInfo->Admin = false;

	ConnectMessage *message = new ConnectMessage(clientSessionInfo);

	// Post request to queue
	g_clientMessageQueueLock.Lock();
	g_clientMessageQueue.push(message);
	g_clientMessageQueueLock.Unlock();
	
	std::cout << "[" << p_clientIP << "] :: Opening connection from and assigning Id = [" << p_clientId << "]" << std::endl;
	
	return clientSessionInfo;
}

void ClientDisconnect(ClientSessionInfo *p_clientSessionInfo)
{
	// close connection
	std::cout << "[" << p_clientSessionInfo->IP << ":" << p_clientSessionInfo->Id << "] :: Received [Terminate]." << std::endl;

	DisconnectMessage *message = new DisconnectMessage(p_clientSessionInfo);

	g_clientMessageQueueLock.Lock();
	g_clientMessageQueue.push(message);
	g_clientMessageQueueLock.Unlock();
}

void ClientAdjust(ClientSessionInfo *p_clientSessionInfo, int p_count)
{
	std::cout << "[" << p_clientSessionInfo->IP << ":" << p_clientSessionInfo->Id << "] :: Received [Adjust PE]." << std::endl;

	if (p_count > p_clientSessionInfo->Group->Size())
	{
		// Request
		int requestCount = p_count - p_clientSessionInfo->Group->Size();

		RequestPEMessage *message = new RequestPEMessage(p_clientSessionInfo, requestCount);

		g_clientMessageQueueLock.Lock();
		g_clientMessageQueue.push(message);
		g_clientMessageQueueLock.Unlock();
	}
	else
	{
		// Release
		int releaseCount = p_clientSessionInfo->Group->Size() - p_count;
					
		if (releaseCount > 0)
		{
			ReleasePEMessage *message = new ReleasePEMessage(p_clientSessionInfo, releaseCount);

			g_clientMessageQueueLock.Lock();
			g_clientMessageQueue.push(message);
			g_clientMessageQueueLock.Unlock();
		}
	}
}

void ClientInput(ClientSessionInfo *p_session, std::vector<std::string>& p_commandTokens)
{
}

void ClientStreamSession(socket_ptr p_socket, ClientSessionInfo *p_session)
{
	Illumina::Core::ImagePPM image;

	int size = /* 32*32*3 */ 512 * 512 * 3;
	char buf[512 * 512 * 3];
	std::fstream fstr;

	char magicNumber[2], whitespace;
	int	width, height, colours;

	for (;;)
	{
		std::cout << "Try Open" << std::endl;

		try 
		{
			fstr.open("Output//result_1.ppm");

			if (fstr.is_open())
			{
				std::cout << "Readfile" << std::endl;
				
				fstr.get(magicNumber[0]);
				fstr.get(magicNumber[1]);
				fstr.get(whitespace);
				fstr >> std::noskipws >> width;
				fstr.get(whitespace);
				fstr >> std::noskipws >> height;
				fstr.get(whitespace);
				fstr >> std::noskipws >> colours;
				fstr.get(whitespace);

				fstr.read(buf, size);
				fstr.close();

				std::cout << "Send body to " << p_socket->remote_endpoint().address().to_string() << ":" << p_socket->remote_endpoint().port() << std::endl;

				boost::asio::write(*p_socket, boost::asio::buffer(buf, size)); 

				std::cout << "Sent body" << std::endl;
			}
		}
		catch(boost::system::system_error e)
		{
			std::cout << "EXC:" << e.what() << ", " << std::endl;
			std::cout << "wait" << std::endl;
			boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		}
	}
}

void ClientStream(boost::asio::io_service &p_ios, ClientSessionInfo *p_clientSessionInfo, std::vector<std::string>& p_commandTokens)
{
	std::cout << "[" << p_clientSessionInfo->IP << ":" << p_clientSessionInfo->Id << "] :: Received [Initialise Image Stream]." << std::endl;

    tcp::resolver resolver(p_ios);
    tcp::resolver::query query(tcp::v4(), p_clientSessionInfo->IP, p_commandTokens[1]);
    tcp::resolver::iterator iterator = resolver.resolve(query);

	socket_ptr clientSocket(new tcp::socket(p_ios));
	tcp::socket *s = clientSocket.get();
    boost::asio::connect(*s, iterator);

	std::cout << "Stream connection initialised!";

	boost::thread handlerThread(boost::bind(ClientStreamSession, clientSocket, p_clientSessionInfo));

	std::cout << "Stream OK!";
}

void ClientSession(socket_ptr p_socket)
{
	boost::asio::io_service io_service;

	// Get client IP 
	std::string clientIP = p_socket->remote_endpoint().address().to_string();
	std::cout << "Connection established with [" << clientIP << "]" << std::endl;

	try
	{
		ClientSessionInfo *clientSessionInfo = NULL;
		
		// Store command string and tokens
		boost::system::error_code error;
		std::vector<std::string> commandTokens;
		std::string commandString;
		char commandBuffer[1024];

		for (;;)
		{
			// clear command buffer and read some data
			memset(commandBuffer, 0, 1024);
			size_t length = 
				p_socket->read_some(boost::asio::buffer(commandBuffer, 1024), error);

			// assign command buffer to command string
			commandString.assign(commandBuffer);
			boost::algorithm::trim(commandString);

			if (commandString.size() == 0)
				continue;

			std::cout << "[" << clientIP << "] :: Received :: [" << commandString << "]" << std::endl;

			/* 
			 * Parse commands
			 */

			// new connection
			if (commandString.find("[ADM_CONN]") != std::string::npos)
			{
				clientSessionInfo = AdminConnect(GetNextClientID(), clientIP);
			}
			else if (commandString.find("[CMD_CONN]") != std::string::npos)
			{
				// We found a connection command : parse arguments
				// Should be in the format : [CMD_CONN]:script_path:priority:units
				TokeniseCommand(commandString, commandTokens);
				clientSessionInfo = ClientConnect(commandTokens, GetNextClientID(), clientIP);
			}
			else 
			{
				// Following commands are valid only if session has been established
				if (clientSessionInfo != NULL) 
				{
					// Is this an admin session?
					if (clientSessionInfo->Admin)
					{
						// Terminate connection
						if (commandString.find("[ADM_TERM]") != std::string::npos)
						{
							AdminDisconnect(clientSessionInfo);
							break;
						} 
						// Change session resources
						else if (commandString.find("[ADM_PE]") != std::string::npos)
						{
							TokeniseCommand(commandString, commandTokens);

							int sessionId = boost::lexical_cast<int>(commandTokens[1]),
								count = boost::lexical_cast<int>(commandTokens[2]);

							ClientSessionInfo *sessionInfo = NULL;

							g_clientSessionMapLock.Lock();

							if (g_clientSessionMap.find(sessionId) != g_clientSessionMap.end())
								sessionInfo = g_clientSessionMap[sessionId];

							g_clientSessionMapLock.Unlock();

							if (sessionInfo != NULL) ClientAdjust(sessionInfo, count);
						}
					}
					else
					{
						// Terminate connection
						if (commandString.find("[CMD_TERM]") != std::string::npos)
						{
							ClientDisconnect(clientSessionInfo);
							break;
						}
						// Change resources
						else if (commandString.find("[CMD_PE]") != std::string::npos)
						{
							TokeniseCommand(commandString, commandTokens);
							ClientAdjust(clientSessionInfo, boost::lexical_cast<int>(commandTokens[1]));
						}
						// Input command
						else if (commandString.find("[CMD_INP]") != std::string::npos)
						{
							TokeniseCommand(commandString, commandTokens);
							ClientInput(clientSessionInfo, commandTokens);
						}
						// Stream
						else if (commandString.find("[CMD_STREAM]") != std::string::npos)
						{
							TokeniseCommand(commandString, commandTokens);
							ClientStream(io_service, clientSessionInfo, commandTokens);
						}
					}
				}
			}
			/*
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
			*/
		}

		clientSessionInfo = NULL;
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception in thread: " << e.what() << "\n";
	}
}

void MasterCommunication(TaskGroupList *p_taskGroupList)
{
	boost::asio::io_service ios;

	tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 6660);
	tcp::acceptor acceptor(ios, endpoint);
	
	for (;;)
	{
		// Potential race hazard!! Change after paper demo!
		socket_ptr clientSocket(new tcp::socket(ios));
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
				IClientMessage *message = NULL;

				while (g_clientMessageQueue.size() > 0)
				{
					// Pop message, if available
					g_clientMessageQueueLock.Lock();

					if (g_clientMessageQueue.size() > 0)
					{
						message = g_clientMessageQueue.front();
						g_clientMessageQueue.pop();
					} 
					else 
						message = NULL;

					g_clientMessageQueueLock.Unlock();


					if (message != NULL)
					{
						// Parse message
						switch (message->Id)
						{
							case CMT_Connect:
							{
								std::cout << "[" << masterTask->GetRank() << "] :: Master popped [CLIENT_CONNECT] from message queue with a request size of [" 
									<< message->SessionInfo->Size << "] from [" << message->SessionInfo->IP << "]" << std::endl;

								// Check if we can handle a request of the specified size
								if (message->SessionInfo->Size > idleGroup->Size())
								{
									std::cout << "[" << masterTask->GetRank() << "] :: Cannot satisfy request. Ignoring." << std::endl;
									// Push request to back of queue
									// g_clientMessageQueueLock.Lock();
									// g_clientMessageQueue.push(message);
									// g_clientMessageQueueLock.Unlock();
								}
								else
								{
									ClientSessionInfo *session = message->SessionInfo;

									// If request can be satisifed, split idle group
									TaskGroup *taskGroup = session->Group =
										new TaskGroup(groupIDSource++);

									taskGroupList.AddTaskGroup(taskGroup->GetId(), taskGroup);
									idleGroup->Split(taskGroup, 0, session->Size - 1);

									// Set group coordinator
									taskGroup->SetCoordinatorRank(taskGroup->TaskList[0]->GetRank());

									// Now we must inform PEs that they have been assigned to a new task group
									RequestMessage requestMessage(taskGroup->GetId(), 
										taskGroup->GetCoordinatorRank(), 
										session->Arguments);

									std::cout << "[Request] :: " <<
										requestMessage.Data->Id << ", " <<
										requestMessage.Data->CoordinatorId << ", " <<
										requestMessage.Data->GroupId << ", " <<
										requestMessage.Data->Config << ", " << 
										requestMessage.MessageSize() << std::endl;

									taskGroup->Broadcast(masterTask, &requestMessage, MM_ChannelMasterDynamic);
					
									// Index request
									std::cout << "[Session] :: " << 
										session->Id << ", " <<
										session->IP << ", " << 
										session->Group->GetId() << ", " <<
										session->Arguments << std::endl;

									g_clientSessionMapLock.Lock();
									g_clientSessionMap[session->Id] = session;
									g_clientSessionMapLock.Unlock();

									std::cout << "[" << masterTask->GetRank() << "] :: Master created new task group with Id [" << taskGroup->GetId() << "]." << std::endl;
								}

								break;
							}

							case CMT_Disconnect:
							{
								std::cout << "[" << masterTask->GetRank() << "] :: Master popped [CLIENT_TERMINATE] from message queue." << std::endl;

								ClientSessionInfo *session = message->SessionInfo;
								TaskGroup *taskGroup = session->Group;

								std::cout << "[" << masterTask->GetRank() << "] :: Master terminating session [" << 
									session->IP << ":" << session->Id <<"]" << std::endl;

								TerminateMessage terminateMessage; 
								masterCommunicator.Send(&terminateMessage, taskGroup->GetCoordinatorRank());

								std::cout << "[" << masterTask->GetRank() << "] :: Master sent [TERMINATE] to coordinator [" << 
									taskGroup->GetCoordinatorRank() << "]." << std::endl;

								g_clientSessionMapLock.Lock();
								g_clientSessionMap.erase(session->Id);
								g_clientSessionMapLock.Unlock();
								
								break;
							}

							case CMT_Request:
							{
								RequestPEMessage *requestMessage = (RequestPEMessage*)message;
								ClientSessionInfo *session = requestMessage->SessionInfo;

								std::cout << "[" << masterTask->GetRank() << "] :: Master popped [ADMIN_REQUEST] from message queue for an additional request size of [" << 
									requestMessage->Count << "] from [" << session->IP << ":" << session->Id << "]" << std::endl;

								// Check if we can handle a request of the specified size
								if (requestMessage->Count > idleGroup->Size())
								{
									std::cout << "[" << masterTask->GetRank() << "] :: Cannot satisfy request. Ignoring." << std::endl;

									// Push request to back of queue
									// g_clientMessageQueueLock.Lock();
									// g_clientMessageQueue.push(message);
									// g_clientMessageQueueLock.Unlock();
								}
								else
								{
									// If request can be satisifed, split idle group
									TaskGroup *taskGroup = message->SessionInfo->Group,
										tempGroup;
										
									idleGroup->Split(&tempGroup, 0, requestMessage->Count - 1);

									// Now merge
									for (std::vector<Task*>::iterator taskIterator = tempGroup.TaskList.begin();
										 taskIterator != tempGroup.TaskList.end(); ++taskIterator)
									{
										taskGroup->Merge(*taskIterator);
									}

									// Now we must inform PEs that they have been assigned to a new task group
									RequestMessage requestMessage(taskGroup->GetId(), 
										taskGroup->GetCoordinatorRank(), 
										session->Arguments);

									std::cout << "[Request] :: " <<
										requestMessage.Data->Id << ", " <<
										requestMessage.Data->CoordinatorId << ", " <<
										requestMessage.Data->GroupId << ", " <<
										requestMessage.Data->Config << ", " << 
										requestMessage.MessageSize() << std::endl;

									tempGroup.Broadcast(masterTask, &requestMessage, MM_ChannelMasterDynamic);
					
									// Index request
									std::cout << "[Session] :: " << 
										session->Id << ", " <<
										session->IP << ", " << 
										session->Group->GetId() << ", " <<
										session->Arguments << std::endl;

									std::cout << "[" << masterTask->GetRank() << "] :: Master incremented task group with Id [" << taskGroup->GetId() << "] to [" << taskGroup->Size() << "] units." << std::endl;
								}

								break;
							}

							case CMT_Release:
							{
								ClientSessionInfo *session = message->SessionInfo;
								ReleasePEMessage *releasePEMessage = (ReleasePEMessage*)message;

								std::cout << "[" << masterTask->GetRank() << "] :: Master popped [ADMIN_RELEASE] from message queue for an reduction of [" << 
									releasePEMessage->Count << "] units from [" << session->IP << ":" << session->Id <<"]" << std::endl;

								ReleaseMessage releaseMessage(releasePEMessage->Count);
								masterCommunicator.Send(&releaseMessage, session->Group->GetCoordinatorRank());

								std::cout << "[" << masterTask->GetRank() << "] :: Master notified coordinator of group with Id [" << session->Group->GetId() << 
									"] to release [" << releasePEMessage->Count << "] units. " << std::endl;

								break;
							}

							/*
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
							*/

							default:
								break;
						}

						delete message;
						message = NULL;
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
			
				std::cout << "[" << masterTask->GetRank() << "] :: Master received [COMPLETED] from worker [" << 
					releaseIndex << "] of group [" << groupId << "]." << std::endl;

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

							ClientSessionInfo *session = NULL;

							g_clientSessionMapLock.Lock();

							for (std::map<int, ClientSessionInfo*>::iterator sessionIterator = g_clientSessionMap.begin();
								 sessionIterator != g_clientSessionMap.end(); ++sessionIterator)
							{
								if ((*sessionIterator).second->Group == taskGroup)
								{
									g_clientSessionMap.erase(session->Id); break;
								}
							}

							g_clientSessionMapLock.Unlock();

							if (session) delete session;
							if (taskGroup) delete taskGroup;
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

/***************************************************************************************************
 * Idle process
 ***************************************************************************************************/
void Idle(bool p_bVerbose)
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
		//boost::this_thread::sleep(boost::posix_time::milliseconds(500));
		//
		//// Set up receive from Master on ChannelMasterStatic
		//if (!masterCommunicator.ReceiveAsynchronous(receiveMessage, idleTask->GetMasterRank(), &receiveRequest, &receiveStatus))
		//{
		//	std::cout << "[" << idleTask->GetRank() << "] :: Idle task awaiting assignment." << std::endl;

		//	while(!masterCommunicator.IsRequestComplete(&receiveRequest, &receiveStatus))
		//	{
		//		boost::this_thread::sleep(boost::posix_time::milliseconds(250));
		//	}
		//}
		std::cout << "[" << idleTask->GetRank() << "] :: Idle task set to idle mode" << std::endl;	

		TaskCommunicator::Probe(idleTask->GetMasterRank(), MM_ChannelMasterDynamic, &receiveStatus);	
		TaskCommunicator::Receive((void*)messageBuffer, TaskCommunicator::GetSize(&receiveStatus), idleTask->GetMasterRank(), MM_ChannelMasterDynamic, &receiveStatus);

		//////////////////////////////////////////////////////////////////////////////////////////////////
		// Handle received message
		//////////////////////////////////////////////////////////////////////////////////////////////////	
		switch (messageVarlen.MessageId())
		{
			case MT_Request:
			{
				// RequestMessage *requestMessage = (RequestMessage*)&receiveMessage;
				RequestMessage requestMessage((void*)messageBuffer);

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

					// Might have to revise constructor for coordinator!!!
					ITaskPipeline::CoordinatorTask coordinator;
					coordinator.task = idleTask;
					coordinator.group.SetMasterRank(idleTask->GetMasterRank());
					coordinator.group.SetCoordinatorRank(idleTask->GetWorkerRank());

					RenderPipeline pipeline(std::string(requestMessage.GetConfig()), p_bVerbose);
					pipeline.Coordinator(coordinator);

					std::cout << "[" << idleTask->GetRank() << "] :: Coordinator changing back to idle task for group [" << requestMessage.GetGroupId() << "]." << std::endl;

					// We need to tell master that we are ready
					CompletedMessage completedMessage(groupId);
					masterCommunicator.SendToMaster((IMessage*)&completedMessage);
				}
				else
				{
					std::cout << "[" << idleTask->GetRank() << "] :: Idle task changing to worker for group [" << requestMessage.GetGroupId() << "]." << std::endl;

					RenderPipeline pipeline(std::string(requestMessage.GetConfig()), p_bVerbose);
					pipeline.Worker(idleTask);

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
		Master(p_bVerbose);
	else 
		Idle(p_bVerbose);

	// Finalise MPI
	MPI_Finalize();
}