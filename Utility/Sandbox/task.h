#pragma once

#include <map>
#include <vector>
#include <cstdarg>

#include <boost/thread.hpp>
#include <boost/mpi.hpp>
namespace mpi = boost::mpi;

namespace Illumina
{
	namespace Core
	{
		enum MessageType 
		{
			Request = 0x01,
			Release = 0x02
		};

		struct Message
		{
			int Id;
			int Value[16];
			int Size;

			const void SetMessage(int p_id, ...)
			{
				Id = p_id;

				va_list list;

 				/*va_start(list, p_id);
 
				for (int nArg=0; nArg < nCount; nArg++)
					 lSum += va_arg(list, int);
 */
				va_end(list);
			}

			const std::string ToString(void) const 
			{
				return "Message.String";
			}

			const int MessageSize(void) 
			{
				return sizeof(int) * (Size + 1);
			}
		};

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
			int GetMasterRank(void) const { return Rank[TT_Master]; }
			void SetMasterRank(int p_nRank) { Rank[TT_Master] = p_nRank; }

			int GetCoordinatorRank(void) const { return Rank[TT_Coordinator]; }
			void SetCoordinatorRank(int p_nRank) { Rank[TT_Coordinator] = p_nRank; }

			int GetWorkerRank(void) const { return Rank[TT_Worker]; }
			void SetWorkerRank(int p_nRank) { Rank[TT_Worker] = p_nRank; }

			bool IsMaster(void) const { return Rank[TT_Worker] == Rank[TT_Master]; }
			bool IsCoordinator(void) const { return Rank[TT_Worker] == Rank[TT_Coordinator]; }
			bool IsWorker(void) const { return !(IsMaster() || IsCoordinator()); }

			bool Send(Task *p_destination, const Message &p_message)
			{
				//return MPI_SUCCESS == MPI_Send((void*)&p_message, sizeof(Message), MPI_BYTE, p_destination->GetWorkerRank(), p_message.CmdId, MPI_COMM_WORLD);
				return MPI_SUCCESS == MPI_Send((void*)&p_message, p_message.MessageSize(), MPI_BYTE, p_destination->GetWorkerRank(), p_message.CmdId, MPI_COMM_WORLD);
			}

			bool SendAsync(Task *p_destination, const Message &p_message, MPI_Request *p_request)
			{
				//MPI_Isend((void*)&p_message, sizeof(Message), MPI_BYTE, p_destination->GetWorkerRank(), p_message.CmdId, MPI_COMM_WORLD, p_request);
				MPI_Isend((void*)&p_message, p_message.MessageSize(), MPI_BYTE, p_destination->GetWorkerRank(), p_message.CmdId, MPI_COMM_WORLD, p_request);
				return IsRequestComplete(p_request);
			}

			bool Receive(Task *p_source, const Message &p_message)
			{
				MPI_Status status;
				return MPI_SUCCESS == MPI_Recv((void*)&p_message, p_message.MessageSize(), MPI_BYTE, p_source->GetWorkerRank(), MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				//return MPI_SUCCESS == MPI_Recv((void*)&p_message, sizeof(Message), MPI_BYTE, p_source->GetWorkerRank(), MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			}

			bool ReceiveAsync(Task *p_source, const Message &p_message, MPI_Request *p_request)
			{
				MPI_Irecv((void*)&p_message, p_message.MessageSize(), MPI_BYTE, p_source->GetWorkerRank(), MPI_ANY_TAG, MPI_COMM_WORLD, p_request);
				//MPI_Irecv((void*)&p_message, sizeof(Message), MPI_BYTE, p_source->GetWorkerRank(), MPI_ANY_TAG, MPI_COMM_WORLD, p_request);
				return IsRequestComplete(p_request);
			}

			bool IsRequestComplete(MPI_Request *p_request)
			{
				int flag;
				MPI_Status status;

				MPI_Test(p_request, &flag, &status);
				return (flag != 0);
			}
		};
	}
}
