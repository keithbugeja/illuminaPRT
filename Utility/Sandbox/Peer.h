//----------------------------------------------------------------------------------------------
//	Filename:	Peer.h
//	Author:		Keith Bugeja
//	Date:		21/09/2013
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <map>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/lexical_cast.hpp>

#include <System/Platform.h>
#include <Maths/Maths.h>
#include <External/Compression/Compression.h>

#include <MessageIdentifiers.h>
#include <RakPeerInterface.h>
#include <ConnectionGraph2.h>
#include <BitStream.h>

#pragma comment(lib,"raknetdll_x64.lib")
//----------------------------------------------------------------------------------------------
class HostId
{
protected:
	Int64 m_hostId;

public:
	static Int64 MakeHostId(const std::string &p_strAddress, unsigned short p_nPort) 
	{
		unsigned long ulIPv4 = 
			boost::asio::ip::address_v4::from_string(p_strAddress.c_str()).to_ulong();

		return MakeInt64(ulIPv4, p_nPort);
	}

	static Int64 MakeHostId(unsigned long p_nAddress, unsigned short p_nPort)
	{
		return MakeInt64(p_nAddress, p_nPort);
	}

	static Int64 MakeHostId(RakNet::SystemAddress *p_pSystemAddress)
	{
		return MakeHostId(p_pSystemAddress->ToString(false), p_pSystemAddress->GetPort());
	}

public:
	HostId(const std::string &p_strAddress, unsigned short p_nPort)
		: m_hostId(HostId::MakeHostId(p_strAddress, p_nPort))
	{ }

	HostId(unsigned long p_nAddress, unsigned short p_nPort)
		: m_hostId(MakeHostId(p_nAddress, p_nPort))
	{ }

	HostId(unsigned long long p_ullHostId)
		: m_hostId((Int64)p_ullHostId)
	{ }

	HostId(const HostId &p_hostId)
		: m_hostId(p_hostId.m_hostId)
	{ }

	HostId(void)
		: m_hostId(0)
	{ }

	bool operator==(const HostId &p_hostId) { return m_hostId == p_hostId.m_hostId; }
	bool operator!=(const HostId &p_hostId) { return m_hostId != p_hostId.m_hostId; }

	unsigned short GetPort(void)
	{
		return (unsigned short)GetLoWord(m_hostId);
	}

	unsigned long GetIPv4(void)
	{
		return (unsigned long)GetHiWord(m_hostId);
	}

	HostId &operator=(unsigned long long p_ullHostId)
	{
		m_hostId = (Int64)p_ullHostId;
		return *this;
	}

	unsigned long long GetHash(void)
	{
		return (unsigned long long)m_hostId;
	}

	std::string ToString(void)
	{
		std::stringstream result;
		result << std::hex << "[" << (unsigned long long)m_hostId << "]" << std::dec;
		return result.str();
	}

	std::string ToIPv4String(void)
	{
		unsigned long IPv4 = GetIPv4();
		unsigned char *pOctet = (unsigned char*)&IPv4;

		std::stringstream result;
		result << (int)pOctet[3] << "." 
			<< (int)pOctet[2] << "."
			<< (int)pOctet[1] << "."
			<< (int)pOctet[0];

		return result.str();
	}
};
//----------------------------------------------------------------------------------------------
class SparseVectorClock
{
public:
	enum Comparison
	{
		LessThan,
		GreaterThan,
		NotEqual,
		Equal
	};

protected:
	std::map<unsigned long long, unsigned int> m_clockMap; 
	unsigned long long m_peerHash;
	HostId m_peerId;

public:
	SparseVectorClock(void) 
	{
		m_peerId = m_peerId.MakeHostId("127.0.0.1", 666);
		m_peerHash = m_peerId.GetHash();
		m_clockMap[m_peerHash] = 0;
	}

	SparseVectorClock(HostId &p_peerId)
		: m_peerId(p_peerId)
		, m_peerHash(p_peerId.GetHash())
	{ 
		m_clockMap[m_peerHash] = 0;
	}

	SparseVectorClock(SparseVectorClock &p_clock) 
		: m_peerId(p_clock.m_peerId)
		, m_peerHash(p_clock.m_peerHash)
		, m_clockMap(p_clock.m_clockMap)
	{ }

	~SparseVectorClock(void) { }

	SparseVectorClock& operator=( SparseVectorClock& p_clock)
	{
		m_peerHash = p_clock.m_peerHash;
		m_peerId = p_clock.m_peerId;

		m_clockMap.clear();

		for (auto entry : p_clock.m_clockMap) {
			m_clockMap[entry.first] = entry.second;
		}

		return *this;
	}

	void Reset(HostId &p_peerId)
	{
		m_peerId = p_peerId;
		m_peerHash = m_peerId.GetHash();
		
		m_clockMap.clear();
		m_clockMap[m_peerHash] = 0;
	}

	int GetValue(void) 
	{ 
		return m_clockMap[m_peerHash]; 
	}

	Comparison Compare(const SparseVectorClock &p_clock)
	{
		bool equal = true,
			lessThan = true,
			greaterThan = true;

		for (auto pairIter : m_clockMap)
		{
			if (p_clock.m_clockMap.find(pairIter.first) != p_clock.m_clockMap.end())
			{
				int value = p_clock.m_clockMap.at(pairIter.first);

				if (pairIter.second < value)
				{
					greaterThan = equal = false;
				}
				if (pairIter.second > value)
				{
					lessThan = equal = false;
				}
			}
			else if (pairIter.second != 0)
			{
				lessThan = equal = false;
			}
		}

		for (auto pairIter : p_clock.m_clockMap)
		{
			if (m_clockMap.find(pairIter.first) == m_clockMap.end() &&
				pairIter.second != 0)
			{
				greaterThan = equal = false;
			}
		}

		if (equal) return Equal;
		else if (greaterThan && !lessThan) return GreaterThan;
		else if (lessThan && !greaterThan) return LessThan;
		else return NotEqual;
	}

	void Supremum(SparseVectorClock &p_clock) 
	{
		for (auto pairIter : p_clock.m_clockMap)
		{
			m_clockMap[pairIter.first] = Maths::Max(
				m_clockMap[pairIter.first],
				pairIter.second);
		}
	}
	
	int Tick(void) 
	{
		return m_clockMap[m_peerHash] = m_clockMap[m_peerHash] + 1;
	}

	int Send(void)
	{
		return Tick();
	}

	void Receive(SparseVectorClock &p_clock)
	{
		Supremum(p_clock);

		Tick();
	}

	bool IsTimestamp(void) const {
		return m_peerHash == 0;
	}

	bool IsClock(void) const {
		return m_peerHash != 0;
	}

	void WriteToBitStream(RakNet::BitStream &p_bitstream)
	{
		std::cout << "---> [P2P Subsystem] :: Write Timestamp :: " << ToString() << std::endl;

		std::cout << "---> Size :: [" << m_clockMap.size() << "]" << std::endl;
		p_bitstream.Write((int)m_clockMap.size());
		// std::cout << "---> Hash :: [" << std::hex <<  m_peerHash << std::dec << "]" << std::endl;
		// p_bitstream.Write(m_peerHash);

		for (auto clockEntry : m_clockMap)
		{
			std::cout << "---> Key :: [" << std::hex << clockEntry.first << std::dec << "]" << std::endl;
			p_bitstream.Write(clockEntry.first);
			std::cout << "---> Value :: [" << std::hex << clockEntry.second << std::dec << "]" << std::endl;
			p_bitstream.Write(clockEntry.second);
		}
	}

	void ReadFromBitStream(RakNet::BitStream &p_bitstream)
	{
		unsigned long long hostIdHash;
		unsigned int counter = 0;
		int clockEntries = 0;

		m_peerId = m_peerHash = 0;
		m_clockMap.clear();

		// Read number of entries in clock
		p_bitstream.Read(clockEntries);
		std::cout << "---> Size :: [" << clockEntries << "]" << std::endl;
		// p_bitstream.Read(m_peerHash);
		// std::cout << "---> Hash :: [" << m_peerHash << "]" << std::endl;
		// m_peerId = m_peerHash;

		for (int entry = 0; entry < clockEntries; entry++)
		{
			p_bitstream.Read(hostIdHash);
			std::cout << "---> Key :: [" << std::hex << hostIdHash << std::dec << "]" << std::endl;
			p_bitstream.Read(counter);
			std::cout << "---> Value :: [" << std::hex << counter << std::dec << "]" << std::endl;

			m_clockMap[hostIdHash] = counter;
		}

		std::cout << "---> [P2P Subsystem] :: Read Timestamp :: " << ToString() << std::endl;
	}

	std::string ToString(void)
	{
		std::stringstream result; 
		HostId hostId;

		result << "PeerId : [" << m_peerId.ToString() << "] :: [";

		for (auto pairIter : m_clockMap)
		{
			hostId = pairIter.first;
			result << "<" << hostId.ToString() << " :: " << pairIter.second << "> ";
		}

		result << "]";

		return result.str();
	} 
};
//----------------------------------------------------------------------------------------------
class Peer
{
	RakNet::RakPeerInterface *m_pRakPeer;

	HostId m_localHostId;

	SparseVectorClock m_clock;

	int m_nMaxConnections,
		m_nMaxIncomingConnections,
		m_nBroadcastPort,
		m_nListenPort;

public:
	Peer(int p_nPort, int p_nBroadcastPort, int p_nMaxConnections, int p_nMaxIncoming)
		: m_nListenPort(p_nPort)
		, m_nBroadcastPort(p_nBroadcastPort)
		, m_nMaxConnections(p_nMaxConnections)
		, m_nMaxIncomingConnections(p_nMaxIncoming)
	{ }

	Peer(void)
		: m_nListenPort(0)
		, m_nBroadcastPort(0)
		, m_nMaxConnections(0)
		, m_nMaxIncomingConnections(0)
	{ }
	
	//----------------------------------------------------------------------------------------------
	HostId GetHostId(void) const { return m_localHostId; }
	//----------------------------------------------------------------------------------------------
	int GetIncomingPort(void) const { return m_nListenPort; }
	int GetOutgoingPort(void) const { return m_nBroadcastPort; }
	//----------------------------------------------------------------------------------------------
	SparseVectorClock& GetClock(void) { return m_clock; }
	//----------------------------------------------------------------------------------------------
	void Configure(int p_nPort, int p_nBroadcastPort, int p_nMaxConnections, int p_nMaxIncoming)
	{
		m_nListenPort = p_nPort;
		m_nBroadcastPort = p_nBroadcastPort;
		m_nMaxConnections = p_nMaxConnections;
		m_nMaxIncomingConnections = p_nMaxIncoming;
	}
	//----------------------------------------------------------------------------------------------
	bool Initialise(void)
	{
		m_pRakPeer = RakNet::RakPeerInterface::GetInstance();

		RakNet::SocketDescriptor socketDescriptor(m_nListenPort, 0);
		socketDescriptor.socketFamily = AF_INET;
		
		RakNet::StartupResult sr = m_pRakPeer->Startup(m_nMaxConnections, &socketDescriptor, 1, THREAD_PRIORITY_NORMAL);
		
		if (sr == RakNet::RAKNET_STARTED)
		{
			m_pRakPeer->SetMaximumIncomingConnections(m_nMaxIncomingConnections);
			
			std::cout << "Peer :: Peer bound to the following addresses :" << std::endl;
			for (int addrIndex = 0; addrIndex < m_pRakPeer->GetNumberOfAddresses(); addrIndex++)
				std::cout << "[" << addrIndex << "] :: " << m_pRakPeer->GetLocalIP(addrIndex) << std::endl;
		
			m_localHostId = HostId::MakeHostId(std::string(m_pRakPeer->GetLocalIP(0)), m_nListenPort);
			
			m_clock.Reset(m_localHostId);
			std::cout << "Peer :: Peer initialised logical clock : [" << m_clock.ToString() << "]" << std::endl;
			
			return true;
		}

		std::cerr << "Peer :: RakNet failed to start! ERR => [" << sr << "]" << std::endl; 
		return false;
	}
	//----------------------------------------------------------------------------------------------	
	void Shutdown(void)
	{
		RakNet::RakPeerInterface::DestroyInstance(m_pRakPeer);
	}
	//----------------------------------------------------------------------------------------------
	bool Ping(const std::string p_strRemoteAddress, unsigned int p_nRemotePort, int p_nTimeout, HostId &p_hostId) //, std::vector<Neighbour> &p_neighbourList)
	{
		RakNet::Packet *pPacket;
		bool bResponse = false;

		// Ping host(s)
		std::cout << "Peer :: Pinging [" << p_strRemoteAddress << " : " << p_nRemotePort << "] ..." << std::endl;
		m_pRakPeer->Ping(p_strRemoteAddress.c_str(), (unsigned short)p_nRemotePort, false);
		
		// Set response deadline
		int deadline = RakNet::GetTimeMS() + p_nTimeout;

		// Get response
		while (RakNet::GetTimeMS() < deadline)
		{
			pPacket = m_pRakPeer->Receive();

			if (pPacket == NULL) 
			{
				boost::thread::yield();
				continue;
			}
			else
			{
				if (pPacket->data[0] == ID_UNCONNECTED_PONG)
				{
					bResponse = true;

					RakNet::TimeMS checkpoint; 
					RakNet::BitStream bitStream(pPacket->data, pPacket->length, false);
					bitStream.IgnoreBytes(1); bitStream.Read(checkpoint);

					HostId host(RakNet::SystemAddress::ToInteger(pPacket->systemAddress), p_nRemotePort);
					long latency = Maths::Min<int>(RakNet::GetTimeMS() - checkpoint, p_nTimeout);
					
					std::cout << "Peer :: Received PONG from " << host.ToString() << " :: Latency [" << latency << "ms]" << std::endl;
				}
				else if (pPacket->data[0] == ID_UNCONNECTED_PING || 
					pPacket->data[0] == ID_UNCONNECTED_PING_OPEN_CONNECTIONS ||
					pPacket->data[0] == ID_CONNECTED_PING || 
					pPacket->data[0] == ID_CONNECTED_PONG)
				{
					std::cout << "Peer :: Received PING. Discarding..." << std::endl;
				}

				m_pRakPeer->DeallocatePacket(pPacket);
			}
		}

		return bResponse;
	}
	//----------------------------------------------------------------------------------------------
	bool Connect(HostId p_hostId, int p_nTimeout = 0)
	{
		std::string IPv4 = p_hostId.ToIPv4String();
		unsigned short port = p_hostId.GetPort();

		RakNet::SystemAddress address;
		address.FromStringExplicitPort(IPv4.c_str(), port);
		
		RakNet::ConnectionAttemptResult car = m_pRakPeer->Connect(IPv4.c_str(), port, NULL, 0);
		
		if (p_nTimeout == 0)
			return (car == RakNet::CONNECTION_ATTEMPT_STARTED);

		int deadline = RakNet::GetTimeMS() + p_nTimeout;
		
		while (RakNet::GetTimeMS() < deadline)
		{
			RakNet::ConnectionState cs = m_pRakPeer->GetConnectionState(address);
			switch(cs)
			{
				case RakNet::IS_CONNECTED: 
					return true;

				case RakNet::IS_CONNECTING:
					continue;
			}
			
			boost::thread::yield();
		}

		return false;
	}
	//----------------------------------------------------------------------------------------------
	void Disconnect(HostId p_hostId)
	{
		RakNet::SystemAddress address;
		address.FromStringExplicitPort(p_hostId.ToIPv4String().c_str(), p_hostId.GetPort());

		RakNet::ConnectionState cs = m_pRakPeer->GetConnectionState(address);
		if (cs == RakNet::ConnectionState::IS_CONNECTED)
			m_pRakPeer->CloseConnection(address, false);
		else if (cs == RakNet::ConnectionState::IS_CONNECTING)
			m_pRakPeer->CancelConnectionAttempt(address);
	}
	//----------------------------------------------------------------------------------------------
	bool IsConnected(HostId p_hostId)
	{
		RakNet::SystemAddress address;
		address.FromStringExplicitPort(p_hostId.ToIPv4String().c_str(), p_hostId.GetPort());
		
		RakNet::ConnectionState cs = m_pRakPeer->GetConnectionState(address);
		return (cs == RakNet::ConnectionState::IS_CONNECTED);
	}
	//----------------------------------------------------------------------------------------------
	RakNet::ConnectionState GetConnectionState(HostId p_hostId)
	{
		RakNet::SystemAddress address;
		address.FromStringExplicitPort(p_hostId.ToIPv4String().c_str(), p_hostId.GetPort());
		
		return m_pRakPeer->GetConnectionState(address);
	}
	//----------------------------------------------------------------------------------------------
	bool GetConnectionList(std::vector<HostId> &p_hostConnectionList) 
	{
		RakNet::SystemAddress* addressList[32];
		unsigned short addressCount;
		
		m_pRakPeer->GetConnectionList((RakNet::SystemAddress*)addressList, &addressCount);
		
		for (; addressCount > 0; addressCount--)
			p_hostConnectionList.push_back(HostId::MakeHostId(addressList[addressCount]));

		return !p_hostConnectionList.empty();
	}
	//----------------------------------------------------------------------------------------------
	bool SendIddStream(HostId p_hostId, unsigned char p_streamId, RakNet::BitStream &p_bitStream, unsigned char p_ucIdOffset = 0)
	{
		//std::cout << "Peer :: SendIddData :: Stream id = [" << (int)p_streamId << "]" << std::endl;

		RakNet::BitStream bitStream;
		bitStream.Write((unsigned char)(ID_USER_PACKET_ENUM + p_ucIdOffset));
		bitStream.Write((unsigned char)p_streamId);
		bitStream.Write(p_bitStream);

		RakNet::SystemAddress address;
		address.FromStringExplicitPort(p_hostId.ToIPv4String().c_str(), p_hostId.GetPort());

		bool result = m_pRakPeer->Send(
			&bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, address, false);

		return result;
	}
	//----------------------------------------------------------------------------------------------
	bool ReceiveIddStream(RakNet::BitStream &p_bitStream, unsigned char &p_streamId, HostId &p_hostId, unsigned char p_ucIdOffset = 0)
	{
		RakNet::Packet *pPacket;

		if (pPacket = m_pRakPeer->Receive())
		{
			//std::cout << "Peer :: ReceivedIddData :: Header [" << (int)pPacket->data[0] << "]" << std::endl;
			
			// Have to discard uninteresting packets!
			if (pPacket->data[0] < ID_USER_PACKET_ENUM)
			{
				m_pRakPeer->DeallocatePacket(pPacket);
				return false;
			} 
			else if (pPacket->data[0] == ID_USER_PACKET_ENUM + p_ucIdOffset)
			{
				// Assign data
				p_bitStream.Reset();
				p_bitStream.Write((const char*)(pPacket->data + 2), pPacket->length - 2);
				p_hostId = HostId::MakeHostId(pPacket->systemAddress.ToString(false), pPacket->systemAddress.GetPort());
				p_streamId = pPacket->data[1];

				//std::cout << "Peer :: ReceivedIddData :: Stream id = [" << (int)p_streamId << "]" << std::endl;

				m_pRakPeer->DeallocatePacket(pPacket);
				return true;
			}
			else 
			{
				m_pRakPeer->PushBackPacket(pPacket, false);
				return false;
			}
		}

		return false;
	}
	//----------------------------------------------------------------------------------------------
	bool SendStream(HostId p_hostId, RakNet::BitStream &p_bitStream, unsigned char p_ucIdOffset = 0)
	{
		RakNet::BitStream bitStream;
		bitStream.Write((unsigned char)(ID_USER_PACKET_ENUM + p_ucIdOffset));
		bitStream.Write(p_bitStream);

		RakNet::SystemAddress address;
		address.FromStringExplicitPort(p_hostId.ToIPv4String().c_str(), p_hostId.GetPort());

		bool result = m_pRakPeer->Send(
			&bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, address, false);

		return result;
	}
	//----------------------------------------------------------------------------------------------
	bool ReceiveStream(RakNet::BitStream &p_bitStream, HostId &p_hostId, unsigned char p_ucIdOffset = 0)
	{
		RakNet::Packet *pPacket;

		if (pPacket = m_pRakPeer->Receive())
		{
			//std::cout << "Peer :: ReceiveData :: Header [" << (int)pPacket->data[0] << "]" << std::endl;
			
			// Have to discard uninteresting packets!
			if (pPacket->data[0] < ID_USER_PACKET_ENUM)
			{
				m_pRakPeer->DeallocatePacket(pPacket);
				return false;
			}
			else if (pPacket->data[0] == ID_USER_PACKET_ENUM + p_ucIdOffset)
			{
				// Assign data
				p_bitStream.Reset();
				p_bitStream.Write((const char*)(pPacket->data + 1), pPacket->length - 1);
				p_hostId = HostId::MakeHostId(pPacket->systemAddress.ToString(false), pPacket->systemAddress.GetPort());

				m_pRakPeer->DeallocatePacket(pPacket);
				return true;
			}
			else 
			{
				m_pRakPeer->PushBackPacket(pPacket, false);
				return false;
			}
		}

		return false;
	}
	//----------------------------------------------------------------------------------------------
	bool SendData(HostId p_hostId, const char *p_pData, int p_nLength, unsigned char p_ucIdOffset = 0)
	{
		RakNet::BitStream bitStream;
		bitStream.Write((unsigned char)(ID_USER_PACKET_ENUM + p_ucIdOffset));
		bitStream.Write(p_pData, p_nLength);

		RakNet::SystemAddress address;
		address.FromStringExplicitPort(p_hostId.ToIPv4String().c_str(), p_hostId.GetPort());

		bool result = m_pRakPeer->Send(
			&bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, address, false);

		return result;
	}
	//----------------------------------------------------------------------------------------------
	bool ReceiveData(std::vector<unsigned char> &p_data, HostId &p_hostId, unsigned char p_ucIdOffset = 0)
	{
		RakNet::Packet *pPacket;

		if (pPacket = m_pRakPeer->Receive())
		{
			std::cout << "Peer :: ReceiveData :: Header [" << (int)pPacket->data[0] << "]" << std::endl;
			
			// Have to discard uninteresting packets!
			if (pPacket->data[0] < ID_USER_PACKET_ENUM)
			{
				m_pRakPeer->DeallocatePacket(pPacket);
				return false;
			}
			else if (pPacket->data[0] == ID_USER_PACKET_ENUM + p_ucIdOffset)
			{
				// Assign data
				p_data.clear(); p_data.assign(pPacket->data + 1, pPacket->data + pPacket->length);
				p_hostId = HostId::MakeHostId(pPacket->systemAddress.ToString(false), pPacket->systemAddress.GetPort());

				m_pRakPeer->DeallocatePacket(pPacket);
				return true;
			}
			else 
			{
				m_pRakPeer->PushBackPacket(pPacket, false);
				return false;
			}
		}

		return false;
	}
	//----------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------

