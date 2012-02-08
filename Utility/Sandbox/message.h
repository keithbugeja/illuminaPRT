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
			MM_ChannelCoordinatorBase = 0x0500,
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
			MT_Direction	= 0x08,
			MT_Position		= 0x09,
			MT_Stream		= 0x0A,
			MT_Count
		};

		/*
		 * Base message class for message-class comms communication within communicator
		 */
		struct IMessage
		{
			enum Type {
				IMT_Variable,
				IMT_Fixed,
				IMT_Null
			};

			virtual IMessage::Type MessageType(void) const { return IMT_Null; }
			virtual int MessageId(void) const { return -1; }
			virtual int MessageSize(void) const { return 0; }
			virtual void *MessageBuffer(void) const { return NULL; }

			virtual const std::string ToString(void) const 
			{
				return "IMessage.Base";
			}
		};

		/*
		 * Base message class for fixed-length messages communication within communicator
		 */
		struct Message
			: public IMessage
		{
		public:
			int Id;
			int Value[4];

		public:
			IMessage::Type MessageType(void) const { return IMT_Fixed; }

			int MessageId(void) const
			{
				return Id;
			}

			int MessageSize(void) const
			{
				return sizeof(Message);
			}

			void *MessageBuffer(void) const
			{
				return (void*)this;
			}

			const std::string ToString(void) const 
			{
				return "Message.Base";
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

		// StreamMessage
		struct StreamMessage
			: public Message
		{
			StreamMessage(int p_port)
			{
				Id = MT_Stream;
				SetMessage(p_port);
			}

			void SetMessage(int p_port) { Value[0] = p_port; }
			int GetPort(void) const { return Value[0]; }

			const std::string ToString(void) const
			{
				return "Message.Stream"; 
			}
		};

		// DirectionMessage -> Sent to coord/workers
		//	
		struct DirectionMessage
			: public Message
		{
			DirectionMessage(int p_direction)
			{
				Id = MT_Direction;
				SetMessage(p_direction);
			}

			void SetMessage(int p_direction) { Value[0] = p_direction; }
			int GetDirection(void) const { return Value[0]; }

			const std::string ToString(void) const
			{
				return "Message.Direction"; 
			}
		};

		struct PositionMessage
			: public Message
		{
			PositionMessage(Vector3 &p_position)
			{
				Id = MT_Position;
				SetMessage(p_position);
			}

			void SetMessage(Vector3 &p_position) 
			{ 
				memcpy((float*)((void*)Value), p_position.Element, sizeof(float) * 3); 
			}

			Vector3 GetPosition(void) const 
			{ 
				Vector3 result;
				memcpy(result.Element, (float*)((void*)Value), sizeof(float) * 3);
				return result;
			}

			const std::string ToString(void) const
			{
				return "Message.Position"; 
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

		// SynchroniseMessage -> Used as a barrier prior to computation start
		typedef TMessage<MT_Synchronise> SynchroniseMessage;

		/*
		 * Variable length message
		 */
		class VarlenMessage
			: public IMessage
		{
		protected:
			void *m_messageBuffer;
			int m_messageSize;
		
		public:
			IMessage::Type MessageType(void) const { return IMT_Variable; }

		public:
			VarlenMessage(void) 
				: m_messageBuffer(NULL)
				, m_messageSize(0)
			{ }

			VarlenMessage(void *p_messageBuffer, int p_messageSize)
				: m_messageBuffer(p_messageBuffer)
				, m_messageSize(p_messageSize)
			{ }

			virtual ~VarlenMessage(void) { }

			int MessageId(void) const { return m_messageBuffer != NULL ? *((int*)m_messageBuffer) : -1; }
			int MessageSize(void) const { return m_messageSize; }
			void* MessageBuffer(void) const { return m_messageBuffer; }
			
			const std::string ToString(void) const { return "VarlenMessage.Base"; }
		};

		class RequestMessage
			: public VarlenMessage
		{
		public:
			struct t_messageStructure 
			{
				int Id;
				int CoordinatorId;
				int GroupId;
				char Config[1024];
			} *Data;
	
		protected:
			bool m_ownMessage;

		public:
			RequestMessage(void *p_messageBuffer)
				: m_ownMessage(false)
			{
				m_messageBuffer = Data = (t_messageStructure*)p_messageBuffer;
				m_messageSize = sizeof(t_messageStructure);
			}

			RequestMessage(int p_groupId, int p_coordinatorId, const std::string &p_config)
				: m_ownMessage(true)
			{
				m_messageBuffer = Data = new t_messageStructure();
				m_messageSize = sizeof(t_messageStructure);

				Data->Id = MT_Request;

				SetMessage(p_groupId, p_coordinatorId, p_config);
			}

			~RequestMessage(void) 
			{ 
				if (m_ownMessage) delete Data; 
			}

			void SetMessage(int p_groupId, int p_coordinatorId, const std::string &p_config)
			{
				Data->GroupId = p_groupId;
				Data->CoordinatorId = p_coordinatorId;
				memcpy(Data->Config, p_config.c_str(), p_config.size());
			}

			int GetGroupId(void) const { return Data->GroupId; }
			int GetCoordinatorId(void) const { return Data->CoordinatorId; }
			char *GetConfig(void) const { return Data->Config; }

			const std::string ToString(void) const { return "VarlenMessage.Request"; }
		};
	}
}
