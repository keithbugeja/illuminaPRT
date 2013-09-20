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
			std::cout << "Exception..." << std::endl;
			return false;
		}

		return true;
	}

	bool Bind(const std::string &p_strIP, int p_nPort)
	{
		try 
		{
			m_endpoint = boost::asio::ip::udp::endpoint(
				boost::asio::ip::address::from_string(p_strIP), boost::lexical_cast<int>(p_nPort));

			std::cout << "Bind :: [" << m_endpoint << "]" << std::endl;

			m_pSocket = new boost::asio::ip::udp::socket(m_ioservice);
			m_pSocket->open(boost::asio::ip::udp::v4());
			m_pSocket->bind(m_endpoint);
		}

		catch (...)
		{
			std::cout << "Exception..." << std::endl;
			return false;
		}

		return true;
	}
	
	bool RawReceive(boost::array<char, 4096> &p_receiveBuffer)
	{
		boost::asio::ip::udp::endpoint sender_endpoint;

		size_t length = m_pSocket->receive_from(
			boost::asio::buffer(p_receiveBuffer), sender_endpoint);

		std::cout.write(p_receiveBuffer.data(), length);

		return true;
	}

	bool RawSend(Peer &p_peer, std::vector<unsigned char> &p_data)
	{
		m_pSocket->send_to(boost::asio::buffer(p_data), p_peer.m_endpoint);
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

