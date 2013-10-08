#pragma once

#include "MultithreadedCommon.h"
#include "Transaction.h"
#include "Peer.h"

#define P2PLISTENER_GIC_EPOCH 0x7FFFFF

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

	// Transactions
	std::map<std::string, int> m_transactionMap;

public:
	HostDirectory &GetHostDirectory(void) { return m_hostDirectory; }

public:
	void Newsupdate(void)
	{
		HostId hostId;
		unsigned char streamId;
		RakNet::BitStream bitStream;

		if (m_pPeer->ReceiveIddStream(bitStream, streamId, hostId))
		{
			std::cout << "Newscast :: Received Data ..." << std::endl;

			switch(streamId)
			{
				case TTPeerList:
				{
					std::cout << "Newscast :: Stream id'd as PeerList" << std::endl;

					std::vector<HostId> hostList;
					HostDirectoryTransaction received;
					received.ReadFromBitStream(bitStream);
					received.GetData(hostList);

					std::cout << "Newscast :: Peers received [" << hostList.size() << "]" << std::endl;
					for (auto host : hostList)
					{
						std::cout << host.ToIPv4String() << ":" << host.GetPort() << std::endl;
					}

					auto me = std::find(hostList.begin(), hostList.end(), m_pPeer->GetHostId());
					if (me != hostList.end()) hostList.erase(me);

					m_hostDirectory.Add(hostId);
					m_hostDirectory.AddToDirectory(hostList);
					m_hostDirectory.Sort();
					m_hostDirectory.Truncate();

					hostList.clear();
					m_hostDirectory.GetDirectory(hostList);

					std::cout << "Newscast :: Host Directory updated ... " << std::endl;
					for (auto host : hostList)
					{
						std::cout << host.ToIPv4String() << ":" << host.GetPort() << std::endl;
					}

					break;
				}

				case TTIrradianceSamples:
				{
					std::cout << "Newscast :: Stream Id'd as IrradianceList" << std::endl;

					std::vector<MLIrradianceCacheRecord> irradianceList;
					IrradianceRecordTransaction received;
					received.ReadFromBitStream(bitStream);
					received.GetData(irradianceList);

					MLIrradianceCache *pIrradianceCache = m_pWFICIntegrator->GetIrradianceCache();
					m_transactionMap[received.GetIdString()] = m_newscastEpoch;

					for (auto irradiance : irradianceList)
					{
						irradiance.Epoch = m_newscastEpoch;
						pIrradianceCache->Insert(m_pWFICIntegrator->RequestRecord(&irradiance));
					}

					std::cout << "Newscast :: Transaction " << received.GetIdString() << " bound to epoch [" << m_newscastEpoch << "]" << std::endl;
					m_newscastEpoch++;
					break;
				}

				default:
				{
					std::cout << "Newscast :: Cannot id Stream" << std::endl;
					break;
				}
			}
		}
	}

	void Newscast(void)
	{
		// Has the deadline run out?
		if (RakNet::GetTimeMS() > m_newscastDeadline && m_newscastState!= NCChoose)
			m_newscastState = NCClose;

		std::vector<HostId> hostList;

		// What's the next step?
		switch (m_newscastState)
		{
			case NCChoose:
			{
				// Can't newscast if empty
				if (m_hostDirectory.IsEmpty()) return;

				m_hostDirectory.GetDirectory(hostList);

				int nextHost = m_random.Next(65535) % hostList.size();
				std::cout << "Next Host Index::" << nextHost << std::endl;

				m_exchangeHostId = 
					hostList[nextHost];

				// Connect to chosen host
				std::cout << "Newscast :: Choose :: Connecting to [" << m_exchangeHostId.ToIPv4String() << ":" << m_exchangeHostId.GetPort() << "]" << std::endl;
				m_pPeer->Connect(m_exchangeHostId, 0);
				
				// New state / set deadline
				m_newscastState = NCConnect;
				m_newscastDeadline = RakNet::GetTimeMS() + 10000;

				break;
			}

			case NCConnect:
			{
				std::cout << "Newscast :: Connect :: Waiting for host ... " << std::endl;

				if (m_pPeer->IsConnected(m_exchangeHostId))
				{
					std::cout << "Newscast :: Connect :: Acknowledged ... " << std::endl;
					m_newscastState = NCExchange;
				}

				break;
			}

			case NCExchange:
			{
				// Create transaction
				HostDirectoryTransaction hostExchangeTR(m_pPeer->GetHostId());
				m_hostDirectory.GetDirectory(hostList);
				hostExchangeTR.SetData(hostList);

				// Serialise to bitstream
				RakNet::BitStream bitstream;
				hostExchangeTR.WriteToBitStream(bitstream);

				std::cout << "Newscast :: Exchange :: Send PeerList transaction ... " << std::endl;
				m_pPeer->SendIddStream(m_exchangeHostId, TTPeerList, bitstream);

				// Create transaction
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

					std::cout << "Newscast :: Exchange :: Send Irradiance transaction ... " << std::endl;
					m_pPeer->SendIddStream(m_exchangeHostId, TTIrradianceSamples, bitstream);

					m_transactionMap[irradianceExchangeTR.GetIdString()] = lastEpoch;
					std::cout << "Newscast :: Transaction " << irradianceExchangeTR.GetIdString() << " bound to epoch [" << lastEpoch << "]" << std::endl;
				}

				m_newscastState = NCWaitClose;

				break;
			}

			case NCWaitClose:
			{
				std::cout << "Newscast :: WaitClose ... " << std::endl;
				break;
			}

			case NCClose:
			{
				std::cout << "Newscast :: Disconnecting..." << std::endl;
				m_pPeer->Disconnect(m_exchangeHostId);

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
			if (m_eRole == P2PReceive) m_pWFICIntegrator->DisableSampleGeneration(true);

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

		/*
		std::cout << "P2PListener :: Populating neighbour list..." << std::endl;
		m_pPeer->Ping("255.255.255.255", m_pPeer->GetOutgoingPort(), 2500, m_neighbourList);
		//m_pPeer->GetNeighbours(m_neighbourList);

		if (!m_neighbourList.empty())
		{
			std::cout << "P2PListener :: Found " << m_neighbourList.size() << " neighbours." << std::endl;
			
			for (auto neighbour : m_neighbourList)
			{
				std::cout << "P2PListener :: Connecting to " << neighbour.Address << ":" << neighbour.Port << "..." << std::endl;
				m_pPeer->Connect(neighbour, 500);
			}
		}
		*/
	}

	void OnEndRender(IIlluminaMT *p_pIlluminaMT)
	{
		/*
		for (auto neighbour : m_neighbourList)
			m_pPeer->Disconnect(neighbour);
		*/
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

		/*
		// Share irradiance?
		if (m_pWFICIntegrator != NULL)
		{
			// Do we have any neighbours with whom to share?
			if (m_neighbourList.size() > 0)
			{
				if (m_eRole == P2PSend)
				{
					int epoch = m_pWFICIntegrator->NextEpoch();
					std::vector<MLIrradianceCacheRecord*> recordList;
					m_pWFICIntegrator->GetByEpoch(epoch, recordList);

					std::cout << "P2PListener :: [Send Role] :: Fetched [" << recordList.size() << "] for epoch [" << epoch << "]" << std::endl;
					if (!recordList.empty())
					{
						MLIrradianceCacheRecord *pRecordBuffer = m_pRecordBuffer;
						for (auto record : recordList)
							*pRecordBuffer++ = *record;

						// For each connected neighbour, share
						for (auto neighbour : m_neighbourList)
						{
							std::cout << "P2PListener :: [Send Role] :: Sending batch to [" << neighbour.Address << ":" << neighbour.Port << "]" << std::endl;
							//m_pPeer->RawSend(neighbour, (char*)m_pRecordBuffer, 
							m_pPeer->SendData(neighbour, (char*)m_pRecordBuffer,
								recordList.size() * sizeof(MLIrradianceCacheRecord));
						}
					}
				} 
				else if (m_eRole == P2PReceive)
				{
					MLIrradianceCache *pIrradianceCache = 
						m_pWFICIntegrator->GetIrradianceCache();

					Neighbour neighbour;

					// While any messages are waiting in the queue
					for(std::vector<unsigned char> receiveBuffer;
						//m_pPeer->RawReceive(receiveBuffer, neighbour);
						m_pPeer->ReceiveData(receiveBuffer, neighbour);
						receiveBuffer.clear())
					{
						if (!receiveBuffer.empty())
						{
							MLIrradianceCacheRecord *pRecord = 
								(MLIrradianceCacheRecord*)receiveBuffer.data();
							int recordCount = receiveBuffer.size() / sizeof(MLIrradianceCacheRecord);

							std::cout << "P2PListener :: [Receive Role] :: Received [" << recordCount << "] samples from [" << neighbour.Address << ":" << neighbour.Port << "]" << std::endl;

							for(;recordCount > 0; --recordCount, pRecord++)
								pIrradianceCache->Insert(m_pWFICIntegrator->RequestRecord(pRecord));
						}
					}
				}
			}
		}
		*/
	}
};
