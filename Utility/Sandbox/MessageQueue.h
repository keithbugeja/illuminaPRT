#pragma once
#include <queue>
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
class MessageIdentifiers
{
public:
	enum Identifiers
	{
		ID_Resource_Register		= 0x0001,
		ID_Resource_Unregister		= 0x0002,
		ID_Resource_Terminate		= 0x0003,
		ID_Worker_Register			= 0x0004,
		ID_Worker_Ready				= 0x0005,
		ID_Coordinator_Reject		= 0x0006,
		ID_Coordinator_Accept		= 0x0007,
		ID_Coordinator_Sync			= 0x0008,
		ID_Coordinator_Unregister	= 0x0009
	};
};
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
struct Message_Header
{
	int MessageID;
};
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
struct Message_Controller_Resource_Register
{
	int MessageID;
	int CoordinatorID;
	int Size;
	char String[2048];
};

struct Message_Controller_Resource_Unregister
{
	int MessageID;
	int Size;
	short Resources[1024];
};

typedef Message_Header Message_Controller_Resource_Terminate;
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
struct Message_Coordinator_Worker_Accept
{
	int MessageID;
	int Size;
	char String[2048];
};

struct Message_Coordinator_Worker_Sync
{
	int  MessageID;
	bool Unregister;
};

typedef Message_Header Message_Worker_Coordinator_Register;
typedef Message_Header Message_Worker_Coordinator_Ready;
typedef Message_Header Message_Coordinator_Worker_Unregister;
typedef Message_Header Message_Coordinator_Worker_Reject;
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
struct ResourceMessage
{
	int	OriginID;
	int RecipientID;
	int Tag;
	int Command;
	int Size;
	unsigned char *Content;

	ResourceMessage(int p_nOriginID, int p_nRecipientID, int p_nTag, int p_nCommand, int p_nMessageSize, unsigned char *p_pMessage)
		: OriginID(p_nOriginID)
		, RecipientID(p_nRecipientID)
		, Tag(p_nTag)
		, Command(p_nCommand)
		, Size(p_nMessageSize)
	{
		Content = new unsigned char[Size];
		memcpy(Content, p_pMessage, Size);
	}

	ResourceMessage(void)
		: Size(0)
		, Content(NULL)
	{ }

	~ResourceMessage(void)
	{
		if (Content != NULL)
			delete[] Content;
	}
};
//----------------------------------------------------------------------------------------------
typedef std::queue<ResourceMessage*> ResourceMessageQueue;
//----------------------------------------------------------------------------------------------
