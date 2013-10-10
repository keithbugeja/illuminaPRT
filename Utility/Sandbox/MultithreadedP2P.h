#pragma once

#include "MultithreadedCommon.h"
#include "Transaction.h"
#include "Peer.h"

#define P2PLISTENER_GIC_EPOCH 0x7FFFFF

/* */
class P2PListener2Way 
	: public IlluminaMTListener
{
public:
	enum Role 
	{
		P2PSend,
		P2PReceive,
		P2PSendReceive
	};

protected:
	enum State
	{
		NCInitiateConnection,
		NCConnect,
		NCPeerSend,
		NCPeerReceive,
		NCTransactionSend,
		NCTransactionReceive,
		NCIrradianceSend,
		NCIrradianceReceive,
		NCQuiescent,
		NCTerminateConnection
	};

protected:
	// P2P message hub for current peer
	Peer *m_pPeer;

	// Role of current peer
	Role m_eRole;

	// WFIC Integrator
	MLICIntegrator *m_pWFICIntegrator;

	// Record buffer for sends/receives
	MLIrradianceCacheRecord *m_pRecordBuffer;

	// Host directory for known peers
	HostDirectory m_hostDirectory;

	// Random number generator
	Illumina::Core::Random m_random;

	// Newscast exchange details
	State m_newscastState;
	int m_newscastDeadline,
		m_newscastEpoch;

	HostId m_exchangeHostId;
	std::vector<boost::uuids::uuid> m_exchangeRequestList;

	// Transactions
	std::map<boost::uuids::uuid, int> m_transactionMap;

	bool m_bIsRunning;

public:
	//----------------------------------------------------------------------------------------------
	HostDirectory& GetHostDirectory(void);
	//----------------------------------------------------------------------------------------------

protected:
	static void BackgroundThreadHandler(P2PListener2Way *p_pListener);

	//----------------------------------------------------------------------------------------------
	bool State_InitiateConnection(void);
	bool State_Connect(HostId p_hostId);
	bool State_PeerSend(HostId p_hostId, bool p_bResponse = false);
	bool State_PeerReceive(RakNet::BitStream &p_bitStream, HostId p_hostId);
	bool State_TransactionSend(HostId p_hostId, bool p_bResponse = false);
	bool State_TransactionReceive(RakNet::BitStream &p_bitStream, HostId p_hostId, std::vector<boost::uuids::uuid> &p_outRequestList);
	bool State_IrradianceSend(HostId p_hostId, std::vector<boost::uuids::uuid> &p_requestList, bool p_bResponse = false);
	bool State_IrradianceReceive(RakNet::BitStream &p_bitStream, HostId p_hostId);
	//----------------------------------------------------------------------------------------------
public:
	//----------------------------------------------------------------------------------------------
	void UpdateLocalCatalogue(void);
	void NewsUpdate(void);
	void NewsCast(void);
	//----------------------------------------------------------------------------------------------
	bool IsRunning(void);
	void SetPeer(Peer *p_pPeer, Role p_eRole = P2PSendReceive);
	void SetCameraPath(std::vector<PathEx> &p_cameraPath);
	//----------------------------------------------------------------------------------------------
	void OnBeginRender(IIlluminaMT *p_pIlluminaMT);
	void OnEndRender(IIlluminaMT *p_pIlluminaMT);
	void OnBeginFrame(IIlluminaMT *p_pIlluminaMT);
	void OnEndFrame(IIlluminaMT *p_pIlluminaMT);
	//----------------------------------------------------------------------------------------------
};

class P2PListener 
	: public IlluminaMTListener
{
public:
	enum Role 
	{
		P2PSend,
		P2PReceive,
		P2PSendReceive
	};

	enum NewscastState
	{
		NCChoose,
		NCConnect,
		NCExchange,
		NCWaitClose,
		NCClose
	};

protected:
	// P2P message hub for current peer
	Peer *m_pPeer;

	// Role of current peer
	Role m_eRole;

	// WFIC Integrator
	MLICIntegrator *m_pWFICIntegrator;

	// Record buffer for sends/receives
	MLIrradianceCacheRecord *m_pRecordBuffer;

	// Host directory for known peers
	HostDirectory m_hostDirectory;

	// Random number generator
	Illumina::Core::Random m_random;

	// Newscast exchange details
	HostId m_exchangeHostId;
	NewscastState m_newscastState;
	int m_newscastDeadline,
		m_newscastEpoch;
	bool m_newscastConnected;

	// Transactions
	std::map<boost::uuids::uuid, int> m_transactionMap;

public:
	HostDirectory &GetHostDirectory(void) { return m_hostDirectory; }

protected:
	//----------------------------------------------------------------------------------------------
	bool ProcessPeerList(RakNet::BitStream &p_peerListBitStream, HostId &p_hostId)
	{
		std::cout << "Newsupdate :: PacketId = [PeerList]" << std::endl;

		std::vector<HostId> hostList;
		HostDirectoryTransaction received;
		received.ReadFromBitStream(p_peerListBitStream);
		received.GetData(hostList);

		std::cout << "Newsupdate :: Received [" << hostList.size() << "] peers:" << std::endl;
		for (auto host : hostList) std::cout << "---" << host.ToIPv4String() << ":" << host.GetPort() << std::endl;
		
		// Remove entry of this host
		auto me = std::find(hostList.begin(), hostList.end(), m_pPeer->GetHostId());
			if (me != hostList.end()) hostList.erase(me);

		// Modify local host directory to include remote list
		m_hostDirectory.Add(p_hostId);
		m_hostDirectory.AddToDirectory(hostList);
		m_hostDirectory.Sort();
		m_hostDirectory.Truncate();

		// Show updated directory
		hostList.clear();
		m_hostDirectory.GetDirectory(hostList);

		std::cout << "Newsupdate :: Host Directory updated:" << std::endl;
		for (auto host : hostList) std::cout << "---" << host.ToIPv4String() << ":" << host.GetPort() << std::endl;

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool ProcessTransactionList(RakNet::BitStream &p_transactionBitStream, HostId &p_hostId)
	{
		std::cout << "Newsupdate :: PacketId = [TransactionList]" << std::endl;

		std::vector<boost::uuids::uuid> transactionList,
			requestList;

		TransactionListTransaction received;
		received.ReadFromBitStream(p_transactionBitStream);
		received.GetData(transactionList);

		// Request transactions from the list that we don't have
		std::cout << "Newsupdate :: Received [" << transactionList.size() << "] transactions, listed below:" << std::endl;
		for (auto uuid : transactionList)
		{
			std::cout << ITransaction::GetIdString(uuid);
			if (m_transactionMap.find(uuid) == m_transactionMap.end())
			{
				requestList.push_back(uuid);
				std::cout << "[-]" << std::endl;
			}
			else
				std::cout << "[+]" << std::endl;
		}

		// If at least one transaction requested...
		if (!requestList.empty())
		{
			RakNet::BitStream bitStream;
			TransactionListTransaction transactionRequestExchangeTR(m_pPeer->GetHostId());
			transactionRequestExchangeTR.SetData(requestList);
			transactionRequestExchangeTR.WriteToBitStream(bitStream);

			std::cout << "Newsupdate :: Exchange :: Requesting [" << requestList.size() << "] transactions ..." << std::endl;
			m_pPeer->SendIddStream(p_hostId, TTTransactionList, bitStream, 0x01);

			// Extend deadline
			m_newscastDeadline += 50 * requestList.size();
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool ProcessIrradianceList(RakNet::BitStream &p_irradianceBitStream, HostId &p_hostId)
	{
		std::cout << "Newsupdate :: PacketId = [IrradianceList]" << std::endl;

		std::vector<MLIrradianceCacheRecord> irradianceList;
		IrradianceRecordTransaction received;
		received.ReadFromBitStream(p_irradianceBitStream);
		received.GetData(irradianceList);

		MLIrradianceCache *pIrradianceCache = m_pWFICIntegrator->GetIrradianceCache();
		m_transactionMap[received.GetId()] = m_newscastEpoch;

		for (auto irradiance : irradianceList)
			pIrradianceCache->Insert(m_pWFICIntegrator->RequestRecord(&irradiance, m_newscastEpoch));

		std::cout << "Newsupdate :: Transaction " << received.GetIdString() << " bound to epoch [" << m_newscastEpoch << "]" << std::endl;
		m_newscastEpoch++;

		return true;
	}

public:
	//----------------------------------------------------------------------------------------------
	// Newsupdate method
	//----------------------------------------------------------------------------------------------
	void Newsupdate(void)
	{
		HostId hostId;
		unsigned char streamId;
		RakNet::BitStream bitStream;

		// Check if we have any pending packets
		if (m_pPeer->ReceiveIddStream(bitStream, streamId, hostId))
		{
			// Received data 
			std::cout << "Newscast :: Received Data from [" << hostId.ToIPv4String() << " : " << hostId.GetPort() << "]" << std::endl;

			// What packet type are we handling?
			switch(streamId)
			{
				//----------------------------------------------------------------------------------------------
				// Received peer list
				//----------------------------------------------------------------------------------------------
				case TTPeerList:
				{
					ProcessPeerList(bitStream, hostId);
					break;
				}

				//----------------------------------------------------------------------------------------------
				// Received transaction list
				//----------------------------------------------------------------------------------------------
				case TTTransactionList:
				{
					ProcessTransactionList(bitStream, hostId);
					break;
				}

				//----------------------------------------------------------------------------------------------
				// Received irradiance sample list
				//----------------------------------------------------------------------------------------------
				case TTIrradianceSamples:
				{
					ProcessIrradianceList(bitStream, hostId);
					break;
				}

				//----------------------------------------------------------------------------------------------
				// Undefined packet
				//----------------------------------------------------------------------------------------------
				default:
				{
					std::cout << "Newscast :: PacketId = [unidentified]" << std::endl;
					break;
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	// Newscast method
	//----------------------------------------------------------------------------------------------
	void Newscast(void)
	{
		// Has the deadline run out?
		if (RakNet::GetTimeMS() > m_newscastDeadline && m_newscastState!= NCChoose)
			m_newscastState = NCClose;

		std::vector<HostId> hostList;

		// What's the next step?
		switch (m_newscastState)
		{
			//----------------------------------------------------------------------------------------------
			// Choose peer to connect to
			//----------------------------------------------------------------------------------------------
			case NCChoose:
			{
				// Can't newscast if empty
				if (m_hostDirectory.IsEmpty()) return;

				// Get current host list
				m_hostDirectory.GetDirectory(hostList);

				// Choose next host
				int nextHost = m_random.Next(65535) % hostList.size();
				m_exchangeHostId = hostList[nextHost];

				// Connect to chosen host
				std::cout << "Newscast :: Choose :: Connecting to [" << m_exchangeHostId.ToIPv4String() << ":" << m_exchangeHostId.GetPort() << "]" << std::endl;
				m_pPeer->Connect(m_exchangeHostId, 0);
				
				// New state / set deadline
				m_newscastState = NCConnect;
				m_newscastConnected = false;
				m_newscastDeadline = RakNet::GetTimeMS() + 5000;

				break;
			}

			//----------------------------------------------------------------------------------------------
			// Connect
			//----------------------------------------------------------------------------------------------
			case NCConnect:
			{
				std::cout << "Newscast :: Connect :: Waiting for host ... " << std::endl;

				RakNet::ConnectionState cs = m_pPeer->GetConnectionState(m_exchangeHostId);
				switch(cs)
				{
					case RakNet::ConnectionState::IS_CONNECTED:
					{
						std::cout << "Newscast :: Connect :: Acknowledged ... " << std::endl;

						m_newscastConnected = true;
						m_newscastState = NCExchange;
						break;
					}

					case RakNet::ConnectionState::IS_DISCONNECTING:
					case RakNet::ConnectionState::IS_DISCONNECTED:
					{
						m_newscastState = NCClose;
						break;
					}
				}

				break;
			}

			//----------------------------------------------------------------------------------------------
			// Exchange data
			//----------------------------------------------------------------------------------------------
			case NCExchange:
			{
				//----------------------------------------------------------------------------------------------
				// Create Peer List transaction
				//----------------------------------------------------------------------------------------------
				HostDirectoryTransaction hostExchangeTR(m_pPeer->GetHostId());
				m_hostDirectory.GetDirectory(hostList);
				hostExchangeTR.SetData(hostList);

				// Serialise to bitstream
				RakNet::BitStream bitstream;
				hostExchangeTR.WriteToBitStream(bitstream);

				std::cout << "Newscast :: Exchange :: Send PeerList transaction to [" << m_exchangeHostId.ToIPv4String() << " : " << m_exchangeHostId.GetPort() << "]" << std::endl;
				m_pPeer->SendIddStream(m_exchangeHostId, TTPeerList, bitstream);

				//----------------------------------------------------------------------------------------------
				// Create Transaction List transaction
				//----------------------------------------------------------------------------------------------
				std::vector<boost::uuids::uuid> transactionList;
				TransactionListTransaction transactionExchangeTR(m_pPeer->GetHostId());
				for (auto transaction : m_transactionMap)
					transactionList.push_back(transaction.first);

				if (!transactionList.empty())
				{
					bitstream.Reset();
					transactionExchangeTR.SetData(transactionList);
					transactionExchangeTR.WriteToBitStream(bitstream);

					std::cout << "Newscast :: Exchange :: Send Catalogue transaction to [" << m_exchangeHostId.ToIPv4String() << " : " << m_exchangeHostId.GetPort() << "]" << std::endl;
					m_pPeer->SendIddStream(m_exchangeHostId, TTTransactionList, bitstream);
				}

				//----------------------------------------------------------------------------------------------
				// Create Irradiance List transaction
				//----------------------------------------------------------------------------------------------
				// We need a quota of at least 100 samples before sending
				if (m_pWFICIntegrator->HasEpochQuota(100))
				{
					IrradianceRecordTransaction irradianceExchangeTR(m_pPeer->GetHostId());
					int lastEpoch = m_pWFICIntegrator->NextEpoch();
					std::vector<MLIrradianceCacheRecord*> recordList;
					m_pWFICIntegrator->GetByEpoch(lastEpoch, recordList);

					std::cout << "Newscast :: Exchange :: Fetched [" << recordList.size() << "] for epoch [" << lastEpoch << "]" << std::endl;
					if (!recordList.empty())
					{
						std::vector<MLIrradianceCacheRecord> irradianceList;
						for (auto record : recordList)
							irradianceList.push_back(*record);

						bitstream.Reset();
						irradianceExchangeTR.SetData(irradianceList);
						irradianceExchangeTR.WriteToBitStream(bitstream);

						std::cout << "Newscast :: Exchange :: Irradiance transaction to [" << m_exchangeHostId.ToIPv4String() << " : " << m_exchangeHostId.GetPort() << "]" << std::endl;
						m_pPeer->SendIddStream(m_exchangeHostId, TTIrradianceSamples, bitstream);

						m_transactionMap[irradianceExchangeTR.GetId()] = lastEpoch;
						std::cout << "Newscast :: Transaction " << irradianceExchangeTR.GetIdString() << " bound to epoch [" << lastEpoch << "]" << std::endl;
					}
				}

				m_newscastState = NCWaitClose;

				break;
			}

			//----------------------------------------------------------------------------------------------
			// Wait for deadline to expire
			//----------------------------------------------------------------------------------------------
			case NCWaitClose:
			{
				std::cout << "Newscast :: WaitClose ... " << std::endl;

				// Check whether 
				RakNet::BitStream bitStream;
				unsigned char streamId;
				HostId hostId;

				// Check if the are pending messages
				while (m_pPeer->ReceiveIddStream(bitStream, streamId, hostId, 0x01))
				{
					// Only allow transaction lists
					if (streamId == TTTransactionList && hostId == m_exchangeHostId)
					{
						// Prepare irradiance exchange transaction
						IrradianceRecordTransaction irradianceExchangeTR(m_pPeer->GetHostId());
						int epoch; std::vector<MLIrradianceCacheRecord*> recordList;

						std::vector<boost::uuids::uuid> transactionList;
						TransactionListTransaction received;
						received.ReadFromBitStream(bitStream);
						received.GetData(transactionList);

						// Go through each transaction and send
						for (auto uuid : transactionList)
						{
							std::cout << "Newscast :: Satisfying transaction " << ITransaction::GetIdString(uuid) << " ..." << std::endl;

							epoch = m_transactionMap[uuid]; recordList.clear();
							m_pWFICIntegrator->GetByEpoch(epoch, recordList);
							irradianceExchangeTR.SetId(uuid);

							std::cout << "Newscast :: Request :: Fetched [" << recordList.size() << "] for epoch [" << epoch << "]" << std::endl;
							if (!recordList.empty())
							{
								std::vector<MLIrradianceCacheRecord> irradianceList;
								for (auto record : recordList)
									irradianceList.push_back(*record);

								bitStream.Reset();
								irradianceExchangeTR.SetData(irradianceList);
								irradianceExchangeTR.WriteToBitStream(bitStream);

								std::cout << "Newscast :: Request :: Send Irradiance transaction ... " << std::endl;
								m_pPeer->SendIddStream(m_exchangeHostId, TTIrradianceSamples, bitStream);
							}
						}
					}
				}

				break;
			}
			//----------------------------------------------------------------------------------------------
			// Close connection
			//----------------------------------------------------------------------------------------------
			case NCClose:
			{
				std::cout << "Newscast :: Disconnecting..." << std::endl;
				m_pPeer->Disconnect(m_exchangeHostId);

				// if (!m_newscastConnected) m_hostDirectory.Remove(m_exchangeHostId);

				m_newscastState = NCChoose;
				break;
			}
		}
	}

	void SetPeer(Peer *p_pPeer, Role p_eRole = P2PSendReceive) 
	{
		m_pPeer = p_pPeer;
		m_eRole = p_eRole;
	}

	void OnBeginRender(IIlluminaMT *p_pIlluminaMT)
	{
		IIntegrator *pIntegrator = p_pIlluminaMT->GetEnvironment()->GetIntegrator();
		if (pIntegrator->GetType() == "WFIC")
		{
			m_pWFICIntegrator = (MLICIntegrator*)pIntegrator;
			if (m_eRole == P2PReceive) { m_pWFICIntegrator->DisableSampleGeneration(true); std::cout << "Peer :: Disable Sample Generation = [true]" << std::endl; }

			// Allocate record buffer (maximum of 100K samples)
			m_pRecordBuffer = new MLIrradianceCacheRecord[102400];
		} 
		else
		{
			m_pWFICIntegrator = NULL;
			m_pRecordBuffer = NULL;
		}

		m_newscastEpoch = P2PLISTENER_GIC_EPOCH;
		m_newscastState = NCChoose;
		m_newscastDeadline = 0;
	}

	void OnEndRender(IIlluminaMT *p_pIlluminaMT)
	{
	}

	void OnBeginFrame(IIlluminaMT *p_pIlluminaMT) 
	{ 
		ICamera* pCamera = p_pIlluminaMT->GetEnvironment()->GetCamera();
		// pCamera->MoveTo(pCamera->GetObserver() + pCamera->GetFrame().W * 1.0f);
	};

	void OnEndFrame(IIlluminaMT *p_pIlluminaMT)
	{
		if (m_pWFICIntegrator != NULL)
		{
			Newscast();
			Newsupdate();
		}
	}
};
