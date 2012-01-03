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

		enum MessageType
		{
			MT_Request		= 0x01,
			MT_Release		= 0x02,
			MT_Terminate	= 0x03,
			MT_Register		= 0x04,
			MT_Completed	= 0x05,		
			MT_Synchronise	= 0x06,
			MT_Count
		};

		// Note that we should probably switch to templated messages, since many messages
		// are command ids without any arguments!

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

		struct RegisterMessage
			: public Message
		{
			RegisterMessage(void)
			{
				Id = MT_Register;
			}

			const std::string ToString(void) const
			{
				return "Message.Register";
			}
		};
		
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

		struct TerminateMessage
			: public Message
		{
			TerminateMessage(void)
			{
				Id = MT_Terminate;
			}

			const std::string ToString(void) const
			{
				return "Message.Terminate";
			}
		};

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
	}
}
