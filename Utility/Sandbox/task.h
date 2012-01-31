#pragma once

#include <map>
#include <vector>
#include <cstdarg>

#include <boost/thread.hpp>
#include <boost/mpi.hpp>
#include <boost/format.hpp>

#include "message.h"

namespace mpi = boost::mpi;

namespace Illumina
{
	namespace Core
	{
		class Task
		{
		public:
			enum TaskType 
			{
				TT_Master	   = 0,
				TT_Coordinator = 1,
				TT_Worker	   = 2,
				TT_Count
			} Type;
			
		public:
			int Rank[TT_Count];

		public:
			Task(int p_workerRank = -1, int p_masterRank = -1, int p_coordinatorRank = -1)
			{
				Rank[TT_Worker] = p_workerRank;
				Rank[TT_Master] = p_masterRank;
				Rank[TT_Coordinator] = p_coordinatorRank;
			}

			inline int GetRank(void) const { return GetWorkerRank(); }

			int GetMasterRank(void) const { return Rank[TT_Master]; }
			void SetMasterRank(int p_nRank) { Rank[TT_Master] = p_nRank; }

			int GetCoordinatorRank(void) const { return Rank[TT_Coordinator]; }
			void SetCoordinatorRank(int p_nRank) { Rank[TT_Coordinator] = p_nRank; }

			int GetWorkerRank(void) const { return Rank[TT_Worker]; }
			void SetWorkerRank(int p_nRank) { Rank[TT_Worker] = p_nRank; }

			bool IsMaster(void) const { return Rank[TT_Worker] >= 0 && Rank[TT_Worker] == Rank[TT_Master]; }
			bool IsCoordinator(void) const { return Rank[TT_Worker] >= 0 && Rank[TT_Worker] == Rank[TT_Coordinator]; }
			bool IsWorker(void) const { return !(IsMaster() || IsCoordinator()); }
			
			std::string ToString(void) const
			{
				std::string strOut = boost::str(boost::format("[master = %d, coordinator = %d, worker = %d]") 
					% this->GetMasterRank()
					% this->GetCoordinatorRank() 
					% this->GetWorkerRank());
				
				return strOut;
			}
		};


		class TaskCommunicator
		{
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

		class MessageCommunicator
			: public TaskCommunicator
		{
		public:
			bool Send(IMessage *p_message, int p_rank, int p_tag) {
				return TaskCommunicator::Send(p_message->MessageBuffer(), p_message->MessageSize(), p_rank, p_tag);
			}

			bool Receive(IMessage *p_message, int p_rank, int p_tag) 
			{
				switch (p_message->MessageType())
				{
					case IMessage::IMT_Variable:
					{
						MPI_Status status; TaskCommunicator::Probe(p_rank, p_tag, &status);
						return TaskCommunicator::Receive(p_message->MessageBuffer(), TaskCommunicator::GetSize(&status), p_rank, p_tag, &status);
					}

					case IMessage::IMT_Fixed:
					{
						return TaskCommunicator::Receive(p_message->MessageBuffer(), p_message->MessageSize(), p_rank, p_tag);
					}

					default:
						return false;
				}
			}

			bool Receive(IMessage *p_message, int p_tag, MPI_Status *p_statusOut) 
			{
				switch (p_message->MessageType())
				{
					case IMessage::IMT_Variable:
					{
						MPI_Status status; TaskCommunicator::Probe(MPI_ANY_SOURCE, p_tag, p_statusOut);
						return TaskCommunicator::Receive(p_message->MessageBuffer(), TaskCommunicator::GetSize(p_statusOut), p_statusOut->MPI_SOURCE, p_tag, p_statusOut);
					}

					case IMessage::IMT_Fixed:
					{
						return TaskCommunicator::Receive(p_message->MessageBuffer(), p_message->MessageSize(), MPI_ANY_SOURCE, p_tag, p_statusOut);
					}

					default:
						return false;
				}
			}

			bool SendAsynchronous(IMessage *p_message, int p_rank, int p_tag, MPI_Request *p_request) {
				return TaskCommunicator::SendAsynchronous(p_message->MessageBuffer(), p_message->MessageSize(), p_rank, p_tag, p_request);
			}

			bool ReceiveAsynchronous(IMessage *p_message, int p_rank, int p_tag, MPI_Request *p_request, MPI_Status *p_status = MPI_STATUS_IGNORE) 
			{
				if (p_message->MessageType() != IMessage::IMT_Fixed)
				{
					std::cout << "ReceiveAsynchronous::Error - Cannot receive variable-sized messages asynchronously!" << std::endl;
					return false;
				}

				return TaskCommunicator::ReceiveAsynchronous(p_message->MessageBuffer(), p_message->MessageSize(), p_rank, p_tag, p_request, p_status);
			}
		};


		class MailboxCommunicator
			: public MessageCommunicator
		{
		protected:
			int *m_rank;

		public:
			using MessageCommunicator::SendAsynchronous;

		public:
			MailboxCommunicator(void)
				: m_rank(NULL)
			{ }

			MailboxCommunicator(int *p_rank)
				: m_rank(p_rank)
			{ }

			MailboxCommunicator(Task *p_task)
				: m_rank(p_task->Rank)
			{ }

			bool SendToCoordinator(IMessage *p_message, int p_tag) {
				return MessageCommunicator::Send(p_message, m_rank[Task::TT_Coordinator], p_tag);
			}

			bool ReceiveFromCoordinator(IMessage *p_message, int p_tag) {
				return MessageCommunicator::Receive(p_message, m_rank[Task::TT_Coordinator], p_tag);
			}

			bool SendToCoordinatorAsynchronous(IMessage *p_message, int p_tag, MPI_Request *p_request) {
				return MessageCommunicator::SendAsynchronous(p_message, m_rank[Task::TT_Coordinator], p_tag, p_request);
			}

			bool ReceiveFromCoordinatorAsynchronous(IMessage *p_message, int p_tag, MPI_Request *p_request) {
				return MessageCommunicator::ReceiveAsynchronous(p_message, m_rank[Task::TT_Coordinator], p_tag, p_request);
			}

			bool SendToMaster(IMessage *p_message, int p_tag) {
				return MessageCommunicator::Send(p_message, m_rank[Task::TT_Master], p_tag);
			}

			bool ReceiveFromMaster(IMessage *p_message, int p_tag) {
				return MessageCommunicator::Receive(p_message, m_rank[Task::TT_Master], p_tag);
			}

			bool SendToMasterAsynchronous(IMessage *p_message, int p_tag, MPI_Request *p_request) {
				return MessageCommunicator::SendAsynchronous(p_message, m_rank[Task::TT_Master], p_tag, p_request);
			}

			bool ReceiveFromMasterAsynchronous(IMessage *p_message, int p_tag, MPI_Request *p_request) {
				return MessageCommunicator::ReceiveAsynchronous(p_message, m_rank[Task::TT_Master], p_tag, p_request);
			}
		};


		template <int T>
		class ChannelCommunicator
			: public MailboxCommunicator
		{
		public:
			ChannelCommunicator(void)
				: MailboxCommunicator(NULL)
			{ }

			ChannelCommunicator(int *p_rank)
				: MailboxCommunicator(p_rank)
			{ }

			ChannelCommunicator(Task *p_task)
				: MailboxCommunicator(p_task->Rank)
			{ }


			bool SendToCoordinator(IMessage *p_message) 
			{
				return MailboxCommunicator::SendToCoordinator(p_message, T);
			}

			bool ReceiveFromCoordinator(IMessage *p_message) {
				return MailboxCommunicator::ReceiveFromCoordinator(p_message, T);
			}

			bool SendToCoordinatorAsynchronous(IMessage *p_message, MPI_Request *p_request) {
				return MailboxCommunicator::SendToCoordinatorAsynchronous(p_message, T, p_request);
			}

			bool ReceiveFromCoordinatorAsynchronous(IMessage *p_message, MPI_Request *p_request) {
				return MailboxCommunicator::ReceiveFromCoordinatorAsynchronous(p_message, T, p_request);
			}


			bool SendToMaster(IMessage *p_message) {
				return MailboxCommunicator::SendToMaster(p_message, T);
			}

			bool ReceiveFromMaster(IMessage *p_message) {
				return MailboxCommunicator::ReceiveFromMaster(p_message, T);
			}

			bool SendToMasterAsynchronous(IMessage *p_message, MPI_Request *p_request) {
				return MailboxCommunicator::SendToMasterAsynchronous(p_message, T, p_request);
			}

			bool ReceiveFromMasterAsynchronous(IMessage *p_message, MPI_Request *p_request) {
				return MailboxCommunicator::ReceiveFromMasterAsynchronous(p_message, T, p_request);
			}


			bool Send(IMessage *p_message, int p_rank) {
				return MessageCommunicator::Send(p_message, p_rank, T);
			}

			bool Receive(IMessage *p_message, int p_rank) {
				return MessageCommunicator::Receive(p_message, p_rank, T);
			}

			bool Receive(IMessage *p_message, MPI_Status *p_statusOut) {
				return MessageCommunicator::Receive(p_message, T, p_statusOut);
			}

			bool SendAsynchronous(IMessage *p_message, int p_rank, MPI_Request *p_request) {
				return MessageCommunicator::SendAsynchronous(p_message, p_rank, T, p_request);
			}

			bool ReceiveAsynchronous(IMessage *p_message, int p_rank, MPI_Request *p_request, MPI_Status *p_status = MPI_STATUS_IGNORE) {
				return MessageCommunicator::ReceiveAsynchronous(p_message, p_rank, T, p_request, p_status);
			}
		};

		typedef ChannelCommunicator<MM_ChannelAny> AnyCommunicator;
		typedef ChannelCommunicator<MM_ChannelGlobal> GlobalCommunicator;
		typedef ChannelCommunicator<MM_ChannelMasterStatic> MasterCommunicator;
		typedef ChannelCommunicator<MM_ChannelWorkerStatic> WorkerCommunicator;
		typedef ChannelCommunicator<MM_ChannelWorkerStatic_0> WorkerCommunicator_0;
		typedef ChannelCommunicator<MM_ChannelWorkerStatic_1> WorkerCommunicator_1;
		typedef ChannelCommunicator<MM_ChannelPipelineStatic> PipelineCommunicator;
	}
}
