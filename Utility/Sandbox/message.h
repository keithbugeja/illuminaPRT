#pragma once

#include <boost/format.hpp>

namespace Illumina
{
	namespace Core
	{
		// We have three kind of messages here
		// 1. Messages between IdlePool from Master 
		// 2. Messages between Coordinator from Master 
		// 3. Messages between Coordinator and Workers

		enum MessageMedium
		{
			MM_ChannelAny = MPI_ANY_TAG,
			MM_ChannelGlobal,
			MM_ChannelBroadcast,
			MM_ChannelMasterStatic,
			MM_ChannelMasterDynamic,
			MM_ChannelWorkerStatic,
			MM_ChannelWorkerDynamic,
			MM_ChannelWorkerStatic_0,
			MM_ChannelWorkerDynamic_0,
			MM_ChannelWorkerStatic_1,
			MM_ChannelWorkerDynamic_1,
			MM_ChannelPipelineStatic,
			MM_ChannelPipelineDynamic,
			MM_ChannelUserBase = 0x1000
		};

		enum MessageType
		{
			MT_Request		= 0x01,
			MT_Release		= 0x02,
			MT_Terminate	= 0x03,
			MT_Register		= 0x04,
			MT_Completed	= 0x05,		
			MT_Synchronise	= 0x06,
			MT_Acknowledge	= 0x07,
			MT_Count
		};

		/*
		 * Base message class for fixed-length messages communication within communicator
		 */
		struct Message
		{
		public:
			int Id;
			int Value[4];

		public:
			virtual const std::string ToString(void) const 
			{
				return "Message.Base";
			}

			int MessageSize(void) const
			{
				return sizeof(Message);
			}
		};

		// RequestMessage -> Sent to idle workers to form a new task group
		struct RequestMessage 
			: public Message
		{
			RequestMessage(int p_groupId, int p_coordinatorId, int p_workerCount)
			{
				SetMessage(p_groupId, p_coordinatorId, p_workerCount);
			}

			void SetMessage(int p_groupId, int p_coordinatorId, int p_workerCount)
			{
				Id = MT_Request;

				Value[0] = p_groupId;
				Value[1] = p_coordinatorId;
				Value[2] = p_workerCount;
			}

			int GetGroupId(void) const { return Value[0]; }
			int GetCoordinatorId(void) const { return Value[1]; }
			int GetWorkerCount(void) const { return Value[2]; }

			const std::string ToString(void) const
			{
				return "Message.Request";
			}
		};
		
		// Release Message -> Sent to coordinator to request release of a number of workers
		struct ReleaseMessage 
			: public Message
		{
			ReleaseMessage(int p_releaseCount)
			{
				Id = MT_Release;
				SetMessage(p_releaseCount);
			}

			void SetMessage(int p_releaseCount) { Value[0] = p_releaseCount; }
			int GetReleaseCount(void) const { return Value[0]; }

			const std::string ToString(void) const
			{
				return "Message.Release";
			}
		};

		// CompletedMessage -> Sent to master by workers completing their tasks
		//	Senders are merged with idle group
		struct CompletedMessage
			: public Message
		{
			CompletedMessage(int p_groupId)
			{
				Id = MT_Completed;
				SetMessage(p_groupId);
			}

			void SetMessage(int p_groupId) { Value[0] = p_groupId; }
			int GetGroupId(void) const { return Value[0]; }

			const std::string ToString(void) const
			{
				return "Message.Completed"; 
			}
		};

		/*
		 * Templated message class
		 */
		template<int T>
		class TMessage
			: public Message
		{
		public:
			TMessage(void) { Id = T; }

			const std::string ToString(void) const { return "Message.Template"; }
		};

		// RegisterMessage -> Sent to coordinators by workers, to register themselves
		typedef TMessage<MT_Register> RegisterMessage;
		
		// TerminateMessage -> Sent to wokers by coordinator, to ask for idle transition
		typedef TMessage<MT_Terminate> TerminateMessage;

		// AcknowledgeMessage -> Sent to coordinators by workers, to signal completion of some task
		typedef TMessage<MT_Acknowledge> AcknowledgeMessage;

		typedef TMessage<MT_Synchronise> SynchroniseMessage;
	}
}
