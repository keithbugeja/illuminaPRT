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

		template <int T>
		class GeneralCommunicator
			: public TaskCommunicator
		{
		protected:
			int *m_rank;

		public:
			GeneralCommunicator(void)
				: m_rank(NULL)
			{ }

			GeneralCommunicator(int *p_rank)
				: m_rank(p_rank)
			{ }

			GeneralCommunicator(Task *p_task)
				: m_rank(p_task->Rank)
			{ }

			bool SendToCoordinator(const Message &p_message) {
				return TaskCommunicator::Send((void*)&p_message, p_message.MessageSize(), m_rank[Task::TT_Coordinator], T);
			}

			bool ReceiveFromCoordinator(const Message &p_message) {
				return TaskCommunicator::Receive((void*)&p_message, p_message.MessageSize(), m_rank[Task::TT_Coordinator], T);
			}

			bool SendToCoordinatorAsync(const Message &p_message, MPI_Request *p_request) {
				return TaskCommunicator::SendAsychronous((void*)&p_message, p_message.MessageSize(), m_rank[Task::TT_Coordinator], T, p_request);
			}

			bool ReceiveFromCoordinatorAsync(const Message &p_message, MPI_Request *p_request) {
				return TaskCommunicator::ReceiveAsychronous((void*)&p_message, p_message.MessageSize(), m_rank[Task::TT_Coordinator], T, p_request);
			}

			bool SendToMaster(const Message &p_message) {
				return TaskCommunicator::Send((void*)&p_message, p_message.MessageSize(), m_rank[Task::TT_Master], T);
			}

			bool ReceiveFromMaster(const Message &p_message) {
				return TaskCommunicator::Receive((void*)&p_message, p_message.MessageSize(), m_rank[Task::TT_Master], T);
			}

			bool SendToMasterAsync(const Message &p_message, MPI_Request *p_request) {
				return TaskCommunicator::SendAsychronous((void*)&p_message, p_message.MessageSize(), m_rank[Task::TT_Master], T, p_request);
			}

			bool ReceiveFromMasterAsync(const Message &p_message, MPI_Request *p_request) {
				return TaskCommunicator::ReceiveAsychronous((void*)&p_message, p_message.MessageSize(), m_rank[Task::TT_Master], T, p_request);
			}

			bool Send(const Message &p_message, int p_rank) {
				return TaskCommunicator::Send((void*)&p_message, p_message.MessageSize(), p_rank, T);
			}

			bool Receive(const Message &p_message, int p_rank) {
				return TaskCommunicator::Receive((void*)&p_message, p_message.MessageSize(), p_rank, T);
			}

			bool Receive(const Message &p_message, MPI_Status *p_statusOut) {
				return TaskCommunicator::Receive((void*)&p_message, p_message.MessageSize(), MPI_ANY_SOURCE, T, p_statusOut);
			}

			bool SendAsync(const Message &p_message, int p_rank, MPI_Request *p_request) {
				return TaskCommunicator::SendAsychronous((void*)&p_message, p_message.MessageSize(), p_rank, T, p_request);
			}

			bool ReceiveAsync(const Message &p_message, int p_rank, MPI_Request *p_request, MPI_Status *p_status = MPI_STATUS_IGNORE) {
				return TaskCommunicator::ReceiveAsynchronous((void*)&p_message, p_message.MessageSize(), p_rank, T, p_request, p_status);
			}
		};

		typedef GeneralCommunicator<MM_ControlStatic> ControlCommunicator;
		typedef GeneralCommunicator<MM_ContextDynamic> ContextCommunicator;
	}
}
