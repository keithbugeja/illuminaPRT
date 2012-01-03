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

		protected:
			// Blocking send/receive
			bool SendMPI(const Message &p_message, int p_rank)
			{
				return MPI_SUCCESS == MPI_Send((void*)&p_message, p_message.MessageSize(), MPI_BYTE, p_rank, p_message.Id, MPI_COMM_WORLD);
			}

			bool ReceiveMPI(const Message &p_message, int p_rank, int p_cmdId = MPI_ANY_TAG)
			{
				MPI_Status status;
				return MPI_SUCCESS == MPI_Recv((void*)&p_message, p_message.MessageSize(), MPI_BYTE, this->GetCoordinatorRank(), p_cmdId, MPI_COMM_WORLD, &status);
			}

			// Asynchronous send/receive 
			bool SendMPIAsync(const Message &p_message, int p_rank, MPI_Request *p_request)
			{
				MPI_Isend((void*)&p_message, p_message.MessageSize(), MPI_BYTE, p_rank, p_message.Id, MPI_COMM_WORLD, p_request);
				return IsRequestComplete(p_request);
			}

			bool ReceiveMPIAsync(const Message &p_message, int p_rank, MPI_Request *p_request, int p_cmdId = MPI_ANY_TAG)
			{
				MPI_Irecv((void*)&p_message, p_message.MessageSize(), MPI_BYTE, p_rank, p_cmdId, MPI_COMM_WORLD, p_request);
				return IsRequestComplete(p_request);
			}

		public:
			Task(int p_workerRank = -1, int p_masterRank = -1, int p_coordinatorRank = -1)
			{
				Rank[TT_Worker] = p_workerRank;
				Rank[TT_Master] = p_masterRank;
				Rank[TT_Coordinator] = p_coordinatorRank;
			}

			int GetMasterRank(void) const { return Rank[TT_Master]; }
			void SetMasterRank(int p_nRank) { Rank[TT_Master] = p_nRank; }

			int GetCoordinatorRank(void) const { return Rank[TT_Coordinator]; }
			void SetCoordinatorRank(int p_nRank) { Rank[TT_Coordinator] = p_nRank; }

			int GetWorkerRank(void) const { return Rank[TT_Worker]; }
			void SetWorkerRank(int p_nRank) { Rank[TT_Worker] = p_nRank; }

			bool IsMaster(void) const { return Rank[TT_Worker] >= 0 && Rank[TT_Worker] == Rank[TT_Master]; }
			bool IsCoordinator(void) const { return Rank[TT_Worker] >= 0 && Rank[TT_Worker] == Rank[TT_Coordinator]; }
			bool IsWorker(void) const { return !(IsMaster() || IsCoordinator()); }

			// Blocking send/receive to/from communicator
			inline bool SendToCoordinator(const Message &p_message) { 
				return SendMPI(p_message, this->GetCoordinatorRank()); 
			}

			inline bool ReceiveFromCoordinator(const Message &p_message, int p_cmdId = MPI_ANY_TAG) {
				return ReceiveMPI(p_message, this->GetCoordinatorRank(), p_cmdId);
			}

			// Asynchronous send/receive to/from communicator
			bool SendToCoordinatorAsync(const Message &p_message, MPI_Request *p_request) {
				return SendMPIAsync(p_message, this->GetCoordinatorRank(), p_request);
			}

			bool ReceiveFromCoordinatorAsync(const Message &p_message, MPI_Request *p_request, int p_cmdId = MPI_ANY_TAG) {
				return ReceiveMPIAsync(p_message, this->GetCoordinatorRank(), p_request, p_cmdId);
			}

			// Blocking send/receive to/from communicator
			inline bool SendToMaster(const Message &p_message) { 
				return SendMPI(p_message, this->GetMasterRank()); 
			}

			inline bool ReceiveFromMaster(const Message &p_message, int p_cmdId = MPI_ANY_TAG) {
				return ReceiveMPI(p_message, this->GetMasterRank(), p_cmdId);
			}

			// Asynchronous send/receive to/from communicator
			bool SendToMasterAsync(const Message &p_message, MPI_Request *p_request) {
				return SendMPIAsync(p_message, this->GetMasterRank(), p_request);
			}

			bool ReceiveFromMasterAsync(const Message &p_message, MPI_Request *p_request, int p_cmdId = MPI_ANY_TAG) {
				return ReceiveMPIAsync(p_message, this->GetMasterRank(), p_request, p_cmdId);
			}

			// Blocking send/receive
			bool Send(Task *p_destination, const Message &p_message)
			{
				return MPI_SUCCESS == MPI_Send((void*)&p_message, p_message.MessageSize(), MPI_BYTE, p_destination->GetWorkerRank(), p_message.Id, MPI_COMM_WORLD);				
			}

			bool Receive(Task *p_source, const Message &p_message, int p_cmdId = MPI_ANY_TAG)
			{
				MPI_Status status;
				return MPI_SUCCESS == MPI_Recv((void*)&p_message, p_message.MessageSize(), MPI_BYTE, p_source->GetWorkerRank(), p_cmdId, MPI_COMM_WORLD, &status);
			}


			// Asynchronous send/receive
			bool SendAsync(Task *p_destination, const Message &p_message, MPI_Request *p_request)
			{
				return (MPI_SUCCESS == MPI_Isend((void*)&p_message, p_message.MessageSize(), MPI_BYTE, p_destination->GetWorkerRank(), p_message.Id, MPI_COMM_WORLD, p_request));
			}

			bool ReceiveAsync(Task *p_source, const Message &p_message, MPI_Request *p_request, int p_cmdId = MPI_ANY_TAG)
			{
				return (MPI_SUCCESS == MPI_Irecv((void*)&p_message, p_message.MessageSize(), MPI_BYTE, p_source->GetWorkerRank(), p_cmdId, MPI_COMM_WORLD, p_request));
			}

			// Blocking receive from any peer
			bool ReceiveAny(const Message &p_message, MPI_Status *p_status, int p_cmdId = MPI_ANY_TAG)
			{
				return (MPI_SUCCESS == MPI_Recv((void*)&p_message, p_message.MessageSize(), MPI_BYTE, MPI_ANY_SOURCE, p_cmdId, MPI_COMM_WORLD, p_status));
			}

			bool ReceiveAnyAsync(const Message &p_message, MPI_Request *p_request, int p_cmdId = MPI_ANY_TAG)
			{
				return (MPI_SUCCESS == MPI_Irecv((void*)&p_message, p_message.MessageSize(), MPI_BYTE, MPI_ANY_SOURCE, p_cmdId, MPI_COMM_WORLD, p_request));
			}

			// Request completion test
			bool IsRequestComplete(MPI_Request *p_request)
			{
				int flag;

				MPI_Test(p_request, &flag, MPI_STATUSES_IGNORE);

				return (flag != 0);
			}

			bool IsRequestComplete(MPI_Request *p_request, MPI_Status *p_statusOut)
			{
				int flag; 
				
				MPI_Test(p_request, &flag, p_statusOut);
				
				return (flag != 0);
			}

			std::string ToString(void) const
			{
				std::string strOut = boost::str(boost::format("[master = %d, coordinator = %d, worker = %d]") 
					% this->GetMasterRank()
					% this->GetCoordinatorRank() 
					% this->GetWorkerRank());
				
				return strOut;
			}
		};
	}
}
