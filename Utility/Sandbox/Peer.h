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
#include <boost/lexical_cast.hpp>

#include <System/Platform.h>
#include <Maths/Maths.h>
#include <External/Compression/Compression.h>

#include <RakPeerInterface.h>
#include <ConnectionGraph2.h>
#include <BitStream.h>

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
	std::map<std::string, unsigned int> m_clockMap; 
	std::string m_strPeerId;

public:
	SparseVectorClock(const std::string &p_strPeerId)
		: m_strPeerId(p_strPeerId)
	{ }

	SparseVectorClock(const SparseVectorClock &p_clock) 
		: m_strPeerId(p_clock.m_strPeerId)
		, m_clockMap(p_clock.m_clockMap)
	{ }

	~SparseVectorClock(void) { }

	int Clock(void) 
	{ 
		return m_clockMap[m_strPeerId]; 
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

	void Supremum(const SparseVectorClock &p_clock) 
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
		return m_clockMap[m_strPeerId] = m_clockMap[m_strPeerId] + 1;
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

	std::string ToString(void)
	{
		std::stringstream result; 

		result << "PeerId : [" << m_strPeerId << "] :: [";

		for (auto pairIter : m_clockMap)
		{
			result << "<" << pairIter.first << " :: " << pairIter.second << "> ";
		}

		result << "]";

		return result.str();
	}
};
//----------------------------------------------------------------------------------------------

class P2PMessage;

struct Neighbour
{
	static std::string MakeKey(const std::string p_strAddress, int p_nPort) {
		return p_strAddress + ":" + boost::lexical_cast<std::string>(p_nPort);
	}

	std::string GetKey(void) {
		return MakeKey(Address, Port);
	}

	std::string Address;
	unsigned int Port;
	unsigned int Latency;
};

class Peer2
{
	RakNet::RakPeerInterface *m_pRakPeer;
	RakNet::ConnectionGraph2 m_connectionGraph;

	// std::vector<Neighbour> m_neighbours;
	std::map<std::string, Neighbour> m_neighbourMap;

	int m_nMaxConnections,
		m_nMaxIncomingConnections,
		m_nListenPort;

public:
	Peer2(int p_nPort, int p_nMaxConnections, int p_nMaxIncoming)
		: m_nListenPort(p_nPort)
		, m_nMaxConnections(p_nMaxConnections)
		, m_nMaxIncomingConnections(p_nMaxIncoming)
	{ }

	Peer2(void)
		: m_nListenPort(0)
		, m_nMaxConnections(0)
		, m_nMaxIncomingConnections(0)
	{ }

	void Configure(int p_nPort, int p_nMaxConnections, int p_nMaxIncoming)
	{
		m_nListenPort = p_nPort;
		m_nMaxConnections = p_nMaxConnections;
		m_nMaxIncomingConnections = p_nMaxIncoming;
	}

	bool Initialise(void)
	{
		m_pRakPeer = RakNet::RakPeerInterface::GetInstance();
		m_pRakPeer->AttachPlugin(&m_connectionGraph);

		RakNet::SocketDescriptor socketDescriptor(m_nListenPort, 0);
		socketDescriptor.socketFamily = AF_INET;
		
		RakNet::StartupResult sr = m_pRakPeer->Startup(m_nMaxConnections, &socketDescriptor, 1, THREAD_PRIORITY_NORMAL);
		
		if (sr == RakNet::RAKNET_STARTED)
		{
			m_pRakPeer->SetMaximumIncomingConnections(m_nMaxIncomingConnections);
			
			std::cout << "Peer :: Peer bound to the following addresses :" << std::endl;
			for (int addrIndex = 0; addrIndex < m_pRakPeer->GetNumberOfAddresses(); addrIndex++)
				std::cout << "[" << addrIndex << "] :: " << m_pRakPeer->GetLocalIP(addrIndex) << std::endl;
			
			return true;
		}

		std::cerr << "Peer :: RakNet failed to start! ERR => [" << sr << "]" << std::endl; 
		return false;
	}

	bool Ping(const std::string p_strRemoteAddress, int p_nRemotePort, int p_nTimeout)
	{
		m_pRakPeer->Ping(p_strRemoteAddress.c_str(), (unsigned short)p_nRemotePort, false);
		
		RakNet::Packet *pPacket;
		int deadline = RakNet::GetTimeMS() + p_nTimeout;
		
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
				RakNet::TimeMS checkpoint; 
				RakNet::BitStream bitStream(pPacket->data, pPacket->length, false);
				bitStream.IgnoreBytes(1); bitStream.Read(checkpoint);

				Neighbour neighbour;
				neighbour.Address = pPacket->systemAddress.ToString();
				neighbour.Port = (unsigned int)p_nRemotePort;
				neighbour.Latency = Maths::Min(RakNet::GetTimeMS() - checkpoint, p_nTimeout);
				m_neighbourMap[neighbour.GetKey()] = neighbour;

				m_pRakPeer->DeallocatePacket(pPacket);

				std::cout << "Got Pong from " << neighbour.Address << " :: Latency = " << neighbour.Latency << "ms" << std::endl;

				return true;
			}
		}

		return false;
	}

	bool Discover(int p_nRemotePort, int p_nTimeout)
	{
		m_pRakPeer->Ping("255.255.255.255", (unsigned short)p_nRemotePort, false);
		
		RakNet::Packet *pPacket;
		int deadline = RakNet::GetTimeMS() + p_nTimeout;
		
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
				RakNet::TimeMS checkpoint; 
				RakNet::BitStream bitStream(pPacket->data, pPacket->length, false);
				bitStream.IgnoreBytes(1); bitStream.Read(checkpoint);

				Neighbour neighbour;
				neighbour.Address = pPacket->systemAddress.ToString();
				neighbour.Port = (unsigned int)p_nRemotePort;
				neighbour.Latency = Maths::Min(RakNet::GetTimeMS() - checkpoint, p_nTimeout);
				m_neighbourMap[neighbour.GetKey()] = neighbour;

				m_pRakPeer->DeallocatePacket(pPacket);

				std::cout << "Got Pong from " << neighbour.Address << " :: Latency = " << neighbour.Latency << "ms" << std::endl;
			}
		}

		return true;
	}

	int RawReceive(boost::array<unsigned char, 4096> &p_receiveBuffer)
	{
		RakNet::Packet *pPacket;

		if (pPacket = m_pRakPeer->Receive())
		{
			int length = Maths::Min(4906, pPacket->length);
			// memcpy((unsigned char*)(p_receiveBuffer.data), pPacket->data, length);

			// deallocate packet
			m_pRakPeer->DeallocatePacket(pPacket);

			return length;
		}

		return 0;
	}

	/* bool RawSend(Peer &p_peer, std::vector<char> &p_data)
	{
		m_pRakPeer->Send(*(p_data.begin()), p_data.size(), HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_RAKNET_GUID, true);
		return true;
	} */

	void Shutdown(void)
	{
		RakNet::RakPeerInterface::DestroyInstance(m_pRakPeer);
	}
};

class Peer 
{
protected:
	boost::asio::io_service m_ioservice;

	boost::asio::ip::udp::endpoint m_endpoint;
	boost::asio::ip::udp::socket *m_pSocket;

public:
	bool RemoteBind(const std::string &p_strIP, int p_nPort)
	{
		try 
		{
			m_endpoint = boost::asio::ip::udp::endpoint(
				boost::asio::ip::address::from_string(p_strIP), boost::lexical_cast<int>(p_nPort));
		
			std::cout << "Remote Bind :: [" << m_endpoint << "]" << std::endl;		
		}

		catch(...)
		{
			std::cout << "Exception on RemoteBind()" << std::endl;
			return false;
		}

		return true;
	}

	bool Bind(const std::string &p_strIP, int p_nPort)
	{
		try 
		{
			if (p_strIP.length() > 0)
			{
				m_endpoint = boost::asio::ip::udp::endpoint(
					boost::asio::ip::address::from_string(p_strIP), boost::lexical_cast<int>(p_nPort));
			}
			else
			{
				m_endpoint = boost::asio::ip::udp::endpoint(
					boost::asio::ip::udp::v4(), boost::lexical_cast<int>(p_nPort));
			}

			std::cout << "Bind :: [" << m_endpoint << "]" << std::endl;

			m_pSocket = new boost::asio::ip::udp::socket(m_ioservice);
			m_pSocket->open(boost::asio::ip::udp::v4());
			m_pSocket->bind(m_endpoint);
		}

		catch (...)
		{
			std::cout << "Exception on Bind()" << std::endl;
			return false;
		}

		return true;
	}
	
	int RawReceive(boost::array<char, 4096> &p_receiveBuffer)
	{
		boost::asio::ip::udp::endpoint sender_endpoint;

		size_t length = m_pSocket->receive_from(
			boost::asio::buffer(p_receiveBuffer), sender_endpoint);

		return length;
	}

	bool RawSend(Peer &p_peer, std::vector<char> &p_data)
	{
		m_pSocket->send_to(boost::asio::buffer(p_data), p_peer.m_endpoint);
		return true;
	}

	/*
	void Connect(void);
	void SendMessage(Peer *p_pPeer, P2PMessage);
	void ReceiveMessage(Peer *p_pPeer, P2PMessage);
	void GetNeighbours(void);
	void Disconnect(void);
	*/
};


//----------------------------------------------------------------------------------------------

