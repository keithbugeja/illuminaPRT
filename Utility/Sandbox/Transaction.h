#include <map>
#include <vector>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <System/Platform.h>
#include "Peer.h"

enum TransactionType
{
	TTGeneric,
	TTPeerList,
	TTTransactionList,
	TTIrradianceSamples,
	TTOriginator
};

class ITransaction
{
protected:
	boost::uuids::uuid m_transactionId;
	SparseVectorClock m_timestamp;
	unsigned long m_ulType;
	HostId m_hostId;

	ITransaction(void)
		: m_transactionId(boost::uuids::random_generator()())
	{ }

	ITransaction(unsigned long p_ulType, HostId p_hostId)
		: m_transactionId(boost::uuids::random_generator()())
		, m_ulType(p_ulType)
		, m_hostId(p_hostId)
		, m_timestamp(p_hostId)
	{ }

	ITransaction(unsigned long p_ulType, HostId p_hostId, SparseVectorClock &p_timestamp)
		: m_transactionId(boost::uuids::random_generator()())
		, m_ulType(p_ulType)
		, m_hostId(p_hostId)
		, m_timestamp(p_timestamp)
	{ }

public:
	static std::string GetIdString(boost::uuids::uuid &p_uuid)
	{
		std::stringstream uid; uid << "[ " << std::hex;
		for (auto idbyte : p_uuid)
			uid << (unsigned int)idbyte;
		uid << " ]" << std::dec;

		return uid.str();
	}

	static boost::uuids::uuid GenerateId(void)
	{
		return boost::uuids::random_generator()();
	}

public:
	// Id
	void SetId(boost::uuids::uuid &p_uuid) { m_transactionId = p_uuid; }
	boost::uuids::uuid GetId(void) { return m_transactionId; }
	std::string GetIdString(void) 
	{
		std::stringstream uid; uid << "[ " << std::hex;
		for (auto idbyte : m_transactionId)
			uid << (unsigned int)idbyte;
		uid << " ]" << std::dec;

		return uid.str();
	}

	// Type
	unsigned long GetType(void) { return m_ulType; }
	
	// Host id
	HostId GetHostId(void) { return m_hostId; }

	// Timestamp
	SparseVectorClock& GetTimestamp(void) { return m_timestamp; }
	void SetTimestamp(SparseVectorClock &p_timestamp) { m_timestamp = p_timestamp; }

	virtual ~ITransaction(void) { }

	virtual size_t GetLength(void) = 0;
	virtual void *GetData(void) = 0;

	virtual void WriteToBitStream(RakNet::BitStream &p_bitstream) = 0;
	virtual void ReadFromBitStream(RakNet::BitStream &p_bitstream) = 0;
};


template <class T, TransactionType K>
class IDataTransaction
	: public ITransaction
{
protected:
	using ITransaction::m_transactionId;
	using ITransaction::m_timestamp;
	using ITransaction::m_hostId;
	using ITransaction::m_ulType;

protected:
	void *m_pData;
	int m_nLength;

public:
	IDataTransaction(void)
		: ITransaction()
		, m_pData(NULL)
		, m_nLength(-1)
	{ }

	IDataTransaction(HostId p_hostId)
		: ITransaction(K, p_hostId)
		, m_pData(NULL)
		, m_nLength(-1)
	{ }

	IDataTransaction(HostId p_hostId, SparseVectorClock &p_timestamp)
		: ITransaction(K, p_hostId, p_timestamp)
		, m_pData(NULL)
		, m_nLength(-1)
	{ }

	~IDataTransaction(void)
	{
		Safe_Delete(m_pData);
	}

	void SetData(std::vector<T> &p_dataList)
	{
		Safe_Delete(m_pData);
		
		m_nLength = p_dataList.size() * sizeof(T);
		m_pData = new unsigned char[m_nLength];
		memcpy(m_pData, p_dataList.data(), m_nLength);
	}

	void GetData(std::vector<T> &p_dataList)
	{
		int count = m_nLength / sizeof(T);
		
		for (T *pDatum = (T*)m_pData; count > 0; count--, pDatum++)
			p_dataList.push_back(*pDatum);
	}

	void SetData(void *p_pData, int p_nLength)
	{
		Safe_Delete(m_pData);
		m_nLength = p_nLength;
		m_pData = new unsigned char[m_nLength];
		memcpy(m_pData, p_pData, m_nLength);
	}

	size_t GetLength(void) { return m_nLength; }
	void* GetData(void) { return m_pData; }

	void WriteToBitStream(RakNet::BitStream &p_bitstream)
	{
		p_bitstream.Write((const char*)m_transactionId.data, 16);
		p_bitstream.Write((unsigned long)m_ulType);
		p_bitstream.Write((unsigned long long)m_hostId.GetHash());
		
		m_timestamp.WriteToBitStream(p_bitstream);

		p_bitstream.Write((int)m_nLength);
		p_bitstream.Write((const char*)m_pData, m_nLength);
	}

	void ReadFromBitStream(RakNet::BitStream &p_bitstream)
	{
		Safe_Delete(m_pData);

		unsigned long long hostId;

		p_bitstream.Read((char*)m_transactionId.data, 16);
		p_bitstream.Read(m_ulType);
		
		p_bitstream.Read(hostId);
		m_hostId = hostId;

		m_timestamp.ReadFromBitStream(p_bitstream);

		p_bitstream.Read(m_nLength);
		m_pData = new unsigned char[m_nLength];
		p_bitstream.Read((char*)m_pData, m_nLength);
	}
};

class HostDirectory
{
protected:
	std::vector<std::pair<HostId, SparseVectorClock>> m_hostList2;
	std::vector<std::pair<HostId, double>> m_hostList;
	int m_nSize;

protected:
	class FindFirstPredicate 
	{
	public:
		FindFirstPredicate(const HostId &hostId) 
			: toFind(hostId) { }
	
		HostId toFind;
		
		bool operator() ( std::pair<HostId, double> &p ) 
		{
			return p.first==toFind;
		}
	};

public:
	HostDirectory(int p_nSize = 10)
		: m_nSize(p_nSize)
	{ }

	void Remove(HostId p_hostId)
	{
		auto it = std::find_if(m_hostList.begin(), m_hostList.end(), FindFirstPredicate(p_hostId)); 
		if (it != m_hostList.end()) m_hostList.erase(it);
	}

	void Add(HostId p_hostId) {
		m_hostList.push_back(std::pair<HostId, double>(p_hostId, Illumina::Core::Platform::GetTime()));
	}

	void Sort(void) 
	{
		std::sort(m_hostList.begin(), m_hostList.end(), 
			[](const std::pair<HostId, double>& lhs, const std::pair<HostId, double>& rhs) {
				return lhs.second > rhs.second; 
			}
		);
	}

	void Truncate(void)
	{
		if (m_hostList.size() > m_nSize)
			m_hostList.erase(m_hostList.begin() + m_nSize, m_hostList.end());
	}

	void AddToDirectory(std::vector<HostId> &p_hostList)
	{
		for (auto host : p_hostList) Add(host);

		std::map<unsigned int, std::pair<HostId, double>> hostMap;
		for (auto host : m_hostList)
		{
			auto hostIt = hostMap.find(host.first.GetIPv4());

			if (hostIt == hostMap.end() || 
				(hostIt != hostMap.end() && hostIt->second.second < host.second))
			{
				hostMap[host.first.GetIPv4()] = host;
			}
		}

		m_hostList.clear();
		for (auto host : hostMap) m_hostList.push_back(host.second);
	}

	void GetDirectory(std::vector<HostId> &p_hostList)
	{
		for (auto host : m_hostList)
			p_hostList.push_back(HostId(host.first));
	}

	bool IsEmpty(void) {
		return m_hostList.empty();
	}

	size_t Size(void) {
		return m_hostList.size();
	}
};

typedef IDataTransaction<HostId, TTPeerList> HostDirectoryTransaction;
typedef IDataTransaction<boost::uuids::uuid, TTTransactionList> TransactionListTransaction;
typedef IDataTransaction<MLIrradianceCacheRecord, TTIrradianceSamples> IrradianceRecordTransaction;

class TransactionRecord
{
protected:
	boost::uuids::uuid m_transactionId;
	unsigned long m_ulType;
	HostId m_originatorId;
	SparseVectorClock m_timestamp;

public:
	TransactionRecord(void) { }

	TransactionRecord(boost::uuids::uuid p_transactionId, unsigned long p_ulType, HostId p_originator, SparseVectorClock p_timestamp)
		: m_transactionId(p_transactionId)
		, m_ulType(p_ulType)
		, m_originatorId(p_originator)
		, m_timestamp(p_timestamp)
	{ }

	boost::uuids::uuid GetTransactionId(void) const { return m_transactionId; }
	unsigned long GetType(void) const { return m_ulType; }
	HostId GetOriginatorId(void) const { return m_originatorId; }
	SparseVectorClock& GetTimestamp(void) { return m_timestamp; }

	std::string ToString(void) 
	{
		std::stringstream result;

		result << ITransaction::GetIdString(m_transactionId) << " : Type = [" << m_ulType << "]" 
			<< " : HostId = [" << m_originatorId.ToIPv4String() << " : " << m_originatorId.GetPort() << "]"
			<< " : Timestamp = [" << m_timestamp.ToString() << "]";

		return result.str();
	}
};